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
#pragma once

#include <complex>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "zc_graph.h"

//! \file sp_data.hpp
//! \brief Provides the basic data types used for the S-parameter data.

//! \brief 2-port S-parameter data structure.
struct sp_params {
	std::complex<double> s11; //!< S11 parameter
	std::complex<double> s12; //!< S12 parameter
	std::complex<double> s21; //!< S21 parameter
	std::complex<double> s22; //!< S22 parameter
};

//! \brief Single frequency point S-parameter data structure.
struct sp_point {
    double frequency;    //!< Frequency in Hz
    sp_params sparams;   //!< S-parameters at this frequency

    //! Declare a '<' operator to allow sorting by frequency.
    bool operator< (const sp_point& other) const {
        return frequency < other.frequency;
    }
};

//! \brief An ordered set of S-parameter points, sorted by frequency.
typedef std::set<sp_point> sp_data_set;

//! \brief Source of data sets.
enum sp_data_source : uint8_t {
    SP_DATA_SOURCE_FILE, //!< Data read from a file
    SP_DATA_SOURCE_VNA,  //!< Data acquired from a VNA
};

//! \brief A structure to hold S-parameter data along with its source information.
struct sp_data_entry {
    sp_data_set data;          //!< The S-parameter data points
    sp_data_source source;     //!< The source of the data
    std::string filename;      //!< Optional filename if data was read from a file
    int valid_ports;           //!< Number of valid ports (1 or 2)
    zc_graph_line_t line_style_l;       //!< Line configuration for left hand data (e.g. colour, thickness)
    zc_graph_line_t line_style_r;       //!< Line configuration for right hand data (e.g. colour, thickness)
    double z0;                 //!< Characteristic impedance of the system for this data (e.g. 50 ohms)
    bool enabled;              //!< Whether this dataset is enabled for display
};

//! \brief SxP data formats
enum sxp_data_format : uint8_t {
    SXP_FORMAT_SDB, //!< S-parameter data in dB/Angle format
    SXP_FORMAT_SMA, //!< S-parameter data in Magnitude/Angle format
    SXP_FORMAT_SRI, //!< S-parameter data in Real/Imaginary format
};

//! \brief Class to manage all the data sets and their sources.
class sp_data {

public:
    //! Constructor for the sp_data class.
    sp_data();
    //! Destructor for the sp_data class.
    ~sp_data();

    //! Load settings from previous sessions.
    void load_settings();
    //! Save current settings for future sessions.
    void save_settings();

    //! Reserve a new file dataset and return its index.
    int add_dataset();

    //! Get a reference to file dataset by index.
    sp_data_entry* get_dataset(int index);

    //! Get the number of file datasets currently stored.
    int get_dataset_count() const;

    //! Read S-parameter data from a file and store it in the dataset.
    //! \param index The index of the dataset to store the data in.
    //! \return True if the data was successfully read and stored, false otherwise.
    bool read_data_from_file(int index);

    //! Acquire S-parameter data from a VNA and store it in the dataset.
    //! \return True if the data was successfully acquired and stored, false otherwise.
    bool acquire_data_from_vna();

    //! \brief Remove all file datasets.
    void clear_file_datasets();

    //! \brief Remove all undisplayed file datasets.
    void clear_undisplayed_file_datasets();

    //! \brief Remove specified dataset by pointer.
    //! \param entry The dataset to remove.
    void remove_dataset(sp_data_entry* entry);

private:

    //! \brief Load S-parameter data from the specified file.
    //! \param std::string filename The name of the file to read the data from.
    //! \param sp_data_entry* entry The data entry to store the data in.
    //! \return True if the data was successfully read and stored, false otherwise.
    bool load_data_from_file(const std::string& filename, sp_data_entry* entry);

    //! \brief Convert S-parameter data from the specified format to the complex R/I format used internally.
    //! \param param1 The first parameter (e.g. magnitude or real part).
    //! \param param2 The second parameter (e.g. angle or imaginary part).
    //! \param format The format of the input data.
    //! \return The converted S-parameter as a complex number.
    std::complex<double> convert_sparam(double param1, double param2, sxp_data_format format);

    //! \brief parse SxP header line to determine the data format and frequency multiplier.
    //! \param header_line The header line to parse.
    //! \param multiplier Output parameter to store the frequency multiplier.
    //! \param format Output parameter to store the data format.
    //! \param z0 Output parameter to store the characteristic impedance.
    //! \return True if the header was successfully parsed, false otherwise.
    bool parse_sxp_header(const std::string& header_line, double& multiplier, sxp_data_format& format, double& z0);

    //! \brief parse SxP data line to extract the frequency and S-parameters.
    //! \param data_line The data line to parse.
    //! \param ports The number of valid ports (1 or 2) to determine how many S-parameters to parse.
    //! \param multiplier The frequency multiplier determined from the header.
    //! \param format The data format determined from the header.
    //! \return A sp_point containing the frequency and S-parameters
    sp_point parse_sxp_data_line(const std::string& data_line, int ports, double multiplier, sxp_data_format format);

    std::vector<sp_data_entry*> datasets_; //!< Vector to hold all datasets etc.

    int nanoVNA_index_ = -1; //!< Index of the dataset containing nanoVNA data, or -1 if no nanoVNA data is stored.

};

extern sp_data* sp_data_; //!< Global pointer to the sp_data instance used by the application.