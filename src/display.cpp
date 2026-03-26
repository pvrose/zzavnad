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

// Include the S-parameter data structures.
#include "sp_data.hpp"

// Include ZZACOMMON drawing constants
#include "zc_drawing.h"
#include "zc_settings.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>

// Include C++ standard library headers.
#include <complex>
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

const std::map<display_mode, display_mode_params_t> DISPLAY_MODE_PARAMS = {
    {DM_SWR, {"S11 SWR", false, &display::convert_sp_point_swr}},
    {DM_S11, {"S11 Raw data", true, &display::convert_sp_point_s11}},
    {DM_S11_RX, {"S11 R/X", true, &display::convert_sp_point_s11_rx}},
//    {DM_S11_RXPAR, {"S11 R||jX", true, &display::convert_sp_point_s11_rxpar}},
//    {DM_S11_SMITH, {"S11 Smith chart", false, &display::convert_sp_point_s11_smith}}, 
//    {DM_S11_MAG_PHASE, {"S11 Mag/Phase", true, &display::convert_sp_point_s11_mag_phase}},
};

// Constant complex 1.0+j0.0.
const std::complex<double> ONE = 1.0;

// Constructor for the display class.
display::display(int X, int Y, int W, int H, const char* L)
    : Fl_Window(X, Y, W, H, L) {
    box(FL_BORDER_BOX);
    align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    // Set the default display mode.
    set_display_mode(DM_SWR);
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
    // Add the display mode selection dropdown.
    ch_mode_ = new Fl_Choice(cx, cy, WBUTTON, HBUTTON, "Display Mode");
    for (const auto& mode_param : DISPLAY_MODE_PARAMS) {
        ch_mode_->add(mode_param.second.label.c_str());
    }
    ch_mode_->callback(cb_display_mode, this);
    ch_mode_->tooltip("Select the display mode for the graph");

    cy += HBUTTON + GAP;   
    // Add the graph widget for plotting the data.
    graph_ = new zc_graph(cx, cy, w() - 2 * GAP, h() - cy - GAP);
    graph_->box(FL_DOWN_FRAME);
    graph_->tooltip("Graph for plotting S-parameter data");

    end();
}

// Set the display mode for the graph.
void display::set_display_mode(display_mode mode) {
    display_mode_ = mode;
    // Set the conversion routine for converting sp_points to graph coordinates based on the selected display mode.
    convert_sp_point_ = DISPLAY_MODE_PARAMS.at(mode).convert_sp_point;
    // Set whether we need dual axes based on the selected display mode.
    dual_axes_ = DISPLAY_MODE_PARAMS.at(mode).dual_axes;
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
                convert_sp_point_(sp_point, *dataset, point_l, point_r);
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
    // None at the moment.
}

// Callback function for when the display mode selection is changed.
void display::cb_display_mode(Fl_Widget* widget, void* data) {
    display* disp = static_cast<display*>(data);
    int mode_index = disp->ch_mode_->value();
    auto mode_it = DISPLAY_MODE_PARAMS.begin();
    std::advance(mode_it, mode_index);
    if (mode_it != DISPLAY_MODE_PARAMS.end()) {
        disp->set_display_mode(mode_it->first);
        disp->configure_graph();
        disp->update_graph();
    }
}

// Convert sp_point into a single coordinate for plotting SWR vs frequency.
void display::convert_sp_point_swr(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r) {
    point_l.x = point.frequency;
    double s11_mag = std::abs(point.sparams.s11);
    point_l.y = (1 + s11_mag) / (1 - s11_mag); // SWR calculation from S11 magnitude.
}

// Convert sp_point into 2 coordinates for plotting S11 magnitude and phase vs frequency.
void display::convert_sp_point_s11(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r) {
    point_l.x = point.frequency;
    point_l.y = std::abs(point.sparams.s11); // S11 magnitude
    point_r.x = point.frequency;
    point_r.y = std::arg(point.sparams.s11) * 180.0 / M_PI; // S11 phase in degrees
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