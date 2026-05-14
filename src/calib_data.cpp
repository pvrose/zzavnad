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

#include "calib_data.hpp"

#include "sp_data.hpp"

#include "zc_settings.h"

#include <algorithm>
#include <complex>
#include <cstdint>
#include <set>
#include <string>

// Constructor for the calib_data class.
calib_data::calib_data() : status_(CALIB_NONE) {
	// Initialize calibration data sets as empty.
	calib_data_entries_.clear();
	load_settings();
}

// Destructor for the calib_data class.
calib_data::~calib_data() {
	save_settings();
}

// Load settings from previous sessions.
void calib_data::load_settings() {
	zc_settings settings;
	zc_settings calib_settings(&settings, "Calibration Data");
	calib_settings.get("Directory", calib_directory_, std::string("")); // Default to empty string (no directory)
	if (!calib_directory_.empty()) {
		// If a directory was saved in settings, attempt to load calibration data from that directory.
		load_calibration_data(calib_directory_);
	}
}

// Save current settings for future sessions.
void calib_data::save_settings() const {
	zc_settings settings;
	zc_settings calib_settings(&settings, "Calibration Data");
	calib_settings.set("Directory", calib_directory_);
	settings.flush();
}

// Set calibration data for a specific type (open, short, load).
void calib_data::set_calibration_data(sp_data_source type, sp_data_entry* entry) {
	calib_data_entries_[type] = entry;
	if (type == SPDS_CALIB_O) (uint32_t&)status_ = status_ | CALIB_OPEN;
	else if (type == SPDS_CALIB_S) (uint32_t&)status_ = status_ | CALIB_SHORT;
	else if (type == SPDS_CALIB_L) (uint32_t&)status_ = status_ | CALIB_LOAD;
	else if (type == SPDS_CALIB_T) (uint32_t&)status_ = status_ | CALIB_THROUGH;
	else if (type == SPDS_CALIB_I) (uint32_t&)status_ = status_ | CALIB_ISOLATION;
}

// Get the calibration data entry for a specific type (open, short, load).
sp_data_entry* calib_data::get_calibration_data(sp_data_source type) const {
	auto it = calib_data_entries_.find(type);
	if (it != calib_data_entries_.end()) {
		return it->second;
	} else {
		return nullptr; // Invalid type or not set
	}
}

// Clear all calibration data and reset the status.
void calib_data::clear_calibration_data() {
	// Remove the calibration datasets from sp_data and clear the entries.
	for (const auto& pair : calib_data_entries_) {
		sp_data_entry* entry = pair.second;
		if (entry != nullptr) {
			sp_data_->remove_dataset(entry);
		}
	}
	status_ = CALIB_NONE;
}

// Save the current calibration data to files (e.g. S11 files).
void calib_data::save_calibration_data(const std::string& directory) {
	// Remember the directory to save to settimgs
	calib_directory_ = directory;
	// Save the open, short, and load calibration data to separate files in the specified directory.
	for (const auto& pair : calib_data_entries_) {
		sp_data_entry* entry = pair.second;
		if (entry != nullptr) {
			if (entry->source == SPDS_CALIB_O) {
				entry->filename = directory + "/calib_open.s1p";
			} else if (entry->source == SPDS_CALIB_S) {
				entry->filename = directory + "/calib_short.s1p";
			} else if (entry->source == SPDS_CALIB_L) {
				entry->filename = directory + "/calib_load.s1p";
			} else if (entry->source == SPDS_CALIB_T) {
				entry->filename = directory + "/calib_through.s1p";
			} else if (entry->source == SPDS_CALIB_I) {
				entry->filename = directory + "/calib_isolation.s1p";
			}
		}
		sp_data_->store_data_to_file(entry);
	}
}

