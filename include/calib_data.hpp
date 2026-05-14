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

#include "sp_data.hpp"

#include <complex>
#include <cstdint>
#include <map>
#include <set>

//! \brief Calibration data and management.
//! 
//! This class manages the calibration data for the nanoVNA, including the
//! open, short, and load calibration measurements. 
//! 
//! Raw calibrattion data can be read from saved data in S11 files or
//! acquired directly from the nanoVNA. The class converts these
//! calibration measurements into the format needed for calibration correction.
//! 
class calib_data {

public:

	//! \brief Constructor for the calib_data class.
	calib_data();

	//! \brief Destructor for the calib_data class.
	~calib_data();

	//! \brief Load settings from previous sessions.
	void load_settings();

	//! \brief Save current settings for future sessions.
	void save_settings() const;

	//! \brief Data status.
	enum calib_status : uint32_t {
		CALIB_NONE = 0,    //!< No calibration data available.
		CALIB_OPEN = 1,    //!< Open circuit calibration data available.
		CALIB_SHORT = 2,   //!< Short circuit calibration data available.
		CALIB_LOAD = 4,    //!< Load calibration data available.
		CALIB_VALID1 = 7,   //!< All calibration data (open, short, load) available and valid.
		CALIB_THROUGH = 8,  //!< Through calibration data available (for 2-port calibration).
		CALIB_ISOLATION = 16, //!< Isolation calibration data available (for 2-port calibration).
		CALIB_VALID2 = 31,  //!< All calibration data (open, short, load) available and valid, and through/isolation data available if needed for 2-port calibration.
		CALIB_CONVERTED = 32, //!< Calibration data has been converted to error terms and is ready for use in calibration correction (1-port calibration).
		
	};

	//! \brief Get the current calibration status.
	calib_status get_calibration_status() const { return status_; }

	//! \brief Set calibration data for a specific type (open, short, load).
	void set_calibration_data(sp_data_source type, sp_data_entry* entry);

	//! \brief Get the calibration data entry for a specific type (open, short, load).
	sp_data_entry* get_calibration_data(sp_data_source type) const;

	//! \brief Calibrate the provided S-parameter data using the available 
	//! calibration data.
	//! 
	//! The frequency is looked up in the calibration data and the
	//! corresponding error terms are interpolated 
	//! to apply the calibration correction.
	void calibrate(sp_params& sparams, double frequency) const;

	//! \brief Check if the calibration data is valid for the given frequency range.
	//! 
	//! Returns true if there is significant overap between the frequency range
	//! of the calibration data and the specified frequency range.
	bool is_calibration_valid(double minf, double maxf) const;

	//! \brief Clear all calibration data and reset the status.
	void clear_calibration_data();

	//! \brief Save the current calibration data to files (e.g. S11 files).
	//! 
	//! Opens a file dialog to select a directory and saves the open, short,
	//! and load calibration data to separate files in that directory.
	void save_calibration_data(std::string directory) const;

	//! \brief Load calibration data from files (e.g. S11 files).
	//! 
	//! Opens a file dialog to select a directory and loads the open, short, 
	//! and load calibration data from separate files in that directory.
	void load_calibration_data(std::string directory);

private:
	//! \brief Current calibration status.
	calib_status status_;

	//! \brief Calibration data entries for open, short, and load measurements.
	//!
	//! Mapped by calib_status onto sp_data entries.
	std::map<sp_data_source, sp_data_entry*> calib_data_entries_;

	//! \brief Calibration data directory.
	std::string calib_directory_;

	//! \brief Individual set of error terms for a single frequency point, used for calibration correction.
	//! \brief Single frequency point S-parameter data structure.
	struct e_point {
		double frequency;    //!< Frequency in Hz
		std::complex<double> e00;   //!< e00 error term at this frequency
		std::complex<double> e11;   //!< e11 error term at this frequency
		std::complex<double> e10e01; //!< e10*e01 error term at this frequency

		//! Declare a '<' operator to allow sorting by frequency.
		bool operator< (const e_point& other) const {
			return frequency < other.frequency;
		}
	};

	//! \brief An ordered set of error term points, sorted by frequency.
	std::set<e_point> error_terms_;

	//! \brief Calculate the error terms based on the available 
	//! calibration data.
	//! 
	//! Calculates the error terms (e00, e11, e10*e01) for each
	//! frequency point in the calibration data and stores them 
	//! in the error_terms_ set for later interpolation and
	//! application of calibration correction.
	void calculate_error_terms();

	//! \brief Interpolate the error terms for a given frequency.
	//! 
	//! Currently uses linear interpolation between the two nearest 
	//! frequency points in the error_terms_ set.
	e_point interpolate_error_terms(double frequency) const;

	//! \brief Apply the calibration correction to the raw S-parameters
	//! using the interpolated error terms.
	void apply_calibration_correction(sp_params& sparams, const e_point& error_terms) const;

};

extern calib_data* calib_data_; //!< Global pointer to the calib_data instance used by the application.