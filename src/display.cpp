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
#include "zc_graph.h"
#include "zc_settings.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Enumerations.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Double_Window.H>

#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>

// Include C++ standard library headers.
#include <complex>
#include <map>
#include <set>
#include <string>
#include <vector>

std::map<display_mode, dm_params_t> display_mode_params_ = {
    { DM_SWR, {
        "SWR",
        "SWR vs frequency",
        false,
    	{ 1e6, 30e6, "Hz", zc_graph::axis_xier_t::SI_PREFIX, 30, true},
        { 1.0F, 15.0F, "SWR", zc_graph::axis_xier_t::NONE, 30, true, 1.0F },
        { 1.0F, 15.0F, "SWR", zc_graph::axis_xier_t::NONE, 30, true, 1.0F },
		"Standing Wave Ratio",
        "",
        display::convert_sp_point_swr
    }},
    { DM_S11, {
        "S11 Raw",
        "S11 Raw data",
        true,
        { 1e6, 30e6, "Hz", zc_graph::axis_xier_t::SI_PREFIX, 30, true },
        { -1.0F, 1.0F, "Sr", zc_graph::axis_xier_t::NONE, 30, true },
        { -1.0F, 1.0F, "Si", zc_graph::axis_xier_t::NONE, 30, true },
        "S11 Real data",
		"S11 Imaginary data",
        display::convert_sp_point_s11
    }},
    { DM_S11_RX, {
        "S11 R+jX",
        "S11 Resistance and Reactance vs frequency",
        true,
        { 1e6, 30e6, "Hz", zc_graph::axis_xier_t::SI_PREFIX, 30, true },
        { 0.0F, 1000.0F, "\xCE\xA9(R)", zc_graph::axis_xier_t::SI_PREFIX, 30, true, 0.0F },
        {  -1000.0F, 1000.0F, "\xCE\xA9(X)", zc_graph::axis_xier_t::SI_PREFIX, 30, true },
		"S11 Resistance",
		"S11 Reactance",
        display::convert_sp_point_s11_rx
    }},
    { DM_S11_MA, {
        "S11 M+A",
        "S11 Magnitude and angle vs frequency",
        true,
        { 1e6, 30e6, "Hz", zc_graph::axis_xier_t::SI_PREFIX, 30, true },
        { 0.0F, 1.0F, "Mag", zc_graph::axis_xier_t::NONE, 30, true },
        { -180.0F, 180.0F, "degree", zc_graph::axis_xier_t::NONE, 60, true, -180.0F, 180.0F },
		"S11 Magnitude",
		"S11 Angle",
        display::convert_sp_point_s11_ma
    }}
};


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
    // Set the default display mode.
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
    int cy = y() + HTEXT;
	const int HLEGEND = HBUTTON * 7; // Height of the legend widgets.

    // Add the graph widget for plotting the data.
    graph_ = new zc_graph(cx, cy, w() - 2 * GAP, h() - HTEXT - HLEGEND);
    graph_->box(FL_BORDER_BOX);
	graph_->color(FL_WHITE);
    graph_->tooltip("Graph for plotting S-parameter data");

    cy += graph_->h();

	// Add the left axis legend.
	legend_l_ = new display_legend(cx, cy, w() / 2 - GAP, HLEGEND);
    legend_l_->box(FL_BORDER_BOX);
	legend_l_->tooltip("Legend for the left Y-axis");
    legend_l_->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT);

    legend_l_->resizable(nullptr);

	// Add the right axis legend.
    cx += legend_l_->w();
	legend_r_ = new display_legend(cx, cy, w() / 2 - GAP, HLEGEND);
	legend_r_->box(FL_BORDER_BOX);
    legend_r_->tooltip("Legend for the right Y-axis");
    legend_r_->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT);

	legend_r_->resizable(nullptr);

	resizable(graph_);

    end();
    show();
}

// Set the display mode for the graph.
void display::set_display_mode(display_mode mode) {
    display_mode_ = mode;
    // Set whether we need dual axes based on the selected display mode.
    dual_axes_ = display_mode_params_.at(mode).dual_axes;
	configure_widgets();
}

// Configure the graph data based on the current display mode and the S-parameter data in sp_data.
void display::configure_graph() {
    // Get the number of S-parameter datasets to plot.
    int num_datasets = 0;
    std::set<int> dataset_indices;
    for (int i = 0; i < sp_data_->get_dataset_count(); i++) {
        if (sp_data_->get_dataset(i)->enabled) {
            num_datasets++;
            dataset_indices.insert(i);
        }
    }
    // Get the number of sets of coordinates to plot for the current display mode.
    // Clear the existing graph data.
    // Map the enabled datasets to graph data sets and add them to the graph.
    dataset_to_graph_map_.clear();
    graph_->clear_data_sets();
    for (int dataset_index : dataset_indices) {
        zc_graph::data_set_t* graph_data_set = new zc_graph::data_set_t();
        auto dataset = sp_data_->get_dataset(dataset_index);
        graph_data_set->y_axis = zc_graph::Y_LEFT; // First set of points on left.
        graph_data_set->style = dataset->line_style_l;
        graph_data_set->data = nullptr; // Set the data pointer to null for now - we'll set it to the correct data after converting the points.
        int ix_l = graph_->add_data_set(*graph_data_set);
        dataset_to_graph_map_[dataset_index].index_l = ix_l;
        if (dual_axes_) {
            // If we need to plot 2 sets of points for this display mode, add a second graph data set for the second set of points.
            graph_data_set = new zc_graph::data_set_t();
            graph_data_set->y_axis = zc_graph::Y_RIGHT; // Second set of points on right.
            graph_data_set->style = dataset->line_style_r;
            graph_data_set->data = nullptr; // Set the data pointer to null for now
            
            int ix_r = graph_->add_data_set(*graph_data_set);
            dataset_to_graph_map_[dataset_index].index_r = ix_r;
        }
    }
	auto& params = display_mode_params_.at(display_mode_);
    if (dual_axes_) {
        graph_->set_params(params.axis_x_options, params.axis_l_options, params.axis_r_options);
    } else {
        graph_->set_params(params.axis_x_options, params.axis_l_options);
	}
	update_legend(zc_graph::Y_LEFT);
    if (dual_axes_) {
        update_legend(zc_graph::Y_RIGHT);
	}
	redraw();
}

