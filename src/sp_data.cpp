/*
	Copyright 2026, Philip Rose, GM3ZZA
	
    This file is part of ZZAVNAD. VNA Analysis Software.

    ZZAVNAD is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

    ZZAVNAD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
	PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZAVNAD. 
	If not, see <https://www.gnu.org/licenses/>. 

*/
#include "sp_data.hpp"

#include "nvna_control.hpp"

// Include ZZACOMMON items.
#include "zc_graph.h"
#include "zc_settings.h"
#include "zc_status.h"
#include "zc_utils.h"

// Include FLTK headers.
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

// Include C++ standard library headers.
#include <complex>
#include <fstream>
#include <iostream> 
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Constructor for the S-parameter data manager.
sp_data::sp_data() {
	datasets_.clear();
	create_active_dataset();
    load_settings();
	// If any datasets were loaded from settings, attempt to read the data for them.
    for (int i = 0; i < datasets_.size(); i++) {
        sp_data_entry* entry = datasets_[i];
        if (entry->source == SPDS_FILE) {
            if (!read_data_from_file(entry)) {
                if (status_) status_->misc_status(ST_ERROR, "Failed to read data from file: %s", entry->filename.c_str());
            }
        }
	}
}

// Destructor for the S-parameter data manager.
sp_data::~sp_data() {
    save_settings();
    // Clean up the datasets.
    for (auto dataset : datasets_) {
        delete dataset;
    }
    datasets_.clear();
}

// Create an active dataset for acquiring data from the VNA.
void sp_data::create_active_dataset() {
	int add_index = add_dataset(SPDS_ACTIVE);
	if (add_index != NANO_VNA_INDEX) {
        // Raise an error - the active dataset must be at the nanoVNA index.
        throw std::logic_error("Active dataset must be at index " + std::to_string(NANO_VNA_INDEX));
    }
    sp_data_entry* entry = get_dataset(add_index);
	entry->enabled = false;
	entry->valid_ports = get_number_ports();
    entry->z0 = default_z0_;
    entry->line_style_l = zc_line_style(COLOUR_CODE[0], 2, FL_SOLID);
    entry->line_style_r = zc_line_style(COLOUR_CODE[0], 1, FL_SOLID);
}

// Load settings from previous sessions.
void sp_data::load_settings() {
    zc_settings settings;
    zc_settings sp_settings(&settings, "Data");
    // Load specific settings for sp_data here.
    int dataset_count;
	sp_settings.get("Dataset Count", dataset_count, 0);
    for (int i = 0; i < dataset_count; i++) {
		std::string dataset_name = "Dataset " + std::to_string(i);
        zc_settings dataset_settings(&sp_settings, dataset_name);
		int index = add_dataset(SPDS_FILE);
		sp_data_entry* entry = get_dataset(index);
		dataset_settings.get<std::string>("Filename", entry->filename, "");
		dataset_settings.get("Enabled", entry->enabled, true);
        dataset_settings.get("Valid Ports", entry->valid_ports, 2);
        dataset_settings.get("Left Colour", entry->line_style_l.colour, COLOUR_CODE[index % 9]);
		dataset_settings.get("Left Width", entry->line_style_l.width, 2);
		dataset_settings.get("Left Style", entry->line_style_l.style, (int)FL_SOLID);
		dataset_settings.get("Right Colour", entry->line_style_r.colour, COLOUR_CODE[index % 9]);
		dataset_settings.get("Right Width", entry->line_style_r.width, 2);
		dataset_settings.get("Right Style", entry->line_style_r.style, (int)FL_DASH);
    }
	// Get settings for the nanoVNA dataset if it exists.
	zc_settings nvna_settings(&sp_settings, "nanoVNA");
	sp_data_entry* nvna_entry = get_dataset(NANO_VNA_INDEX);
    if (nvna_entry) {
        nvna_settings.get("Z0", nvna_entry->z0);
		nvna_settings.get("Enabled", nvna_entry->enabled);
		nvna_settings.get("Valid Ports", nvna_entry->valid_ports);
		nvna_settings.get("Left Colour", nvna_entry->line_style_l.colour);
		nvna_settings.get("Left Width", nvna_entry->line_style_l.width);
        nvna_settings.get("Left Style", nvna_entry->line_style_l.style);
        nvna_settings.get("Right Colour", nvna_entry->line_style_r.colour);
        nvna_settings.get("Right Width", nvna_entry->line_style_r.width);
        nvna_settings.get("Right Style", nvna_entry->line_style_r.style);
    }
	// Get the data type (S1P or S2P) if it exists, default to S1P if not.
	sp_settings.get("Number VNA Ports", number_ports_, 1);
}

