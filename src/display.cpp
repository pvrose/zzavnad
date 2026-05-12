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

#include "display.hpp"
#include "display_control.hpp"
#include "display_legend.hpp"
// Include the S-parameter data structures.
#include "sp_data.hpp"

// Include ZZACOMMON drawing constants
#include "zc_drawing.h"
#include "zc_graph_.h"
#include "zc_settings.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Enumerations.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Double_Window.H>

#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>

// Include C++ standard library headers.
#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

// Structure combining paremeters required for specific display modes.
struct display_mode_params_t {
    std::string label; //!< Label for the display mode.
    bool dual_axes; //!< Whether this display mode requires dual Y-axes.
    void (*convert_sp_point)(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r); //!< Function pointer to the routine for converting sp_points to graph coordinates for this display mode.
};

// Constant complex 1.0+j0.0.
const std::complex<double> ONE = 1.0;

// Constructor for the display class.
display::display(int W, int H, const char* L)
    : Fl_Double_Window(W, H, L) {
    box(FL_BORDER_BOX);
    align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    hide();
}

// Pre-create method.
void display::create() {
    // This method can be used by derived classes to run any code 
    // that needs to be run before the widgets are created, 
    // such as setting display mode parameters that are 
    // needed for widget configuration.
	configure_dm_params();
    create_widgets();
    configure_graph();
    load_default_settings();
    configure_widgets();
}

// Destructor for the display class.
display::~display() {
    save_current_settings();
}

// Create the widgets for the display window.
void display::create_widgets() {
    int cx = x() + GAP;
    int cy = y() + GAP;
	const int HLEGEND = HBUTTON * 3 + HTEXT; // Height of the legend widgets.

    // Add the graph widget for plotting the data.
    graph_ = create_graph(cx, cy, w() - 2 * GAP, h() - GAP - HLEGEND - GAP);
    graph_->box(FL_BORDER_BOX);
	graph_->color(FL_WHITE);
    graph_->tooltip("Graph for plotting S-parameter data");

    cy += graph_->h();

    // Put a group around the legends so they will be resized together.
	Fl_Group* legend_group = new Fl_Group(cx, cy, graph_->w(), HLEGEND);

	int number_legends = params_.axis_params.size() - 1;
	int w_legend = number_legends == 0 ? 1 :legend_group->w() / number_legends;
    for (int axis = 1; axis < params_.axis_params.size(); axis++) {
        legends_[axis] = new display_legend(cx, cy, w_legend, HLEGEND);
        legends_[axis]->box(FL_BORDER_BOX);
        legends_[axis]->copy_tooltip(("Legend for the " + std::to_string(axis) + " axis").c_str());
        legends_[axis]->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT);
		cx += w_legend;
    };
    legend_group->end();
    legend_group->resizable(legend_group);

	resizable(graph_);

    end();
}

// Configure the graph data based on the current display mode and the S-parameter data in sp_data.
void display::configure_graph() {
    // Get the number of sets of coordinates to plot for the current display mode.
    // Clear the existing graph data.
    // Map the enabled datasets to graph data sets and add them to the graph.
    graph_->start_config();
	// Set the graph parameters for the axes based on the display mode parameters.
	for (const auto& axis : params_.axis_params) {
        graph_->set_axis_params(
            axis.first, 
            axis.second.unit_modifier,
            axis.second.unit,
            axis.second.label,
            30);
        graph_->set_axis_ranges(
            axis.first,
			axis.second.inner_range,
			axis.second.outer_range,
			axis.second.default_range);
    }
    add_markers();
    update_graph_data();
	redraw();
}

// Update the graph with the current S-parameter data from sp_data, converting the S-parameter points to graph coordinates based on the current display mode.
void display::update_graph_data() {
	// Clear the existing graph data.
	graph_->clear_data_sets();
    std::set<int> dataset_indices;
    // Get the number of S-parameter datasets to plot.
    int num_datasets = 0;
    // Create a range for each data type
	graph_data_ranges_t graph_data_ranges = get_all_data_ranges();
    for (int i = 0; i < sp_data_->get_dataset_count(); i++) {
        auto dataset = sp_data_->get_dataset(i);
        if (dataset->enabled) {
            num_datasets++;
            dataset_indices.insert(i);
        }
    }

    for (int dataset_index : dataset_indices) {
        graph_data_map_t* graph_data_map = new graph_data_map_t();
        auto dataset = sp_data_->get_dataset(dataset_index);
        // For each axis we need to plot for this display mode, add
        // a graph data set for the points to plot on that axis.
        convert_sp_to_coords(*dataset, *graph_data_map, graph_data_ranges);
        // Add the graph data set for each axis to the graph.
        for (const auto& axis : params_.axis_params) {
			auto it = graph_data_map->find(axis.first);
            if (it != graph_data_map->end()) {
                graph_->add_data_set(
                    axis.first,
                    it->second->data,
                    it->second->style
                );
            }
        };
    }
    graph_->end_config();

    // For each data type, update the legend to show the datasets that are plotted for that data type.
    for (int axis = 1; axis < params_.axis_params.size(); axis++) {
        update_legend(axis);
    }

    redraw();
}

