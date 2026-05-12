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

	//! \brief Enumeration of the types of markers that can be added to the display.
	enum marker_type : uint8_t {
		MT_SWR,
		MT_FREQUENCY,
		MT_COUNT
	};

	//! \brief Serialize the marker type for use in the settings.
	const std::map<marker_type, std::string> marker_type_names_ = {
		{MT_SWR, "SWR"},
		{MT_FREQUENCY, "Frequency"},
	};

	//! \brief Return whether the marker of the given type is enabled.
	bool is_marker_enabled(marker_type type) const;

	//! \brief Return the base colour for the marker of the given type.
	Fl_Color marker_color(marker_type type) const;

protected:

	//! \brief Callback for marker enabled/disabled.
	static void cb_marker_enabled(Fl_Widget* widget, void* data);

	//! \brief Callback for marker colour changed.
	static void cb_marker_colour(Fl_Widget* widget, void* data);

	//! \brief Draw button for the SWR marker type.
	void draw_swr_marker_button(Fl_Button* button);

	//! \brief Draw button for the frequency marker type.
	void draw_frequency_marker_button(Fl_Button* button);

	//! \brief Marker settings
	struct marker_settings_t {
		bool enabled;
		Fl_Color colour;
	};

	//! \brief Map of marker settings by marker type.
	std::map<marker_type, marker_settings_t> marker_settings_;

	//! \brief Map of marker type to corresponding checkbox widget.
	std::map<marker_type, Fl_Check_Button*> marker_checkboxes_;
	//! \brief Map of marker type to corresponding colour button widget.
	std::map<marker_type, Fl_Button*> marker_colour_buttons_;
	//! \brief Map of marker type to corresponding draw function for the colour button.
	std::map<marker_type, void (markers::*)(Fl_Button*)> marker_colour_draw_functions_ = {
		{MT_SWR, &markers::draw_swr_marker_button},
		{MT_FREQUENCY, &markers::draw_frequency_marker_button},
	};

};