// Load calibration data from files (e.g. S11 files).
void calib_data::load_calibration_data(const std::string& directory) {

	bool ok = true;
	calib_data_entries_[SPDS_CALIB_O] = sp_data_->get_dataset(sp_data_->add_dataset(SPDS_CALIB_O));
	sp_data_entry* open_entry = calib_data_entries_[SPDS_CALIB_O];
	open_entry->filename = directory + "/calib_open.s1p";
	ok &= sp_data_->read_data_from_file(open_entry);
	if (ok) (uint32_t&)status_ = status_ | CALIB_OPEN;

	calib_data_entries_[SPDS_CALIB_S] = sp_data_->get_dataset(sp_data_->add_dataset(SPDS_CALIB_S));
	sp_data_entry* short_entry = calib_data_entries_[SPDS_CALIB_S];
	short_entry->filename = directory + "/calib_short.s1p";
	ok &= sp_data_->read_data_from_file(short_entry);
	if (ok) (uint32_t&)status_ = status_ | CALIB_SHORT;

	calib_data_entries_[SPDS_CALIB_L] = sp_data_->get_dataset(sp_data_->add_dataset(SPDS_CALIB_L));
	sp_data_entry* load_entry = calib_data_entries_[SPDS_CALIB_L];
	load_entry->filename = directory + "/calib_load.s1p";
	ok &= sp_data_->read_data_from_file(load_entry);
	if (ok) (uint32_t&)status_ = status_ | CALIB_LOAD;

	if (sp_data_->get_number_ports() == 2) {
		calib_data_entries_[SPDS_CALIB_T] = sp_data_->get_dataset(sp_data_->add_dataset(SPDS_CALIB_T));
		sp_data_entry* through_entry = calib_data_entries_[SPDS_CALIB_T];
		through_entry->filename = directory + "/calib_through.s1p";
		ok &= sp_data_->read_data_from_file(through_entry);
		if (ok) (uint32_t&)status_ = status_ | CALIB_THROUGH;

		calib_data_entries_[SPDS_CALIB_I] = sp_data_->get_dataset(sp_data_->add_dataset(SPDS_CALIB_I));
		sp_data_entry* isolation_entry = calib_data_entries_[SPDS_CALIB_I];
		isolation_entry->filename = directory + "/calib_isolation.s1p";
		ok &= sp_data_->read_data_from_file(isolation_entry);
		if (ok) (uint32_t&)status_ = status_ | CALIB_ISOLATION;
	}

    if (ok) {
		calib_directory_ = directory; // Update the calibration data directory if loading was successful.
	} else {
		clear_calibration_data();
	}
}

// Calibrate the provided S-parameter data using the available calibration data.
void calib_data::calibrate(
	sp_point& point) const {
	if (!(status_ & CALIB_USE_IT)) {
		return; // Cannot calibrate if calibration data is not complete and converted to error terms.
	}
	// Get error terms at the specified frequency by looking up the calibration data and interpolating if necessary.
	e_point e_values = interpolate_error_terms(point.frequency);
	apply_calibration_correction(point.sparams, e_values);
}

// Check if the calibration data is valid for the given frequency range.
bool calib_data::is_calibration_valid(double minf, double maxf) const {
	if (!(status_ & CALIB_CONVERTED)) {
		return false; // Calibration data is not complete.
	}
	// Check if there is significant overlap between the frequency range of the error terms and the specified frequency range.
	double calib_minf = error_terms_.begin()->frequency; // Minimum frequency in the calibration data
	double calib_maxf = error_terms_.rbegin()->frequency; // Maximum frequency in the calibration data
	double overlap_minf = std::max(minf, calib_minf);
	double overlap_maxf = std::min(maxf, calib_maxf);
	double overlap_range = overlap_maxf - overlap_minf;
	double required_overlap = 0.9 * (maxf - minf); // Require at least 90% overlap for valid calibration.
	return overlap_range >= required_overlap;
}

// Calculate the error terms at the specified frequency by looking up the calibration data and interpolating if necessary.
calib_data::e_point calib_data::interpolate_error_terms(double frequency) const {
	// Find the two calibration points that bracket the specified frequency.
	auto upper_it = error_terms_.lower_bound({frequency, {}, {}, {}});
	if (upper_it == error_terms_.end()) {
		// If the specified frequency is above the range of the calibration data, return the error terms at the maximum frequency.
		// TODO : Consider whether extrapolation beyond the calibration data range would be more appropriate than just returning the edge values.
		return *error_terms_.rbegin();
	} else if (upper_it == error_terms_.begin()) {
		// If the specified frequency is below the range of the calibration data, return the error terms at the minimum frequency.
		// TODO : Consider whether extrapolation beneath the calibration data range would be more appropriate than just returning the edge values.
		return *error_terms_.begin();
	}
	else if (upper_it->frequency == frequency) {
		// If the specified frequency exactly matches a calibration point, return the error terms at that point.
		return *upper_it;
	}
	else {
		// Otherwise, interpolate between the two bracketing points.
		// TODO: Consider whether linear interpolation is sufficient or if a higher-order interpolation method 
		// would provide better accuracy for the error terms.
		auto lower_it = std::prev(upper_it);
		const e_point& lower = *lower_it;
		const e_point& upper = *upper_it;
		double t = (frequency - lower.frequency) / (upper.frequency - lower.frequency);
		e_point interpolated;
		interpolated.frequency = frequency;
		interpolated.e00 = lower.e00 + t * (upper.e00 - lower.e00);
		interpolated.e11 = lower.e11 + t * (upper.e11 - lower.e11);
		interpolated.e10e01 = lower.e10e01 + t * (upper.e10e01 - lower.e10e01);
		return interpolated;
	}
}

