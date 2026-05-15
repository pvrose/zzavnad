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

//! Include the base class header file
#include "display.hpp"
#include "display_control.hpp"
#include "sp_data.hpp"	

#include "zc_graph_.h"

#include <cfloat>
#include <cmath>
#include <complex>
#include <cstring>

#include <fftw3.h>

namespace display_modes {

	class s11_tdr : public display {

	private:
		
		fftw_complex* in_array_ = nullptr;
		fftw_complex* out_array_ = nullptr;
		fftw_plan fft_plan_ = nullptr;

	public:

		s11_tdr(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
		}
			
		void configure_dm_params() override {
			params_.serial_name = "S11 TDR";
			params_.title = "S11 Time-domain reflectometry";

			axis_params_t x_axis_params;
			x_axis_params.outer_range = { 0.0, DBL_MAX };
			x_axis_params.default_range = { 0.0, 100e-9 };
			x_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			x_axis_params.unit = "s";
			x_axis_params.label = "Time";
			params_.axis_params[0] = x_axis_params;

			axis_params_t y_axis_params;
			y_axis_params.outer_range = { 0.0, DBL_MAX };
			y_axis_params.default_range = { 0.0, 1.0 };
			y_axis_params.unit_modifier = zc_graph_::modifier_t::NO_MODIFIER;
			y_axis_params.unit = "";
			y_axis_params.label = "Amplitude";
			params_.axis_params[1] = y_axis_params;

			axis_params_t yr_axis_params;
			yr_axis_params.outer_range = { 0.0, 100.0 };
			yr_axis_params.default_range = { 0.0, 100.0 };
			yr_axis_params.unit_modifier = zc_graph_::modifier_t::NO_MODIFIER;
			yr_axis_params.unit = "%";
			yr_axis_params.label = "Accumulated";
			params_.axis_params[2] = yr_axis_params;
		}

		void convert_sp_to_coords(
			const sp_data_entry& dataset,
			graph_data_map_t& coords,
			graph_data_ranges_t& ranges) override
		{
			// Convert Perform inverse Fourier transform of S11 data to get the time-domain reflectometry (TDR) response.
	        
			// Number of samples for the FFT.
			int num_samples = dataset.data.size();
			if (num_samples == 0) {
				return; // No data to convert
			}

			// Add padding to the number of samples to improve FFT resolution. 
			// This will add zeros to the end of the input array, 
			// which corresponds to zero-padding in the frequency domain and
			// thus interpolation in the time domain.
			int padding_factor = 16; // This can be adjusted based on the desired resolution and performance trade-off.
			int padded_num_samples = 1;
			while (padded_num_samples < num_samples * padding_factor) padded_num_samples *= 2;
			// Create input and output arrays for the FFT.
			in_array_ = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * padded_num_samples);
			out_array_ = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * padded_num_samples);

			// Create a plan for the inverse FFT and execute it.
			fft_plan_ = fftw_plan_dft_1d(padded_num_samples, in_array_, out_array_, FFTW_BACKWARD, FFTW_ESTIMATE);

			// Fill the input array with S11 data.
			size_t ix = 0;
			for (auto& entry : dataset.data) {
				in_array_[ix][0] = entry.sparams.s11.real(); // Real part of S11
				in_array_[ix][1] = entry.sparams.s11.imag(); // Imaginary part of S11
				++ix;
			}
			// Zero-pad the remaining input samples.
			memset(in_array_ + ix, 0, sizeof(fftw_complex) * (padded_num_samples - ix));
			fftw_execute(fft_plan_);

			// Create the data points for the graph from the FFT output.
			dm_data_set_t* tdr_coords = new dm_data_set_t;
			tdr_coords->style = dataset.line_style_l; // Use the line style from the dataset for the TDR response
			coords[1] = tdr_coords; // Assuming axis 1 is for the TDR response
			double f_max = dataset.data.rbegin()->frequency; // Maximum frequency from the dataset
			double f_min = dataset.data.begin()->frequency;   // Minimum frequency from the dataset
			double delta_f = (f_max - f_min) / (num_samples - 1);
			double f_sampling = num_samples * delta_f;     // Effective sampling rate for FFT
			double delta_t = 1.0 / f_sampling;   // Time step
			for (size_t ix = 0; ix < dataset.data.size() / 2; ++ix) {
				double time = ix * delta_t; // Time corresponding to this FFT bin
				double amplitude = sqrt(out_array_[ix][0] * out_array_[ix][0] + out_array_[ix][1] * out_array_[ix][1]); // Magnitude of the FFT output
				tdr_coords->data->emplace_back(time, amplitude);
				// Update the range for the x and y axes based on the TDR data.
				ranges[0] |= time; // Update x-axis range with time values
				ranges[1] |= amplitude; // Update y-axis range with amplitude values
			}
			// Normalise and convert to percentage for the accumulated response on the right Y axis.
			dm_data_set_t* accumulated_coords = new dm_data_set_t;
			accumulated_coords->style = dataset.line_style_r; // Use the line style from the dataset for the accumulated response
			coords[2] = accumulated_coords; // Assuming axis 2 is for the accumulated response
			double total = 0.0;
			for (size_t ix = 0; ix < tdr_coords->data->size(); ++ix) {
				total += tdr_coords->data->at(ix).second;
			}
			double accumulated = 0.0;
			for (size_t ix = 0; ix < tdr_coords->data->size(); ++ix) {
				accumulated += tdr_coords->data->at(ix).second;
				double percentage = (accumulated / total) * 100.0; // Convert to percentage
				accumulated_coords->data->emplace_back(tdr_coords->data->at(ix).first, percentage);
			}

			// Clean up FFT resources.
			fftw_destroy_plan(fft_plan_);
			fftw_free(in_array_);
			fftw_free(out_array_);
		}

		zc_graph_* create_graph(int X, int Y, int W, int H) override {
			return new zc_graph_cartesian_2y(X, Y, W, H);
		}

		graph_data_ranges_t get_all_data_ranges() override {
			graph_data_ranges_t ranges;
			ranges[0] = get_range(0);
			ranges[1] = get_range(1);
			ranges[2] = get_range(2);
			return ranges;
		}
	};
};

