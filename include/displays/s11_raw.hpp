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

#include <cfloat>
#include <complex>

namespace display_modes {

	class s11_raw : public display {

	public:

		s11_raw(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
		}

		void configure_dm_params() override {
			params_.serial_name = "S11 Raw";
			params_.title = "S11 Real and Imaginary vs frequency";

			zc_graph_axis::axis_params_t x_axis_params;
			x_axis_params.orientation = zc_graph_axis::orientation_t::X_AXIS;
			x_axis_params.outer_range = { 0.0F, FLT_MAX };
			x_axis_params.inner_range = { 1e6F, 30e6F };
			x_axis_params.default_range = { 1e6F, 30e6F };
			x_axis_params.modifier = zc_graph_axis::modifier_t::SI_PREFIX;
			x_axis_params.unit = "Hz";
			x_axis_params.label = "Frequency";
			x_axis_params.tick_spacing_pixels = 30;
			params_.axis_params[zc_graph_axis::orientation_t::X_AXIS] = x_axis_params;

			zc_graph_axis::axis_params_t yl_axis_params;
			yl_axis_params.orientation = zc_graph_axis::orientation_t::YL_AXIS;
			yl_axis_params.outer_range = { -1.0F, 1.0F };
			yl_axis_params.inner_range = { -1.0F, 1.0F };
			yl_axis_params.default_range = { -1.0F, 1.0F };
			yl_axis_params.modifier = zc_graph_axis::modifier_t::NO_MODIFIER;
			yl_axis_params.unit = "Sr";
			yl_axis_params.label = "S11 Real";
			yl_axis_params.tick_spacing_pixels = 30;
			params_.axis_params[zc_graph_axis::orientation_t::YL_AXIS] = yl_axis_params;

			zc_graph_axis::axis_params_t yr_axis_params;
			yr_axis_params.orientation = zc_graph_axis::orientation_t::YR_AXIS;
			yr_axis_params.outer_range = { -1.0F, 1.0F };
			yr_axis_params.inner_range = { -1.0F, 1.0F };
			yr_axis_params.default_range = { -1.0F, 1.0F };
			yr_axis_params.modifier = zc_graph_axis::modifier_t::NO_MODIFIER;
			yr_axis_params.unit = "Si";
			yr_axis_params.label = "S11 Imaginary";
			yr_axis_params.tick_spacing_pixels = 30;
			params_.axis_params[zc_graph_axis::orientation_t::YR_AXIS] = yr_axis_params;

		}

		void convert_sp_point(
			const sp_point& point,
			zc_graph_base::coord& point_l,
			zc_graph_base::coord& point_r) const
		{
			point_l.a = point.frequency;
			point_l.b = point.sparams.s11.real(); // S11 real part
			point_r.a = point.frequency;
			point_r.b = point.sparams.s11.imag(); // S11 imaginary part
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
			// The left Y axis is for S11 real, and the right Y axis is for S11 imaginary.
			zc_graph_base::data_set_t* real_coords = new zc_graph_base::data_set_t;
			real_coords->type_a = zc_graph_base::data_type_t::X_VALUE;
			real_coords->type_b = zc_graph_base::data_type_t::Y_VALUE;
			real_coords->style = dataset.line_style_l;
			coords[zc_graph_axis::orientation_t::YL_AXIS] = real_coords;
			zc_graph_base::data_set_t* imag_coords = new zc_graph_base::data_set_t;
			imag_coords->type_a = zc_graph_base::data_type_t::X_VALUE;
			imag_coords->type_b = zc_graph_base::data_type_t::Y_VALUE;
			imag_coords->style = dataset.line_style_r;
			coords[zc_graph_axis::orientation_t::YR_AXIS] = imag_coords;
			for (const sp_point& point : dataset.data) {
				zc_graph_base::coord point_real;
				zc_graph_base::coord point_imag;
				convert_sp_point(point, point_real, point_imag);
				real_coords->data->push_back(point_real);
				imag_coords->data->push_back(point_imag);
				// Update axis ranges based on the data points.
				update_range_point(ranges[zc_graph_base::data_type_t::X_VALUE], point_real.a);
				update_range_point(ranges[zc_graph_base::data_type_t::X_VALUE], point_imag.a);
				update_range_point(ranges[zc_graph_base::data_type_t::Y_VALUE], point_real.b);
				update_range_point(ranges[zc_graph_base::data_type_t::Y2_VALUE], point_imag.b);
			}
		}

		zc_graph_base* create_graph(int X, int Y, int W, int H) override {
			return new zc_graph_x2y(X, Y, W, H);
		}

		zc_graph_axis::range get_range(
			zc_graph_base::data_type_t data_type
		) override {
			switch (data_type) {
			case zc_graph_base::data_type_t::X_VALUE:
				return graph_->get_data_range(zc_graph_axis::orientation_t::X_AXIS);
			case zc_graph_base::data_type_t::Y_VALUE:
				return graph_->get_data_range(zc_graph_axis::orientation_t::YL_AXIS);
			case zc_graph_base::data_type_t::Y2_VALUE:
				return graph_->get_data_range(zc_graph_axis::orientation_t::YR_AXIS);
			default:
				return { 0.0F, 1.0F }; // Default range if data type is not recognized
			}
		}


		graph_data_ranges_t get_all_data_ranges() override {
			graph_data_ranges_t ranges;
			ranges[zc_graph_base::data_type_t::X_VALUE] = get_range(zc_graph_base::data_type_t::X_VALUE);
			ranges[zc_graph_base::data_type_t::Y_VALUE] = get_range(zc_graph_base::data_type_t::Y_VALUE);
			ranges[zc_graph_base::data_type_t::Y2_VALUE] = get_range(zc_graph_base::data_type_t::Y2_VALUE);
			return ranges;
		}
	};
};