// Apply the calibration correction to the raw S-parameters using the provided error terms.
void calib_data::apply_calibration_correction(sp_params& raw_sparams, const e_point& e_values) const {
	// Apply the 1-port error correction equations to the raw S-parameters using the error terms.
	sp_params corrected;
	const std::complex<double>& s11 = raw_sparams.s11;
	const std::complex<double>& e00 = e_values.e00;
	const std::complex<double>& e11 = e_values.e11;
	const std::complex<double>& e10e01 = e_values.e10e01;
	corrected.s11 = (e00 - s11) / (e11 * (s11 - e00) + e10e01);
	raw_sparams = corrected;
}

// Caclulate the error terms from the open, short, and load calibration data and store them in the error_terms_ set for later interpolation during calibration correction.
void calib_data::calculate_error_terms() {
	if (status_ != CALIB_VALID1) {
		return; // Cannot calculate error terms if calibration data is not complete and valid.
	}
	error_terms_.clear();
	// Step through each calibration data set in parallel and calculate the error terms at each frequency point where all three calibration datasets have data.
	// The three datasets should have exactly the same frequency points, but we will step through them in parallel.
	sp_data_entry* open_entry = calib_data_entries_[SPDS_CALIB_O];
	sp_data_entry* short_entry = calib_data_entries_[SPDS_CALIB_S];
	sp_data_entry* load_entry = calib_data_entries_[SPDS_CALIB_L];

	auto it_o = open_entry->data.begin();
	auto it_s = short_entry->data.begin();
	auto it_l = load_entry->data.begin();
	while (it_o != open_entry->data.end() && it_s != short_entry->data.end() && it_l != load_entry->data.end()) {
		if (it_o->frequency == it_s->frequency && it_s->frequency == it_l->frequency) {
			// If the frequency points match, calculate the error terms at this frequency and add them to the set.
			e_point e;
			e.frequency = it_o->frequency;
			const std::complex<double>& s11_o = it_o->sparams.s11;
			const std::complex<double>& s11_s = it_s->sparams.s11;
			const std::complex<double>& s11_l = it_l->sparams.s11;
			const std::complex<double> two = { 2.0, 0.0 };
			// Calculate e00, e11, and e10*e01 error terms using the open, short, and load S-parameters at this frequency.
			e.e00 = s11_l;
			e.e11 = (two * s11_l - s11_s - s11_o) / (s11_s - s11_o);
			e.e10e01 = (two * (s11_l - s11_o) * (s11_l - s11_s)) / (s11_s - s11_o);
			error_terms_.insert(e);
		}
		// Step the iterator for the dataset with the lowest frequency point to keep them in sync.
		bool step_o = false;
		bool step_s = false;
		bool step_l = false;
		if (it_o->frequency <= it_s->frequency && it_o->frequency <= it_l->frequency) step_o = true;
		if (it_s->frequency <= it_o->frequency && it_s->frequency <= it_l->frequency) step_s = true;
		if (it_l->frequency <= it_o->frequency && it_l->frequency <= it_s->frequency) step_l = true;
		if (step_o) ++it_o;
		if (step_s) ++it_s;
		if (step_l) ++it_l;
	}
	// After calculating the error terms, update the calibration status to valid if we have successfully calculated
	// error terms for all frequency points in the calibration data.
	if (!error_terms_.empty()) {
		(uint32_t&)status_ |= CALIB_CONVERTED; // Set the converted flag to indicate that error terms have been calculated.
	}
}

// Set/unset the flag indicating whether the calibration data should be used.
void calib_data::use_calibration(bool use_calib) {
	if (use_calib) {
		if (status_ & CALIB_CONVERTED) {
			(uint32_t&)status_ |= CALIB_USE_IT; // Set the flag to indicate that calibration data is available for use.
		}
	}
	else {
		(uint32_t&)status_ &= ~CALIB_USE_IT; // Unset the flag to indicate that calibration data should not be used.
	}
}