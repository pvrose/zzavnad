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
#include "zc_graph_xy.h"

#include "zc_line_style.h"

#include <cfloat>
#include <complex>

namespace display_modes {

	class s11_swr : public display {

	public:

		s11_swr(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
		}

		void configure_dm_params() override {
			params_.serial_name = "SWR";
			params_.title = "SWR vs frequency";

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

			zc_graph_axis::axis_params_t y_axis_params;
			y_axis_params.orientation = zc_graph_axis::orientation_t::YL_AXIS;
			y_axis_params.outer_range = { 1.0F, 15.0F };
			y_axis_params.inner_range = { 1.0F, 15.0F };
			y_axis_params.default_range = { 1.0F, 15.0F };
			y_axis_params.modifier = zc_graph_axis::modifier_t::NO_MODIFIER;
			y_axis_params.unit = "";
			y_axis_params.label = "SWR";
			y_axis_params.tick_spacing_pixels = 30;
			params_.axis_params[zc_graph_axis::orientation_t::YL_AXIS] = y_axis_params;
		}

		void add_markers() override {
			// Add marker for SWR=3 (a common threshold for acceptable SWR).
			graph_->add_marker(zc_graph_base::Y_VALUE, zc_line_style(FL_RED, 1, FL_DASH), 3.0F);
			// Add band bar
			graph_->add_marker(zc_graph_base::X_VALUE, zc_line_style(FL_GRAY, 1, FL_DASH), 28e6F, 29.7e6F);
		}

		void convert_sp_point(
			const sp_point& point,
			zc_graph_base::coord& point_l) const
		{
			point_l.a = point.frequency;
			double s11_mag = ::std::abs(point.sparams.s11);
			point_l.b = (1 + s11_mag) / (1 - s11_mag); // SWR calculation from S11 magnitude.
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
			// The Y axis is for SWR.
			zc_graph_base::data_set_t* swr_coords = new zc_graph_base::data_set_t;
			swr_coords->type_a = zc_graph_base::data_type_t::X_VALUE;
			swr_coords->type_b = zc_graph_base::data_type_t::Y_VALUE;
			swr_coords->style = dataset.line_style_l;
			coords[zc_graph_axis::orientation_t::YL_AXIS] = swr_coords;

			for (const sp_point& point : dataset.data) {
				zc_graph_base::coord point_swr;
				convert_sp_point(point, point_swr);
				swr_coords->data->push_back(point_swr);
				// Update axis ranges based on the data points.
				update_range_point(ranges[zc_graph_base::data_type_t::X_VALUE], point_swr.a);
				update_range_point(ranges[zc_graph_base::data_type_t::Y_VALUE], point_swr.b);
			}
		}

		zc_graph_base* create_graph(int X, int Y, int W, int H) override {
			return new zc_graph_xy(X, Y, W, H);
		}

		zc_graph_axis::range get_range(
			zc_graph_base::data_type_t data_type
		) override {
			switch (data_type) {
			case zc_graph_base::data_type_t::X_VALUE:
				return graph_->get_data_range(zc_graph_axis::orientation_t::X_AXIS);
			case zc_graph_base::data_type_t::Y_VALUE:
				return graph_->get_data_range(zc_graph_axis::orientation_t::YL_AXIS);
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