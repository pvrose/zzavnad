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

//! \file s11_rj.hpp
//! \brief Display mode for S11 Resistance vs Reactance.

#include "display.hpp"
#include "sp_data.hpp"	

#include "zc_graph_.h"

#include <FL/Fl_Widget.H>

#include <cfloat>
#include <complex>
#include <cstdio>
#include <string>
#include <map>

namespace display_modes {

	//! \brief Display mode for S11 Resistance vs Reactance.
	//! The resistance (R) is plotted on the X axis, and the reactance (jX) is plotted on the Y axis.
	class s11_rj : public display {

	public:

		//! Constructor for the s11_rj display mode.
		s11_rj(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
		}

		//! Configure the display mode parameters for S11 Resistance vs Reactance.
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

		//! Convert an S11 point to resistance and reactance coordinates for plotting.
		//! The S11 parameter is converted to impedance using the formula Z = Z0 * (1 + S11) / (1 - S11), where Z0 is the characteristic impedance (typically 50 ohms).
		//! The real part of the impedance is the resistance (R), and the imaginary part is the reactance (jX).
		//! \param point The S-parameter data point to convert.
		//! \param Z0 The characteristic impedance.
		//! \param pixel The output data point in resistance and reactance coordinates.
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

		//! Convert the S-parameter dataset to coordinates for plotting in the S11 Resistance vs Reactance display mode.
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

		//! \brief Callback for graph clicks to get the frequency at the clicked point.
		static void cb_graph(Fl_Widget* widget, void* data) {
			display* disp = zc::ancestor_view<display>(widget);
			zc_graph_::data_point_t clicked_point = ((zc_graph_*)widget)->value();
			//// Do something with the clicked frequency, e.g. update a label or emit a signal.
			//// For this example, we'll just print it to the console.
			//printf("Clicked data: %f @ %f Hz\n", clicked_point.second, clicked_point.first);

			// Save the clicked frequency to the display's value and trigger any callbacks that depend on it.
			disp->value(clicked_point.first); // Store the clicked frequency 
			disp->do_callback();
		}

		//! \brief Create a new graph widget for the S11 Resistance vs Reactance display mode.
		//! \param X The X coordinate of the graph.
		//! \param Y The Y coordinate of the graph.
		//! \param W The width of the graph.
		//! \param H The height of the graph.
		//! \return A pointer to the created graph widget.
		zc_graph_* create_graph(int X, int Y, int W, int H) override {
			zc_graph_* graph = new zc_graph_cart_overlay(X, Y, W, H);
			graph->callback(cb_graph);
			return graph;
		}

		//! \brief Get the data ranges for both axes in the S11 Resistance vs Reactance display mode.
		//! \return A structure containing the ranges for the X and Y axes.
		graph_data_ranges_t get_all_data_ranges() override {
			graph_data_ranges_t ranges;
			ranges[0] = get_range(0);
			ranges[1] = get_range(1);
			return ranges;
		}

		//! \brief Format the S11 value for display in a tooltip or label.
		//! \param point The S-parameter data point to format.
		//! \return A string representation of the S11 value in terms of resistance and reactance.
		std::string format_value(sp_point point) override {
			char buffer[100];
			// Assume Z0 = 50 ohms for formatting the S11 value.
			double Z0 = 50.0;
			zc_graph_::data_point_t point_coords;
			convert_sp_point(point, Z0, point_coords);
			// Ensure negative reactance is displayed as "-jX" instead of "+ j- X".
			if (point_coords.second < 0) {
				snprintf(buffer, sizeof(buffer), "%.2f - j%.2f \xCE\xA9",  point_coords.first, -point_coords.second);
			}
			else {
				snprintf(buffer, sizeof(buffer), "%.2f + j%.2f \xCE\xA9",  point_coords.first, point_coords.second);
			}
			return std::string(buffer);
		}
	};

};