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
	class s11_z_polar : public display {
	public:

		//! Constructor for the s11_z_polar display mode.
		s11_z_polar(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
		}

		// Configure the display mode parameters for the s11_z_polar display mode.
		void configure_dm_params() override {
			params_.serial_name = "S11 Z Polar";
			params_.title = "S11 Impedance Polar Plot";
			axis_params_t r_axis_params;
			r_axis_params.outer_range = { 0, 1000.0F };
			r_axis_params.inner_range = { 0, 50.0F };
			r_axis_params.default_range = { 0, 500.0F };
			r_axis_params.unit_modifier = zc_graph_::modifier_t::SI_PREFIX;
			r_axis_params.unit = "\xCE\xA9";
			r_axis_params.label = "Impedance";
			params_.axis_params[0] = r_axis_params;

			axis_params_t theta_axis_params;
			theta_axis_params.outer_range = { -180.0F, 180.0F };
			theta_axis_params.inner_range = { -180.0F, 180.0F };
			theta_axis_params.default_range = { -180.0F, 180.0F };
			theta_axis_params.unit_modifier = zc_graph_::modifier_t::NO_MODIFIER;
			theta_axis_params.unit = "\xC2\xB0";
			theta_axis_params.label = "Phase";
			params_.axis_params[1] = theta_axis_params;
		}

		// Add markers for the s11_z_polar display mode.
		// For the polar plot, we might want to add markers for specific
		// impedance values (e.g. 50 Ohms) or for the unit circle (|S11|=1).
		void add_markers() override {
			graph_->add_marker(0, zc_graph_::FOREGROUND, zc_line_style(FL_RED, 1, FL_DASH), 50.0F);
			graph_->add_label(0, zc_graph_::FOREGROUND, "|Z|=50\xCE\xA9", zc_text_style(FL_RED, 0, FL_NORMAL_SIZE - 2), { 50.0F, 0 });
		}

		// Convert a sp_point to coordinates for the polar plot.
		void convert_sp_point(
			const sp_point& point,
			double Z0,
			zc_graph_::data_point_t& point_r_theta) const
		{
			::std::complex<double> ONE(1.0, 0.0);
			::std::complex<double> s11 = point.sparams.s11;
			// Get Z0 from the dataset.
			::std::complex<double> z0 = Z0;
			::std::complex<double> z = z0 * (ONE + s11) / (ONE - s11); // Convert S11 to impedance.
			point_r_theta.first = std::abs(z); // Magnitude of impedance
			point_r_theta.second = std::arg(z) * 180.0 / zc::PI; // Phase of impedance
		}

		// Convert sp_data to coordinates for the polar plot.
		void convert_sp_to_coords(
			const sp_data_entry& dataset,
			graph_data_map_t& coords,
			graph_data_ranges_t& ranges	) override
		{
			if (dataset.valid_ports < 1) {
				return; // Not enough valid ports for S11
			}
			// We have one dataset to populate for this display mode.
			// The R axis is for the magnitude of impedance, and the Theta axis is for the phase of impedance.
			dm_data_set_t* r_theta_coords = new dm_data_set_t;
			r_theta_coords->style = dataset.line_style_l; // Use left line style for this plot.
			coords[1] = r_theta_coords;
			for (const sp_point& point : dataset.data) {
				zc_graph_::data_point_t point_r_theta;
				convert_sp_point(point, dataset.z0, point_r_theta);
				r_theta_coords->data->push_back(point_r_theta);
				// Update axis ranges based on the data points.
				ranges[0] |= point_r_theta.first;
			}
		}

		zc_graph_* create_graph(int X, int Y, int W, int H) override {
			return new zc_graph_polar(X, Y, W, H);
		}

		graph_data_ranges_t get_all_data_ranges() override {
			graph_data_ranges_t ranges;
			ranges[0] = get_range(0);
			return ranges;
		}
	};
};
