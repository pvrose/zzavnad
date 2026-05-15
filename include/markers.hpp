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
#pragma once

#include <FL/Fl_Group.H>

#include <cstdint>
#include <map>
#include <string>

class Fl_Button;
class Fl_Check_Button;
class Fl_Choice;
class Fl_Input;	

//! \brief This class defines the markers to add to appropriate display modes. 
//! 
//! 
class markers : public Fl_Group {

public:

	//! \brief Constructor.
	markers(int x, int y, int w, int h, const char* label = nullptr);

	//! \brief Destructor.
	~markers();

	//! \brief Load the previous settings for the display.
	void load_settings();
	//! \brief Save the current settings for the display.
	void save_settings();
	//! \brief Configure the widgets based on the current settings.
	void configure_widgets();
	//! \brief Instantiate the widgets for the display control.
	void create_widgets();

protected:

	//! \brief Callback for marker enabled/disabled.
	static void cb_marker_enabled(Fl_Widget* widget, void* data);

	//! \brief Callback for marker colour changed.
	static void cb_marker_colour(Fl_Widget* widget, void* data);

	//! \brief Callback for SWR marker value
	static void cb_swr_marker_value(Fl_Widget* widget, void* data);

	//! \brief Callback for TDR marker type: distance or time.
	static void cb_tdr_marker_type(Fl_Widget* widget, void* data);

	//! \brief Callback for TDR velocity factor material selection.
	static void cb_tdr_vf_material(Fl_Widget* widget, void* data);

	//! \brief Callback for TDR velocity factor value.
	static void cb_tdr_vf_value(Fl_Widget* widget, void* data);

	//! \brief Draw button for the SWR marker type.
	void draw_line_marker_button(Fl_Button* button, Fl_Color& colour);

	//! \brief Draw button for the frequency marker type.
	void draw_block_marker_button(Fl_Button* button, Fl_Color& colour);

	// Widgets
	Fl_Check_Button* swr_marker_checkbox_;        //!< Checkbox to enable/disable the SWR marker.
	Fl_Button* swr_marker_colour_button_;         //!< Button to select the colour of the SWR marker.
	Fl_Input* swr_marker_value_input_;            //!< Input to set the value of the SWR marker.

	Fl_Check_Button* frequency_marker_checkbox_;  //!< Checkbox to enable/disable the frequency marker.
	Fl_Button* frequency_marker_colour_button_;   //!< Button to select the colour of the frequency marker.

	Fl_Check_Button* tdr_marker_checkbox_;        //!< Checkbox to enable/disable the TDR marker.
	Fl_Button* tdr_marker_colour_button_;         //!< Button to select the colour of the TDR marker.
	Fl_Check_Button* tdr_marker_type_select_;     //!< Selector for the TDR marker type: distance (true) or time (false).
	Fl_Choice* tdr_vf_material_choice_;           //!< Choice widget for the TDR marker velocity factor material.
	Fl_Input* tdr_vf_input_;               //!< Input to set the TDR marker velocity factor (material = OTHER)

	// Atributes
	bool swr_marker_enabled_;        //!< Whether the SWR marker is enabled.
	Fl_Color swr_marker_colour_;     //!< The colour of the SWR marker.
	double swr_marker_value_;       //!< The value of the SWR marker.

	bool frequency_marker_enabled_;  //!< Whether the frequency marker is enabled.
	Fl_Color frequency_marker_colour_; //!< The colour of the frequency marker.

	bool tdr_marker_enabled_;        //!< Whether the TDR marker is enabled.
	Fl_Color tdr_marker_colour_;     //!< The colour of the TDR marker.
	bool tdr_marker_type_distance_; //!< Whether the TDR marker type is distance (true) or time (false).
	std::string tdr_vf_material_;   //!< The material selected for the TDR marker velocity factor.
	double tdr_vf_value_;         //!< The value of the TDR marker velocity factor (material = OTHER)

	// Table of materials and their corresponding velocity factors for TDR marker.
	const std::map<std::string, double> tdr_vf_materials_ = {
		{"Air", 1.0},
		{"PTFE", 0.69},
		{"Polyethylene", 0.66},
		{"PVC", 0.66},
		{"Air/Poly", 0.84},
		{"Other", 0.0} // Placeholder for user-defined velocity factor
	};
};