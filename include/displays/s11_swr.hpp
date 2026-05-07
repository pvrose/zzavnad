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

			axis_params_t x_axis_params;
			x_axis_params.outer_range = { 0.0F, DBL_MAX };
			x_axis_params.inner_range = { 1e6F, 30e6F };
			x_axis_params.default_range = { 1e6F, 30e6F };
			x_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			x_axis_params.unit = "Hz";
			x_axis_params.label = "Frequency";
			params_.axis_params[0] = x_axis_params;

			axis_params_t y_axis_params;
			y_axis_params.outer_range = { 1.0F, 15.0F };
			y_axis_params.inner_range = { 1.0F, 15.0F };
			y_axis_params.default_range = { 1.0F, 15.0F };
			y_axis_params.unit_modifier = zc_graph_::modifier_t::NO_MODIFIER;
			y_axis_params.unit = "";
			y_axis_params.label = "SWR";
			params_.axis_params[1] = y_axis_params;
		}

		void add_markers() override {
			add_frequency_markers();
			// Add marker for SWR=3 (a common threshold for acceptable SWR).
			graph_->add_marker(1, zc_graph_::FOREGROUND, zc_line_style(FL_RED, 1, FL_DASH), 3.0);
			graph_->add_label(1, zc_graph_::FOREGROUND, "SWR=3:1", zc_text_style(FL_RED, 0, graph_->textsize()), { -DBL_MAX, 3.0 }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_ABOVE);
		}

		void convert_sp_point(
			const sp_point& point,
			zc_graph_::data_point_t& point_l) const
		{
			point_l.first = point.frequency;
			double s11_mag = ::std::abs(point.sparams.s11);
			point_l.second = (1 + s11_mag) / (1 - s11_mag); // SWR calculation from S11 magnitude.
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
			dm_data_set_t* swr_coords = new dm_data_set_t;
			swr_coords->style = dataset.line_style_l;
			coords[1] = swr_coords;

			for (const sp_point& point : dataset.data) {
				zc_graph_::data_point_t point_swr;
				convert_sp_point(point, point_swr);
				swr_coords->data->push_back(point_swr);
				// Update axis ranges based on the data points.
				ranges[0] |= point_swr.first; // Update X axis range with frequency
				ranges[1] |= point_swr.second; // Update Y axis range with SWR
			}
		}

		zc_graph_* create_graph(int X, int Y, int W, int H) override {
			return new zc_graph_cartesian(X, Y, W, H);
		}

		graph_data_ranges_t get_all_data_ranges() override {
			graph_data_ranges_t ranges;
			ranges[0] = get_range(0);
			ranges[1] = get_range(1);
			return ranges;
		}
	};

};