// Save current settings for future sessions.
void sp_data::save_settings() {
    zc_settings settings;
    zc_settings sp_settings(&settings, "Data");
	sp_settings.clear(); // Clear any existing settings for this group before saving the current settings.
    // We only save settings for file datasets.
    int dataset_count = 0;
	for (auto dataset : datasets_) {;
        zc_settings* dataset_settings;
        if (dataset->source == SPDS_FILE) {
            dataset_settings = new zc_settings(&sp_settings, "Dataset " + std::to_string(dataset_count));
            dataset_settings->set("Filename", dataset->filename);
			dataset_settings->set("Enabled", dataset->enabled);
            dataset_settings->set("Valid Ports", dataset->valid_ports);
            dataset_settings->set("Left Colour", dataset->line_style_l.colour);
            dataset_settings->set("Left Width", dataset->line_style_l.width);
            dataset_settings->set("Right Colour", dataset->line_style_r.colour);
            dataset_settings->set("Right Width", dataset->line_style_r.width);
            dataset_settings->set("Left Style", dataset->line_style_l.style);
            dataset_settings->set("Right Style", dataset->line_style_r.style);
            dataset_count++;
        } else if (dataset->source == SPDS_ACTIVE) {
            dataset_settings = new zc_settings(&sp_settings, "nanoVNA");
            dataset_settings->set("Z0", dataset->z0);
			dataset_settings->set("Enabled", dataset->enabled);
            dataset_settings->set("Valid Ports", dataset->valid_ports);
            dataset_settings->set("Left Colour", dataset->line_style_l.colour);
            dataset_settings->set("Left Width", dataset->line_style_l.width);
            dataset_settings->set("Right Colour", dataset->line_style_r.colour);
            dataset_settings->set("Right Width", dataset->line_style_r.width);
            dataset_settings->set("Left Style", dataset->line_style_l.style);
            dataset_settings->set("Right Style", dataset->line_style_r.style);
		}
    }
	sp_settings.set("Dataset Count", dataset_count);
	sp_settings.set("Number VNA Ports", number_ports_);
}

// Reserve a new file dataset and return its index.
int sp_data::add_dataset(sp_data_source source) {
    sp_data_entry* entry = new sp_data_entry;
    datasets_.push_back(entry);
	size_t index = datasets_.size() - 1;
    entry->source = source;
    entry->filename = "";
    entry->valid_ports = 2;
    // Do not use COLOUR_CODE[9] as this is white!
    entry->line_style_l = zc_line_style(COLOUR_CODE[index % 9], 2, FL_SOLID);
    entry->line_style_r = zc_line_style(COLOUR_CODE[index % 9], 1, FL_SOLID);
    entry->z0 = default_z0_;
    return index;
}

// Get a reference to file dataset by index.
sp_data_entry* sp_data::get_dataset(int index) {
    if (index < 0 || index >= datasets_.size()) {
        // Raise an error.
        throw std::out_of_range("Dataset index out of range");
    }
    return datasets_[index];
}

// Get the number of file datasets currently stored.
int sp_data::get_dataset_count() const {
    return datasets_.size();
}

//! Read S-parameter data from a file and store it in the dataset.
//! \param index The index of the dataset to store the data in.
//! \return True if the data was successfully read and stored, false otherwise.
bool sp_data::read_data_from_file(sp_data_entry* entry) {
    if (entry->source != SPDS_FILE) {
        // Raise an error.
        throw std::invalid_argument("Dataset at index is not a file dataset");
    }
    return load_data_from_file(entry->filename, entry);
}

