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

#include "zc_graph_axis.h"
#include "zc_graph_base.h"
#include "zc_graph_xy0.h"

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

			zc_graph_axis::axis_params_t x_axis_params;
			x_axis_params.orientation = zc_graph_axis::orientation_t::X0_AXIS;
			x_axis_params.outer_range = { -FLT_MAX, FLT_MAX };
			x_axis_params.inner_range = { -50.0F, 50.0F };
			x_axis_params.default_range = { -50.0F, 50.0F };
			x_axis_params.modifier = zc_graph_axis::modifier_t::SI_PREFIX;
			x_axis_params.unit = "\xCE\xA9";
			x_axis_params.label = "S11 Resistance";
			x_axis_params.tick_spacing_pixels = 30;
			params_.axis_params[zc_graph_axis::orientation_t::X0_AXIS] = x_axis_params;

			zc_graph_axis::axis_params_t y_axis_params;
			y_axis_params.orientation = zc_graph_axis::orientation_t::Y0_AXIS;
			y_axis_params.outer_range = { -FLT_MAX, FLT_MAX };
			y_axis_params.inner_range = { -50.0F, 50.0F };
			y_axis_params.default_range = { -50.0F, 50.0F };
			y_axis_params.modifier = zc_graph_axis::modifier_t::SI_PREFIX;
			y_axis_params.unit = "\xCE\xA9";
			y_axis_params.label = "S11 Reactance";
			y_axis_params.tick_spacing_pixels = 30;
			params_.axis_params[zc_graph_axis::orientation_t::Y0_AXIS] = y_axis_params;

		}

		void convert_sp_point(
			const sp_point& point,
			float Z0,
			zc_graph_base::coord& pixel) const
		{
			::std::complex<double> ONE(1.0, 0.0);
			::std::complex<double> s11 = point.sparams.s11;
			// Get Z0 from the dataset.
			::std::complex<double> z0 = Z0;
			::std::complex<double> z = z0 * (ONE + s11) / (ONE - s11); // Convert S11 to impedance.
			pixel.a = z.real(); // Resistance
			pixel.b = z.imag(); // Reactance
		}

		void convert_sp_to_coords(
			const sp_data_entry& dataset,
			graph_data_map_t& coords,
			graph_data_ranges_t& ranges) const override
		{
			if (dataset.valid_ports < 1) {
				return; // Not enough valid ports for S11
			}
			// We have one data set to populate for this display mode, which will be used for both resistance and reactance since they share the same X values (frequency).
			zc_graph_base::data_set_t* data_coords = new zc_graph_base::data_set_t;
			data_coords->type_a = zc_graph_base::data_type_t::X_VALUE;
			data_coords->type_b = zc_graph_base::data_type_t::Y_VALUE; 
			data_coords->style = dataset.line_style_l; 
			coords[zc_graph_axis::orientation_t::Y0_AXIS] = data_coords;
			for (const sp_point& point : dataset.data) {
				zc_graph_base::coord point_coords;
				convert_sp_point(point, dataset.z0, point_coords);
				data_coords->data->push_back(point_coords);
				// Update axis ranges based on the data points.
				update_range_point(ranges[zc_graph_base::data_type_t::X_VALUE], point_coords.a);
				update_range_point(ranges[zc_graph_base::data_type_t::Y_VALUE], point_coords.b);
			}
		}

		zc_graph_base* create_graph(int X, int Y, int W, int H) override {
			return new zc_graph_xy0(X, Y, W, H);
		}

		zc_graph_axis::range get_range(
			zc_graph_base::data_type_t data_type
		) override {
			switch (data_type) {
			case zc_graph_base::data_type_t::X_VALUE:
				return graph_->get_data_range(zc_graph_axis::orientation_t::X0_AXIS);
			case zc_graph_base::data_type_t::Y_VALUE:
				return graph_->get_data_range(zc_graph_axis::orientation_t::Y0_AXIS);
			default:
				return { 0.0F, 1.0F }; // Default range if data type is not recognized
			}
		}

		graph_data_ranges_t get_all_data_ranges() override {
			graph_data_ranges_t ranges;
			ranges[zc_graph_base::data_type_t::X_VALUE] = get_range(zc_graph_base::data_type_t::X_VALUE);
			ranges[zc_graph_base::data_type_t::Y_VALUE] = get_range(zc_graph_base::data_type_t::Y_VALUE);
			return ranges;
		}
	};

};