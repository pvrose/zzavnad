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

	class s11_rx : public display {

	public:

		s11_rx(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
		}

		void configure_dm_params() override {
			params_.serial_name = "S11 R+jX";
			params_.title = "S11 Resistance and Reactance vs frequency";

			axis_params_t x_axis_params;
			x_axis_params.outer_range = { 0.0F, DBL_MAX };
			x_axis_params.inner_range = { 1e6F, 30e6F };
			x_axis_params.default_range = { 1e6F, 30e6F };
			x_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			x_axis_params.unit = "Hz";
			x_axis_params.label = "Frequency";
			params_.axis_params[0] = x_axis_params;

			axis_params_t yl_axis_params;
			yl_axis_params.outer_range = { 0.0F, 1000.0F };
			yl_axis_params.inner_range = { 0.0F, 1000.0F };
			yl_axis_params.default_range = { 0.0F, 1000.0F };
			yl_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			yl_axis_params.unit = "\xCE\xA9";
			yl_axis_params.label = "S11 Resistance";
			params_.axis_params[1] = yl_axis_params;

			axis_params_t yr_axis_params;
			yr_axis_params.outer_range = { -1000.0F, 1000.0F };
			yr_axis_params.inner_range = { -1000.0F, 1000.0F };
			yr_axis_params.default_range = { -1000.0F, 1000.0F };
			yr_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			yr_axis_params.unit = "\xCE\xA9";
			yr_axis_params.label = "S11 Reactance";
			params_.axis_params[2] = yr_axis_params;
		}

		void add_markers() override {
			add_frequency_markers();
		}

		void convert_sp_point(
			const sp_point& point,
			double Z0,
			zc_graph_::data_point_t& point_l,
			zc_graph_::data_point_t& point_r) const
		{
			::std::complex<double> ONE(1.0, 0.0);
			point_l.first = point.frequency;
			::std::complex<double> s11 = point.sparams.s11;
			// Get Z0 from the dataset.
			::std::complex<double> z0 = Z0;
			::std::complex<double> z = z0 * (ONE + s11) / (ONE - s11); // Convert S11 to impedance.
			point_l.second = z.real(); // Resistance
			point_r.first = point.frequency;
			point_r.second = z.imag(); // Reactance
		}

		void convert_sp_to_coords(
			const sp_data_entry& dataset,
			graph_data_map_t& coords,
			graph_data_ranges_t& ranges) const override
		{
			if (dataset.valid_ports < 1) {
				return; // Not enough valid ports for S11
			}
			// We have two datasets to populate for this display mode.
			// The left Y axis is for S11 resistance, and the right Y axis is for S11 reactance.
			dm_data_set_t* resistance_coords = new dm_data_set_t;
			resistance_coords->style = dataset.line_style_l;
			coords[1] = resistance_coords;
			dm_data_set_t* reactance_coords = new dm_data_set_t;
			reactance_coords->style = dataset.line_style_r;
			coords[2] = reactance_coords;
			for (const sp_point& point : dataset.data) {
				zc_graph_::data_point_t point_resistance;
				zc_graph_::data_point_t point_reactance;
				convert_sp_point(point, dataset.z0, point_resistance, point_reactance);
				resistance_coords->data->push_back(point_resistance);
				reactance_coords->data->push_back(point_reactance);
				// Update axis ranges based on the data points.
				ranges[0] |= point_resistance.first;
				ranges[0] |= point_reactance.first;
				ranges[1] |= point_resistance.second;
				ranges[2] |= point_reactance.second;
			}
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