//! \brief Load S-parameter data from the specified file.
//! \param std::string filename The name of the file to read the data from.
//! \param sp_data_entry* entry The data entry to store the data in.
//! \return True if the data was successfully read and stored, false otherwise.
bool sp_data::load_data_from_file(const std::string& filename, sp_data_entry* entry) {
	// Check the file extension to check it's compatible.
    if (filename.size() >= 4) {
        std::string extension = filename.substr(filename.size() - 4);
        if (extension == ".s1p") {
            if (number_ports_ != 1) {
                // Raise a warning - the file extension indicates S1P data but the current data type is S2P. We will attempt to read it as S1P data but it may not be displayed correctly.
                if (status_) status_->misc_status(ST_ERROR, "File extension indicates S1P data but current data type is S2P: %s", filename.c_str());
            }
            entry->valid_ports = 1;
        }
        else if (extension == ".s2p") {
            if (number_ports_ != 2) {
                // Raise a warning - the file extension indicates S2P data but the current data type is S1P. We will attempt to read it as S2P data but it may not be displayed correctly.
                if (status_) status_->misc_status(ST_ERROR, "File extension indicates S2P data but current data type is S1P: %s", filename.c_str());
            }
            entry->valid_ports = 2;
        }
        else {
            // Unrecognized file extension, raise a warning and return false.
            status_->misc_status(ST_WARNING, "Unrecognized file extension: %s", extension.c_str());
            return false;
        }
    }
    else {
        // Filename too short to have a valid extension, raise a warning and return false.
        status_->misc_status(ST_WARNING, "Filename too short to have a valid extension: %s", filename.c_str());
        return false;
    }
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    // File format parameters.
    double multiplier = 1.0;
    sxp_data_format format = SXP_FORMAT_SDB;

    // Add comment lines.
	entry->notes = "";
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '!') {
            if (line.length() > 2) {
                // This is a comment line, add it to the notes for this dataset (removing the leading '!' and any leading whitespace).
                std::string comment = line.substr(2);
                entry->notes += comment + "\n";
			}
            continue;
        }
        else if (line[0] == '#') {
            // This is a header line, parse it to determine the file format.
            if (!parse_sxp_header(line, multiplier, format, entry->z0)) {
                // Failed to parse header, raise a warning and return false.
                status_->misc_status(ST_WARNING, "Failed to parse header line in file: %s", line.c_str());
                file.close();
                return false;
            }
            // Header successfully parsed, break out of the loop to start reading data lines.
			// We assume the header is after all pertinent comment lines.
            break;
        }
        else {
            // This is a data line, break out of the loop to start reading data lines.
            break;
        }
    }
    // Now read the data lines.
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '!') {
            // This is a comment line, skip it.
            continue;
        } else {
            // This is a data line, parse it to extract the frequency and S-parameters.
            sp_point point = parse_sxp_data_line(line, entry->valid_ports, multiplier, format);
            entry->data.insert(point);
        }
    }
    if (!file.eof()) {
		status_->misc_status(ST_WARNING, "Error reading file: %s", filename.c_str());
    }
    file.close();
    return true;
}

//! Parse SxP header line to determine the data format and frequency multiplier.
//! \param header_line The header line to parse.
//! \param multiplier Output parameter to store the frequency multiplier.
//! \param format Output parameter to store the data format.
//! \param z0 Output parameter to store the characteristic impedance.
//! \return True if the header was successfully parsed, false otherwise.
bool sp_data::parse_sxp_header(const std::string& header_line, double& multiplier, sxp_data_format& format, double& z0){
    // Example header: # HZ S RI R 50
    std::istringstream header_stream(header_line);
    std::string token;
    header_stream >> token; // Skip the '#'
    header_stream >> token;
    if (token == "GHZ") {
        multiplier = 1e9;
    }
    else if (token == "MHZ") {
        multiplier = 1e6;
    }
    else if (token == "KHZ") {
        multiplier = 1e3;
    }
    else if (token == "HZ") {
        multiplier = 1.0;
    }
    else {
        // Unrecognized frequency unit, raise a warning.
        status_->misc_status(ST_WARNING, "Unrecognized frequency unit in header: %s", token.c_str());
        return false;
    }
    header_stream >> token;
    if (token == "S") {
        // This is an S-parameter file, continue parsing.
    } else {
        // Unrecognized parameter type, raise a warning.
        status_->misc_status(ST_WARNING, "Unrecognized parameter type in header: %s", token.c_str());
        return false;
    }
    header_stream >> token;
    if (token == "RI") {
        format = SXP_FORMAT_SRI;
    }
    else if (token == "MA") {
        format = SXP_FORMAT_SMA;
    }
    else if (token == "DB") {
        format = SXP_FORMAT_SDB;
    }
    else {
        // Unrecognized format, raise a warning.
        status_->misc_status(ST_WARNING, "Unrecognized data format in header: %s", token.c_str());
        return false;
    }
    header_stream >> token;
    header_stream >> token;
    // Now read the impedance and check it is 50 ohms.
    double impedance;
    try {               
        impedance = std::stod(token);
    }
    catch (const std::exception& e) {
        // Invalid impedance value, raise a warning.
        status_->misc_status(ST_WARNING, "Invalid impedance value in header: %s", token.c_str());
        return false;
    }
    if (std::abs(impedance - default_z0_) > 1e-6) {
        // Impedance is not the default Z0, raise a warning.
        status_->misc_status(ST_WARNING, "Impedance in header is not the default Z0: %f vs %f", impedance, default_z0_);
        return false;
    }
    z0 = impedance;
    return true;
}

