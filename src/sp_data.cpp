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
    load_settings();
	// If any datasets were loaded from settings, attempt to read the data for them.
    for (int i = 0; i < datasets_.size(); i++) {
        sp_data_entry* entry = datasets_[i];
        if (entry->source == SP_DATA_SOURCE_FILE) {
            if (!read_data_from_file(i)) {
                if (status_) status_->misc_status(ST_ERROR, "Failed to read data from file: %s", entry->filename.c_str());
            }
        }
        else if (entry->source == SP_DATA_SOURCE_VNA) {
            if (!acquire_data_from_vna()) {
                if (status_) status_->misc_status(ST_ERROR, "Failed to acquire data from VNA");
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

// Load settings from previous sessions.
void sp_data::load_settings() {
    zc_settings settings;
    zc_settings sp_settings(&settings, "Data");
    // Load specific settings for sp_data here.
    int dataset_count;
	sp_settings.get("Dataset Count", dataset_count, 0);
    for (int i = 0; i < dataset_count; i++) {
		std::string dataset_name = i == 0 ? "nanoVNA" : "Dataset " + std::to_string(i);
        zc_settings dataset_settings(&sp_settings, dataset_name);
        sp_data_entry* entry = new sp_data_entry;
        if (i != 0) {
            entry->source = SP_DATA_SOURCE_FILE;
            dataset_settings.get("Filename", entry->filename, std::string(""));
        }
        else {
            entry->source = SP_DATA_SOURCE_VNA;
            nanoVNA_index_ = i;
        }
        dataset_settings.get("Valid Ports", entry->valid_ports, 2);
        dataset_settings.get("Left Colour", entry->line_style_l.colour, FL_BLUE);
		dataset_settings.get("Left Thickness", entry->line_style_l.thickness, 2);
		dataset_settings.get("Left Style", entry->line_style_l.style, (int)FL_SOLID);
		dataset_settings.get("Right Colour", entry->line_style_r.colour, FL_GREEN);
		dataset_settings.get("Right Thickness", entry->line_style_r.thickness, 2);
		dataset_settings.get("Right Style", entry->line_style_r.style, (int)FL_SOLID);
        datasets_.push_back(entry);
    }
    if (datasets_.size() == 0) {
        sp_data_entry* entry = new sp_data_entry;
        entry->source = SP_DATA_SOURCE_VNA;
        entry->valid_ports = 2;
        entry->line_style_l = { FL_BLUE, 2, 0 };
        datasets_.push_back(entry);
    }
}

// Save current settings for future sessions.
void sp_data::save_settings() {
    zc_settings settings;
    zc_settings sp_settings(&settings, "Data");
    int dataset_count = datasets_.size();
    sp_settings.set("Dataset Count", dataset_count);
    for (int i = 0; i < dataset_count; i++) {
        sp_data_entry* entry = datasets_[i];
        zc_settings* dataset_settings;
        if (entry->source == SP_DATA_SOURCE_VNA) {
            dataset_settings = new zc_settings(&sp_settings, "nanoVNA");
        }
        else {
            dataset_settings = new zc_settings(&sp_settings, "Dataset " + std::to_string(i));
            dataset_settings->set("Filename", entry->filename);
        }
        dataset_settings->set("Valid Ports", entry->valid_ports);
        dataset_settings->set("Left Colour", entry->line_style_l.colour);
        dataset_settings->set("Left Thickness", entry->line_style_l.thickness);
        dataset_settings->set("Right Colour", entry->line_style_r.colour);
        dataset_settings->set("Right Thickness", entry->line_style_r.thickness);
    }
}

// Reserve a new file dataset and return its index.
int sp_data::add_dataset() {
    sp_data_entry* entry = new sp_data_entry;
    entry->source = SP_DATA_SOURCE_FILE;
    entry->filename = "";
    entry->valid_ports = 2;
    entry->line_style_l = zc_graph_line_t{FL_BLUE, 2, FL_SOLID};
    entry->line_style_r = zc_graph_line_t{FL_RED, 2, FL_SOLID};
    datasets_.push_back(entry);
    return datasets_.size() - 1;
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
bool sp_data::read_data_from_file(int index) {
    if (index < 0 || index >= datasets_.size()) {
        // Raise an error.
        throw std::out_of_range("Dataset index out of range");
    }
    sp_data_entry* entry = datasets_[index];
    if (entry->source != SP_DATA_SOURCE_FILE) {
        // Raise an error.
        throw std::invalid_argument("Dataset at index is not a file dataset");
    }
    return load_data_from_file(entry->filename, entry);
}

//! Acquire S-parameter data from a VNA and store it in the dataset.
//! \return True if the data was successfully acquired and stored, false otherwise.
bool sp_data::acquire_data_from_vna() {
    // TODO: Implement VNA data acquisition.
    return false;
}


//! \brief Load S-parameter data from the specified file.
//! \param std::string filename The name of the file to read the data from.
//! \param sp_data_entry* entry The data entry to store the data in.
//! \return True if the data was successfully read and stored, false otherwise.
bool sp_data::load_data_from_file(const std::string& filename, sp_data_entry* entry) {
    // Set S1P or S2P based on the filename extension.
    if (filename.size() >= 4) {
        std::string extension = filename.substr(filename.size() - 4);
        if (extension == ".s1p") {
            entry->valid_ports = 1;
        }
        else if (extension == ".s2p") {
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

    // Skip comment lines.
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '!') {
            // This is a comment line, skip it.
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
    if (std::abs(impedance - 50.0) > 1e-6) {
        // Impedance is not 50 ohms, raise a warning.
        status_->misc_status(ST_WARNING, "Impedance in header is not 50 ohms: %f", impedance);
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

//! \brief Remove all file datasets.
void sp_data::clear_file_datasets() {
    for (auto it = datasets_.begin(); it != datasets_.end();) {
        if ((*it)->source == SP_DATA_SOURCE_FILE) {
            delete *it;
            it = datasets_.erase(it);
        }
        else {
            ++it;
        }
    }
}

//! \brief Remove all undisplayed file datasets.
void sp_data::clear_undisplayed_file_datasets() {
    for (auto it = datasets_.begin(); it != datasets_.end();) {
        if ((*it)->source == SP_DATA_SOURCE_FILE && !(*it)->enabled) {
            delete *it;
            it = datasets_.erase(it);
        }
        else {
            ++it;
        }
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
