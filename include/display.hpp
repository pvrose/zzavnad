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

#include "zc_graph_axis.h"
#include "zc_graph_base.h"

#include <FL/Fl_Double_Window.H>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

//! Forward declarations.
class Fl_Choice;
class Fl_Group;
class Fl_Widget;
class display;
class display_legend;
enum display_mode : uint8_t;

typedef std::map<zc_graph_axis::orientation_t, zc_graph_axis::axis_params_t> axis_params_map_t;

//! \brief Various parameters for the display modes.
//! This structure holds all the parameters for each display mode,
//! and will be set by the individual overloads.
struct dm_params_t {
    std::string serial_name = "";        //!< Name for use in serialisation.
	std::string title = "";              //!< Title of the display window for this mode.
	axis_params_map_t axis_params;       //!< Map of axis parameters for each axis in this display mode.
	int number_ports = 1;                //!< Number of valid VNA ports required for this display mode (1 or 2).
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

    //! \brief Data definition for the entirety of graph data.
    typedef std::map<zc_graph_axis::orientation_t, zc_graph_base::data_set_t*> graph_data_map_t;

    //! \brief data ranges for each data type,
	typedef std::map<zc_graph_base::data_type_t, zc_graph_axis::range> graph_data_ranges_t;

	//! Convert sp_data into graph coordinates for plotting, based on the current display mode.
    //! \param dataset The dataset to which this point belongs, which may be needed for some display modes.
    //! \param coords The coordinates to plot for this point (e.g. SWR, or S11 magnitude and phase).
	virtual void convert_sp_to_coords(
        const sp_data_entry& dataset,
        graph_data_map_t& coords,
        graph_data_ranges_t& ranges) const = 0;

	dm_params_t& get_params() { return params_; }

    void update_range_point(
        zc_graph_axis::range& range,
        float value
    ) const {
        if (value < range.min) {
            range.min = value;
        }
        if (value > range.max) {
            range.max = value;
        }
    }

    //! The create method to be run after construction.
    //! This must be called after the derived class is fully constructed to avoid pure virtual function calls.
    //! It will configure display mode parameters and create all widgets.
    void create();

    //! \brief Get the range of data supported by the axis for the data.
    virtual zc_graph_axis::range get_range(
        zc_graph_base::data_type_t data_type
    ) = 0;

	//! \brief Get all data ranges for the current display mode.
	virtual graph_data_ranges_t get_all_data_ranges() = 0;


protected:

    //! \brief Add the specific graph widget for this display mode.
	virtual zc_graph_base* create_graph(int X, int Y, int W, int H) = 0;

	//! \brief Configure the display mode parameters for this display mode.
	virtual void configure_dm_params() = 0;

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

    //! Update the legend for each axis based on the current display mode and data.
	void update_legend(zc_graph_axis::orientation_t axis);

    //! Each sp_data dataset can be mapped to a number of graph data sets for plotting.
	typedef std::map<zc_graph_axis::orientation_t, int> graph_data_indices_t; //!< The indices of the graph data sets for a given sp_data dataset.
    //! Map sp_data datasets to graph data sets for plotting. 
    std::map<int, graph_data_indices_t> dataset_to_graph_map_ = {};

    //! The graph widget for plotting the data.
    zc_graph_base* graph_ = nullptr;

	//! The legend widgets for the left and right axes.
	std::map<zc_graph_axis::orientation_t, display_legend*> legends_ = {};
};

extern display* display_;
