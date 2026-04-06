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

#include <FL/Fl_Double_Window.H>

#include <cstdint>
#include <map>

//! Forward declarations.
class Fl_Choice;
class Fl_Group;
class Fl_Widget;
class zc_graph;
class display;
class display_legend;
enum display_mode : uint8_t;

//! \brief Various parameters for the display modes.
//! This structure holds all the parameters for each display mode,
//! and will be set by the individual overloads.
struct dm_params_t {
    std::string serial_name = "";        //!< Name for use in serialisation.
	std::string title = "";              //!< Title of the display window for this mode.
    bool dual_axes = false;              //!< Has both left and right axes.
	zc_graph::options_t axis_x_options;  //!< Options for x axis
    zc_graph::options_t axis_l_options;  //!< Options for left y axis
    zc_graph::options_t axis_r_options;  //!< Options for right y axis
	std::string legend_l = "";              //!< Legend for left y axis data.
	std::string legend_r = "";              //!< Legend for right y axis data.
	int number_ports = 1;                 //!< Number of valid ports required for this display mode (1 or 2).
    bool enabled = false;                //!< Whether this display mode is being shown.
};

//! \brief The display class manages the plotting of S-parameter data on the graph.
//! It will form the base calss for the various display modes.
class display : public Fl_Double_Window {
public:
    
    //! Constructor for the display class.
    //! \param W The width of the display window.
    //! \param H The height of the display window.
    //! \param L The label for the display window.
    display(int W, int H, const char* L = nullptr);
    
    //! Destructor for the display class.
    ~display();

    //! \brief Configure the graph data.
    void configure_graph();

    //! \brief Update the graph with the current S-parameter data.
    void update_graph();

    //! Convert sp_point into 1 or 2 sets of coordinates for plotting based on the current display mode.
    //! \param sp_point The sp_point to convert.
    //! \param dataset The dataset to which this point belongs, which may be needed for some display modes.
    //! \param point_l The first coordinate to plot for this point (e.g. SWR, or S11 magnitude).
    //! \param point_r The second coordinate to plot for this point (e.g. S11 phase).
	virtual void convert_sp_point(
        const sp_point& point,
        const sp_data_entry& dataset,
        zc_graph::coord& point_l,
        zc_graph::coord& point_r) const = 0;

	dm_params_t& get_params() { return params_; }

protected:

    dm_params_t params_ = {}; //!< The parameters for this display mode, set by the individual overloads.

    //! The current display mode.
    display_mode display_mode_ = static_cast<display_mode>(0);

    //! Create the widgets for the display window.
    void create_widgets();
    //! Load the previous settings for the display.
    void load_default_settings();
    //! Save the current settings for the display.
    void save_current_settings();
    //! Configure the widgets based on the current settings.
    void configure_widgets();

    //! The create methods to be run after configuration with
    //! theinherited display mode parameters.
    void create();

    //! Update the legend for each axis based on the current display mode and data.
	void update_legend(zc_graph::y_axis_t axis);

    //! Each sp_data dataset can be mapped to 1 or 2 graph data sets for plotting.
    struct graph_datasets_t {
        int index_l = -1; //!< The index of the first graph data set for this sp_data dataset (e.g. for SWR or S11 magnitude).
        int index_r = -1; //!< The index of the second graph data set for this sp_data dataset (e.g. for S11 phase). This may be -1 if only one graph data set is needed for this display mode.
    };
    //! Map sp_data datasets to graph data sets for plotting. 
    std::map<int, graph_datasets_t> dataset_to_graph_map_ = {};

    //! The graph widget for plotting the data.
    zc_graph* graph_ = nullptr;

	//! The legend widgets for the left and right axes.
	display_legend* legend_l_ = nullptr;
    display_legend* legend_r_ = nullptr;
};

extern display* display_;
