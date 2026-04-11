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
#include "zc_graph_axis.h"
#include "zc_graph_base.h"
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
}

// Pre-create method.
void display::create() {
    // This method can be used by derived classes to run any code 
    // that needs to be run before the widgets are created, 
    // such as setting display mode parameters that are 
    // needed for widget configuration.
	configure_dm_params();
	update_supported_data_types();
    create_widgets();
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
    graph_->create();

    cy += graph_->h();

    // Put a group around the legends so they will be resized together.
	Fl_Group* legend_group = new Fl_Group(cx, cy, graph_->w(), HLEGEND);

	int number_legends = params_.data_types.size();
	int w_legend = number_legends == 0 ? 1 :legend_group->w() / number_legends;
    for (auto& data_type : params_.data_types) {
        legends_[data_type] = new display_legend(cx, cy, w_legend, HLEGEND);
        legends_[data_type]->box(FL_BORDER_BOX);
        legends_[data_type]->copy_tooltip(("Legend for the " + std::to_string(data_type) + " axis").c_str());
        legends_[data_type]->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT);
		cx += w_legend;
    };
    legend_group->end();
    legend_group->resizable(legend_group);

	resizable(graph_);

    end();
    show();
}

// Configure the graph data based on the current display mode and the S-parameter data in sp_data.
void display::configure_graph() {
    // Get the number of sets of coordinates to plot for the current display mode.
    // Clear the existing graph data.
    // Map the enabled datasets to graph data sets and add them to the graph.
    dataset_to_graph_map_.clear();
    graph_->clear_data_sets();
	// Set the graph parameters for the axes based on the display mode parameters.
	for (const auto& axis : params_.axis_params) {
        graph_->set_axis_params(axis.second);
    }
    update_graph();
	// For each data type, update the legend to show the datasets that are plotted for that data type.
    for (const auto& data_type : params_.data_types) {
        update_legend(data_type);
    }
	redraw();
}

// Update the graph with the current S-parameter data from sp_data, converting the S-parameter points to graph coordinates based on the current display mode.
void display::update_graph() {
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
            if (graph_data_map->find(axis.first) != graph_data_map->end()) {
                graph_->add_data_set(graph_data_map->at(axis.first));
            }
        };
    }
	// Update the axis ranges based on the data.
    for (const auto& data_range : graph_data_ranges) {
        graph_->set_data_range(data_range.first, data_range.second);
	}

    redraw();
}

// Update the legend for each data type based on the current display mode and data.
void display::update_legend(zc_graph_base::data_type_t data_type) {
    // Get the label for this data type based on the current display mode.
    std::string label;
    // For each dataset map onto next legend entry for this data type.
    int num_datasets = sp_data_->get_dataset_count();
    std::vector<legend_entry_t> legend_entries;
    for (int i = 0; i < num_datasets; i++) {
        auto dataset = sp_data_->get_dataset(i);
        if (dataset->enabled) {
            legend_entry_t entry;
            if (data_type == zc_graph_base::Y_VALUE) {
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
	legends_.at(data_type)->set_entries(legend_entries);
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
	for (const auto& data_type : params_.data_types) {
		zc_graph_axis::orientation_t axis = graph_->get_axis(data_type);
 		legends_.at(data_type)->copy_label(params_.axis_params.at(axis).label.c_str());
		legends_.at(data_type)->show();
    }
}

// Update the supported data types for the current display mode based on the display mode parameters.
void display::update_supported_data_types() {
    params_.data_types.clear();
    for (const auto& axis : params_.axis_params) {
        switch (axis.second.orientation) {
        case zc_graph_axis::YL_AXIS:
            params_.data_types.insert(zc_graph_base::Y_VALUE);
            break;
        case zc_graph_axis::YR_AXIS:
            params_.data_types.insert(zc_graph_base::Y2_VALUE);
            break;
        case zc_graph_axis::R_AXIS:
            params_.data_types.insert(zc_graph_base::RADIUS);
            break;
        default:
            break;
        }
    }
}