//! Parse SxP data line to extract the frequency and S-parameters.
//! \param data_line The data line to parse.
//! \param ports The number of valid ports (1 or 2) to determine how many S-parameters to parse.
//! \param multiplier The frequency multiplier determined from the header.
//! \param format The data format determined from the header.
//! \return A sp_point containing the frequency and S-parameters.
sp_point sp_data::parse_sxp_data_line(const std::string& data_line, int ports, double multiplier, sxp_data_format format) {
    std::istringstream data_stream(data_line);
    sp_point point;
    data_stream >> point.frequency;
    point.frequency *= multiplier;
    double param1, param2;
    data_stream >> param1 >> param2;
    point.sparams.s11 = convert_sparam(param1, param2, format);
    if (ports == 2) {
        data_stream >> param1 >> param2;
        point.sparams.s21 = convert_sparam(param1, param2, format);
        data_stream >> param1 >> param2;
        point.sparams.s12 = convert_sparam(param1, param2, format);
        data_stream >> param1 >> param2;
        point.sparams.s22 = convert_sparam(param1, param2, format);
    } else {
        point.sparams.s21 = { 0.0, 0.0 };
        point.sparams.s12 = { 0.0, 0.0 };
        point.sparams.s22 = { 0.0, 0.0 };
	}
    return point;
}

//! Convert S-parameter data from the specified format to the complex R/I format used internally.
//! \param param1 The first parameter (e.g. magnitude or real part).
//! \param param2 The second parameter (e.g. angle or imaginary part).
//! \param format The format of the input data.
//! \return The converted S-parameter as a complex number.
std::complex<double> sp_data::convert_sparam(double param1, double param2, sxp_data_format format) {
    switch (format) {
        case SXP_FORMAT_SRI:
            return std::complex<double>(param1, param2);
        case SXP_FORMAT_SMA: {
            double magnitude = param1;
            double angle_rad = param2 * zc::DEGREE_RADIAN;
            return std::polar(magnitude, angle_rad);
        }
        case SXP_FORMAT_SDB: {
            double magnitude = std::pow(10.0, param1 / 20.0);
            double angle_rad = param2 * zc::DEGREE_RADIAN;
            return std::polar(magnitude, angle_rad);
        }
        default:
            // Unrecognized format, raise an error
            throw std::invalid_argument("Unrecognized S-parameter data format");
    }
}

//! \brief Remove specified dataset by pointer.
//! \param entry The dataset to remove.
void sp_data::remove_dataset(sp_data_entry* entry) {
    for (auto it = datasets_.begin(); it != datasets_.end();) {
        if (*it == entry) {
            delete *it;
            it = datasets_.erase(it);
            break;
        }
        else {
            ++it;
        }
    }
}

//! \brief Store the specified dataset as a file.
//! \param index The index of the dataset to store.
//! \return True if the data was successfully stored, false otherwise.
bool sp_data::store_data_to_file(sp_data_entry* entry) {
    if (entry->source != SPDS_FILE && entry->source != SPDS_KEPT) {
        // Raise an error.
        throw std::invalid_argument("Dataset at index is not a valid dataset for writing");
    }
    // We will write the data in SRI format with a header line.
    std::ofstream file(entry->filename);
    if (!file.is_open()) {
        return false;
    }
    // Write comment lines with the notes for this dataset.
    std::istringstream notes_stream(entry->notes);
    std::string note_line;
    while (std::getline(notes_stream, note_line)) {
        file << "! " << note_line << "\n";
    }
    // Write header line.
    file << "# HZ S RI R " << entry->z0 << "\n";
    // Write the data lines.
    for (const auto& point : entry->data) {
        file << 
            std::setprecision(0) << std::fixed << point.frequency << " " << 
            std::setprecision(9) << std::fixed <<
            point.sparams.s11.real() << " " << point.sparams.s11.imag();
        if (entry->valid_ports >= 2) {
            file << " "
            << point.sparams.s21.real() << " " << point.sparams.s21.imag() << " "
            << point.sparams.s12.real() << " " << point.sparams.s12.imag() << " "
            << point.sparams.s22.real() << " " << point.sparams.s22.imag() << "\n";
        }
        else {
            file << "\n";
        }
    }
    file.close();
    return true;
}