// Update the legend for each data type based on the current display mode and data.
void display::update_legend(int  axis) {
    // Get the label for this data type based on the current display mode.
    std::string label;
    // For each dataset map onto next legend entry for this data type.
    int num_datasets = sp_data_->get_dataset_count();
    std::vector<legend_entry_t> legend_entries;
    for (int i = 0; i < num_datasets; i++) {
        auto dataset = sp_data_->get_dataset(i);
        if (dataset->enabled) {
            legend_entry_t entry;
            if (axis == 1) {
                entry.style = dataset->line_style_l;
            }
            else {
                // == Y2_VALUE
                entry.style = dataset->line_style_r;
            }
            switch (dataset->source) {
                case SPDS_ACTIVE:
                    entry.source = "nanoVNA";
                    break;
                case SPDS_FILE: {
                    std::string filename = zc::terminal(dataset->filename);
                    // Remove the file extension from the filename for
                    // display in the legend.
                    size_t last_dot = filename.find_last_of(".");
                    if (last_dot != std::string::npos) {
                        filename = filename.substr(0, last_dot);
                    }
                    entry.source = filename;
                    break;
                }
                case SPDS_KEPT:
                    entry.source = dataset->timestamp;
                    break;
            }
            // Set the legend entry for this dataset.
            legend_entries.push_back(entry);
        }
    }
	legends_.at(axis)->set_entries(legend_entries);
    redraw();
}


// Load the previous settings for the display.
void display::load_default_settings() {
    zc_settings settings;
    // None at the moment.
}

// Save the current settings for the display.
void display::save_current_settings() {
    zc_settings settings;
    // None at the moment.
}

// Configure the widgets based on the current settings.
void display::configure_widgets() {
	for (int axis = 1; axis < params_.axis_params.size(); axis++) {
 		legends_.at(axis)->copy_label(params_.axis_params[axis].label.c_str());
		legends_.at(axis)->show();
    }
}

void display::add_frequency_markers() {
    // Add frequency markers to the graph for the current display mode and data.
	zc_settings settings;
	zc_settings markers_settings(&settings, "Markers");
	zc_settings frequency_markers_settings(&markers_settings, "Frequency");
    bool show_frequency_markers;
	frequency_markers_settings.get("Enabled", show_frequency_markers, true);
    if (show_frequency_markers) {
        // Add band bars for the amateur radio bands.
        Fl_Color text_colour;
		frequency_markers_settings.get("Colour", (uint32_t&)text_colour, FL_BLUE);
        Fl_Color band_colour = fl_color_average(FL_WHITE, text_colour, 0.875F);
        Fl_Fontsize fz = graph_->textsize();
        Fl_Font f = graph_->textfont();

        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 135.7e3, 137.8e3);
        graph_->add_label(0, zc_graph_::BACKGROUND, "2190m", zc_text_style(text_colour, f, fz), { 135.7e3, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 472e3F, 479e3F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "630m", zc_text_style(text_colour, f, fz), { 472e3F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 1.81e6F, 2.0e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "160m", zc_text_style(text_colour, f, fz), { 1.81e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 3.5e6F, 3.8e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "80m", zc_text_style(text_colour, f, fz), { 3.5e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 5.2585e6F, 5.4065e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "60m", zc_text_style(text_colour, f, fz), { 5.2585e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 7.0e6F, 7.2e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "40m", zc_text_style(text_colour, f, fz), { 7.0e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 10.1e6F, 10.15e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "30m", zc_text_style(text_colour, f, fz), { 10.1e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 14.0e6F, 14.35e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "20m", zc_text_style(text_colour, f, fz), { 14.0e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 18.068e6F, 18.168e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "17m", zc_text_style(text_colour, f, fz), { 18.068e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 21.0e6F, 21.45e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "15m", zc_text_style(text_colour, f, fz), { 21.0e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 24.89e6F, 24.99e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "12m", zc_text_style(text_colour, f, fz), { 24.89e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 28e6F, 29.7e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "10m", zc_text_style(text_colour, f, fz), { 28e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 50e6F, 52e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "6m", zc_text_style(text_colour, f, fz), { 50e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 70e6F, 70e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "4m", zc_text_style(text_colour, f, fz), { 70e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 144e6F, 146e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "2m", zc_text_style(text_colour, f, fz), { 144e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 430e6F, 440e6F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "70cm", zc_text_style(text_colour, f, fz), { 430e6F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 1.24e9F, 1.325e9F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "23cm", zc_text_style(text_colour, f, fz), { 1.24e9F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
        graph_->add_marker(0, zc_graph_::BACKGROUND, zc_line_style(band_colour, 1, FL_DASH), 2.3e9F, 2.45e9F);
        graph_->add_label(0, zc_graph_::BACKGROUND, "13cm", zc_text_style(text_colour, f, fz), { 2.3e9F, DBL_MAX }, zc_graph_::ALIGN_RIGHT | zc_graph_::ALIGN_BELOW);
    }
}