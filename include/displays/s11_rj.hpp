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
#include "sp_data.hpp"	

#include "zc_graph_.h"

#include <cfloat>
#include <complex>
#include <map>

namespace display_modes {

	class s11_rj : public display {

	public:

		s11_rj(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
		}

		void configure_dm_params() override {
			params_.serial_name = "S11 R v jX";
			params_.title = "S11 Resistance vs Reactance";

			axis_params_t x_axis_params;
			x_axis_params.outer_range = { -DBL_MAX, DBL_MAX };
			x_axis_params.inner_range = { -50.0F, 50.0F };
			x_axis_params.default_range = { -500.0F, 500.0F };
			x_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			x_axis_params.unit = "\xCE\xA9";
			x_axis_params.label = "S11 Resistance";
			params_.axis_params[0] = x_axis_params;

			axis_params_t y_axis_params;
			y_axis_params.outer_range = { -DBL_MAX, DBL_MAX };
			y_axis_params.inner_range = { -50.0F, 50.0F };
			y_axis_params.default_range = { -500.0F, 500.0F };
			y_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			y_axis_params.unit = "\xCE\xA9";
			y_axis_params.label = "S11 Reactance";
			params_.axis_params[1] = y_axis_params;

		}

		void convert_sp_point(
			const sp_point& point,
			double Z0,
			zc_graph_::data_point_t& pixel) const
		{
			::std::complex<double> ONE(1.0, 0.0);
			::std::complex<double> s11 = point.sparams.s11;
			// Get Z0 from the dataset.
			::std::complex<double> z0 = Z0;
			::std::complex<double> z = z0 * (ONE + s11) / (ONE - s11); // Convert S11 to impedance.
			pixel.first = z.real(); // Resistance
			pixel.second = z.imag(); // Reactance
		}

		void convert_sp_to_coords(
			const sp_data_entry& dataset,
			graph_data_map_t& coords,
			graph_data_ranges_t& ranges) override
		{
			if (dataset.valid_ports < 1) {
				return; // Not enough valid ports for S11
			}
			// We have one data set to populate for this display mode, which will be used for both resistance and reactance since they share the same X values (frequency).
			dm_data_set_t* data_coords = new dm_data_set_t;
			data_coords->style = dataset.line_style_l; 
			coords[1] = data_coords;
			for (const sp_point& point : dataset.data) {
				zc_graph_::data_point_t point_coords;
				convert_sp_point(point, dataset.z0, point_coords);
				data_coords->data->push_back(point_coords);
				// Update axis ranges based on the data points.
				ranges[0] |= point_coords.first;
				ranges[1] |= point_coords.second;
			}
		}

		zc_graph_* create_graph(int X, int Y, int W, int H) override {
			return new zc_graph_cart_overlay(X, Y, W, H);
		}

		graph_data_ranges_t get_all_data_ranges() override {
			graph_data_ranges_t ranges;
			ranges[0] = get_range(0);
			ranges[1] = get_range(1);
			return ranges;
		}
	};

};