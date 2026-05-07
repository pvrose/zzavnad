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

#include "zc_graph_.h"

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

//! \brief Parameters for each axis to be set by the individual display modes.
struct axis_params_t {
    zc_graph_::modifier_t unit_modifier = zc_graph_::NO_MODIFIER; //!< Modifier for axis units (e.g. SI prefix or power of 10).
    std::string unit = "";              //!< Unit to display on the axis (e.g. "Hz").
    std::string label = "";             //!< Base label for the axis (e.g. "Frequency").
    zc_graph_::range_t outer_range;       //!< Range of data values for this axis.
    zc_graph_::range_t inner_range;       //!< Range of data values currently displayed for this axis (may be zoomed or scrolled).
    zc_graph_::range_t default_range;     //!< Default range for this axis in the absence of data.
};

//! \brief The data and how it is to be displayed for each axis.
struct dm_data_set_t {
	std::vector<zc_graph_::data_point_t>* data; //!< Pointer to a vector of data points to be plotted for this axis.
	zc_line_style style; //!< Line style to use for plotting this data set.

	// Default constructor initializes to an empty data set with a solid black line style.
	dm_data_set_t() : data(
        new std::vector<zc_graph_::data_point_t>()), 
        style({ FL_SOLID, 1, FL_BLACK }) {}
};

//! \brief Various parameters for the display modes.
//! This structure holds all the parameters for each display mode,
//! and will be set by the individual overloads.
struct dm_params_t {
    std::string serial_name = "";        //!< Name for use in serialisation.
	std::string title = "";              //!< Title of the display window for this mode.
	std::map<int, axis_params_t> axis_params;       //!< Map of axis parameters for each axis in this display mode.
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
    typedef std::map<int, dm_data_set_t*> graph_data_map_t;

    //! \brief data ranges for each data type,
	typedef std::map<int, zc_graph_::range_t> graph_data_ranges_t;

	//! Convert sp_data into graph coordinates for plotting, based on the current display mode.
    //! \param dataset The dataset to which this point belongs, which may be needed for some display modes.
    //! \param coords The coordinates to plot for this point (e.g. SWR, or S11 magnitude and phase).
	virtual void convert_sp_to_coords(
        const sp_data_entry& dataset,
        graph_data_map_t& coords,
        graph_data_ranges_t& ranges) const = 0;

	dm_params_t& get_params() { return params_; }

    //! The create method to be run after construction.
    //! This must be called after the derived class is fully constructed to avoid pure virtual function calls.
    //! It will configure display mode parameters and create all widgets.
    void create();

    //! \brief Get the range of data supported by the axis for the data.
    zc_graph_::range_t  get_range(
        int axis
    ) {
        return graph_->get_axis_range(axis);
    }

	//! \brief Get all data ranges for the current display mode.
	virtual graph_data_ranges_t get_all_data_ranges() = 0;

    //! Add markers to the graph for the current display mode and data.
	//! The base implementation does nothing, but individual display modes can override this to add markers for specific data values (e.g. SWR=3).
    virtual void add_markers() {};



protected:

    //! \brief Add the specific graph widget for this display mode.
	virtual zc_graph_* create_graph(int X, int Y, int W, int H) = 0;

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
	void update_legend(int axis);

	//! Update supported data types from axis parameters.
	void update_supported_data_types();

	//! Add frequency markers to the graph for the current display mode and data.
	void add_frequency_markers();

    //! The graph widget for plotting the data.
    zc_graph_* graph_ = nullptr;

	//! The legend widgets for the left and right axes.
	std::map<int, display_legend*> legends_ = {};

};

extern display* display_;