// Update the graph with the current S-parameter data from sp_data, converting the S-parameter points to graph coordinates based on the current display mode.
void display::update_graph() {
    for (const auto& dataset_map : dataset_to_graph_map_) {
        int dataset_index = dataset_map.first;
        auto dataset = sp_data_->get_dataset(dataset_index);
        if (dataset->enabled) {
            std::vector<zc_graph::coord>* graph_points_l = new std::vector<zc_graph::coord>();
            std::vector<zc_graph::coord>* graph_points_r = new std::vector<zc_graph::coord>();
            for (const auto& sp_point : dataset->data) {
                zc_graph::coord point_l;
                zc_graph::coord point_r;
				dm_params_t& params = display_mode_params_.at(display_mode_);
                params.convert_sp_point(sp_point, *dataset, point_l, point_r);
                graph_points_l->push_back(point_l);
                if (dual_axes_) {
                    graph_points_r->push_back(point_r);
                }
            }
            int ix_l = dataset_map.second.index_l;
            graph_->set_data(ix_l, graph_points_l); // Set the data pointer to the converted points.
            if (dual_axes_) {
                int ix_r = dataset_map.second.index_r;
                graph_->set_data(ix_r, graph_points_r); // Set the data pointer to the converted points.
            }
        }
    }
	// After updating the data, we need to adjust the graph scales to fit the new data.
    graph_->set_drawing_area();
    graph_->adjust_scale_x();
    //graph_->adjust_scale_y(zc_graph::Y_LEFT);
    //if (dual_axes_) {
    //    graph_->adjust_scale_y(zc_graph::Y_RIGHT);
    //}
    redraw();
}

// Update the legend for each axis based on the current display mode and data.
void display::update_legend(zc_graph::y_axis_t axis) {
    // Get the label for this axis based on the current display mode.
    std::string label;
    // For each dataset map onto next legend entry for this axis.
    int num_datasets = sp_data_->get_dataset_count();
    std::vector<legend_entry_t> legend_entries;
    for (int i = 0; i < num_datasets; i++) {
        auto dataset = sp_data_->get_dataset(i);
        if (dataset->enabled) {
            legend_entry_t entry;
            if (axis == zc_graph::Y_LEFT) {
                entry.style = dataset->line_style_l;
            }
            else {
                entry.style = dataset->line_style_r;
            }
            switch (dataset->source) {
                case SPDS_ACTIVE:
                    entry.source = "nanoVNA " + dataset->timestamp;
                    break;
				case SPDS_FILE:
					entry.source = zc::terminal(dataset->filename);
                    break;
                case SPDS_KEPT:
                    entry.source = dataset->timestamp;
                    break;
            }
            // Set the legend entry for this dataset.
            legend_entries.push_back(entry);
        }
    }
    if (axis == zc_graph::Y_LEFT) {
        legend_l_->set_entries(legend_entries);
    }
    else {
        legend_r_->set_entries(legend_entries);
    }
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
    legend_l_->show();
	legend_l_->copy_label(display_mode_params_.at(display_mode_).legend_l.c_str());
    if (dual_axes_) {
        legend_r_->show();
		legend_r_->copy_label(display_mode_params_.at(display_mode_).legend_r.c_str());
    } else {
        legend_r_->hide();
	}
}

// Convert sp_point into 2 coordinates for plotting S11 as resistance and reactance vs frequency.
void display::convert_sp_point_s11_rx(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r) {
    point_l.x = point.frequency;
    std::complex<double> s11 = point.sparams.s11;
    // Get Z0 from the dataset.
    std::complex<double> z0 = dataset.z0;
    std::complex<double> z = z0 * (ONE + s11) / (ONE - s11); // Convert S11 to impedance.
    point_l.y = z.real(); // Resistance
    point_r.x = point.frequency;
    point_r.y = z.imag(); // Reactance
}

// Convert sp_point into 2 coordinates for plotting S11 as magnitude and phase vs frequency.
void display::convert_sp_point_s11_ma(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r) {
    point_l.x = point.frequency;
    std::complex<double> s11 = point.sparams.s11;
    point_l.y = std::abs(s11); // S11 magnitude
    point_r.x = point.frequency;
    point_r.y = std::arg(s11) * zc::RADIAN_DEGREE; // S11 phase in degrees
}

// Convert sp_point into 2 coordinates for plotting S11 raw data.
void display::convert_sp_point_s11(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r) {
    point_l.x = point.frequency;
    point_l.y = point.sparams.s11.real(); // S11 real part
    point_r.x = point.frequency;
    point_r.y = point.sparams.s11.imag(); // S11 imaginary part
}

// Convert sp_point into a single coordinate for plotting SWR vs frequency.
void display::convert_sp_point_swr(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r) {
    point_l.x = point.frequency;
    double s11_mag = std::abs(point.sparams.s11);
    point_l.y = (1 + s11_mag) / (1 - s11_mag); // SWR calculation from S11 magnitude.
}
