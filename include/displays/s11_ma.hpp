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

#include "zc_graph_axis.h"
#include "zc_graph_base.h"
#include "zc_graph_x2y.h"
#include "zc_utils.h"

#include <cfloat>
#include <complex>

namespace display_modes {

	class s11_ma : public display {

	public:

		s11_ma(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
		}

		void configure_dm_params() override {
			params_.serial_name = "S11 M+A";
			params_.title = "S11 Magnitude and angle vs frequency";

			axis_params_t x_axis_params;
			x_axis_params.outer_range = { 0.0F, DBL_MAX };
			x_axis_params.inner_range = { 1e6F, 30e6F };
			x_axis_params.default_range = { 1e6F, 30e6F };
			x_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			x_axis_params.unit = "Hz";
			x_axis_params.label = "Frequency";
			params_.axis_params[0] = x_axis_params;

			axis_params_t yl_axis_params;
			yl_axis_params.outer_range = { 0.0F, 1.0F };
			yl_axis_params.inner_range = { 0.0F, 1.0F };
			yl_axis_params.default_range = { 0.0F, 1.0F };
			yl_axis_params.unit_modifier = zc_graph_::modifier_t::NO_MODIFIER;
			yl_axis_params.unit = "";
			yl_axis_params.label = "S11 Magnitude";
			params_.axis_params[1] = yl_axis_params;

			axis_params_t yr_axis_params;
			yr_axis_params.outer_range = { -180.0F, 180.0F };
			yr_axis_params.inner_range = { -180.0F, 180.0F };
			yr_axis_params.default_range = { -180.0F, 180.0F };
			yr_axis_params.unit_modifier = zc_graph_::modifier_t::NO_MODIFIER;
			yr_axis_params.unit = "degree";
			yr_axis_params.label = "S11 Angle";
			params_.axis_params[2] = yr_axis_params;
		}

		void add_markers() override {
			add_frequency_markers();
		}

		void convert_sp_point(
			const sp_point& point,
			zc_graph_::data_point_t& point_l,
			zc_graph_::data_point_t& point_r) const 
		{
			point_l.first = point.frequency;
			::std::complex<double> s11 = point.sparams.s11;
			point_l.second = ::std::abs(s11); // S11 magnitude
			point_r.first = point.frequency;
			point_r.second = ::std::arg(s11) * zc::RADIAN_DEGREE; // S11 phase in degrees
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
			// The left Y axis is for S11 magnitude, and the right Y axis is for S11 angle.
			dm_data_set_t* mag_coords = new dm_data_set_t;
			mag_coords->style = dataset.line_style_l;
			coords[1] = mag_coords;
			dm_data_set_t* angle_coords = new dm_data_set_t;
			angle_coords->style = dataset.line_style_r;
			coords[2] = angle_coords;

			for (const sp_point& point : dataset.data) {
				zc_graph_::data_point_t point_mag;
				zc_graph_::data_point_t point_angle;
				convert_sp_point(point, point_mag, point_angle);
				mag_coords->data->push_back(point_mag);
				angle_coords->data->push_back(point_angle);
				// Update axis ranges based on the data points.
				ranges[0] |= point_mag.first;
				ranges[0] |= point_angle.first;
				ranges[1] |= point_mag.second;
				ranges[2] |= point_angle.second;
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