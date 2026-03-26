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

//! \file display.hpp
//! \brief Provides the display class which manages the plotting of S-parameter data.

#pragma once

#include "sp_data.hpp"
#include "zc_graph.h"

#include <FL/Fl_Window.H>

#include <cstdint>

//! Forward declarations.
class Fl_Choice;
class Fl_Group;
class zc_graph;

enum display_mode : uint8_t {
    DM_NONE,       //!< No display mode selected
    DM_SWR,        //!< Display SWR vs frequency
    DM_S11,        //!< Display Raw S11 values vs frequency
    DM_S11_RX,     //!< Display S11 as resistance/reactance vs frequency
//    DM_S11_RXPAR,  //!< Display S11 as resistance/reactance as R||jX vs frequency
//    DM_S11_SMITH,  //!< Display S11 on a Smith chart
//    DM_S11_MAG_PHASE, //!< Display S11 magnitude and phase vs frequency
};

//! \brief The display class manages the plotting of S-parameter data on the graph.
class display : public Fl_Window {
public:
    
    //! Constructor for the display class.
    //! \param X The x-coordinate of the display window.
    //! \param Y The y-coordinate of the display window.
    //! \param W The width of the display window.
    //! \param H The height of the display window.
    //! \param L The label for the display window.
    display(int X, int Y, int W, int H, const char* L = nullptr);
    
    //! Destructor for the display class.
    ~display();

    //! Set the display mode for the graph.
    //! \param mode The display mode to set.
    void set_display_mode(display_mode mode);

    //! Get the current display mode for the graph.
    //! \return The current display mode.
    display_mode get_display_mode() const { return display_mode_; }

    //! \brief Configure the graph data.
    void configure_graph();

    //! \brief Update the graph with the current S-parameter data.
    void update_graph();

    //! Convert sp_point into 1 or 2 sets of coordinates for plotting based on the current display mode.
    //! \param sp_point The sp_point to convert.
    //! \param dataset The dataset to which this point belongs, which may be needed for some display modes.
    //! \param point_l The first coordinate to plot for this point (e.g. SWR, or S11 magnitude).
    //! \param point_r The second coordinate to plot for this point (e.g. S11 phase).
    static void convert_sp_point_swr(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r);
    static void convert_sp_point_s11(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r);
    static void convert_sp_point_s11_rx(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r);


private:
    //! Create the widgets for the display window.
    void create_widgets();
    //! Load the previous settings for the display.
    void load_default_settings();
    //! Save the current settings for the display.
    void save_current_settings();
    //! Configure the widgets based on the current settings.
    void configure_widgets();

    //! Callback function for when the display mode selection is changed.
    static void cb_display_mode(Fl_Widget* widget, void* data);

    //! The current display mode.
    display_mode display_mode_ = DM_NONE;

    //! The display has both a left and right Y-axis. 
    bool dual_axes_ = false;

    //! Each sp_data dataset can be mapped to 1 or 2 graph data sets for plotting.
    struct graph_datasets_t {
        int index_l = -1; //!< The index of the first graph data set for this sp_data dataset (e.g. for SWR or S11 magnitude).
        int index_r = -1; //!< The index of the second graph data set for this sp_data dataset (e.g. for S11 phase). This may be -1 if only one graph data set is needed for this display mode.
    };
    //! Map sp_data datasets to graph data sets for plotting. 
    std::map<int, graph_datasets_t> dataset_to_graph_map_ = {};

    //! Selected conversion routines for the current display mode.
    void (*convert_sp_point_)(const sp_point& point, const sp_data_entry& dataset, zc_graph::coord& point_l, zc_graph::coord& point_r) = nullptr;

    //! The graph widget for plotting the data.
    zc_graph* graph_ = nullptr;
    //! The choice widget for selecting the display mode.
    Fl_Choice* ch_mode_ = nullptr;



};

extern display* display_;