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

#include "zc_graph.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Rect.H>
#include <FL/Fl_Widget.H>

// C++ standard library headers.
#include <cstdint>
#include <map>
#include <string>

//! \brief Header file for the display control class.
//! This manages the displaying of the various views of the S-parameter data.
//! It comprises a single checkbox for each of the display modes (display_mode), which
//! when selected allows a separate window to be opened showing the
//! data in the format for that display mode. 

//! \brief Display modes for the graph.
enum display_mode : uint8_t {
	DM_SWR,        //!< Display SWR vs frequency
	DM_S11,        //!< Display Raw S11 values vs frequency
	DM_S11_RX,     //!< Display S11 as resistance/reactance vs frequency
	//    DM_S11_RXPAR,  //!< Display S11 as resistance/reactance as R||jX vs frequency
	//    DM_S11_SMITH,  //!< Display S11 on a Smith chart
	DM_S11_MA,     //!< Display S11 magnitude and phase vs frequency
	DM_S21_GAIN,   //!< Display S21 gain (dB magnitude and phase) vs frequency
	DM_COUNT       //!< Number of display modes
};


//! Forward declarations.
class display;
class Fl_Check_Button;
struct dm_params_t;

class display_control : public Fl_Group {

public:

	//! \brief Constructor.
	//! \param X The x-coordinate of the display window.
	//! \param Y The y-coordinate of the display window.
	//! \param W The width of the display window.
	//! \param H The height of the display window.
	//! \param L The label for the display window.
	display_control(int X, int Y, int W, int H, const char* L = nullptr);

	//! \brief Destructor.
	~display_control();

	//! \brief Load the previous settings for the display.
	void load_settings();
	//! \brief Save the current settings for the display.
	void save_settings();
	//! \brief Configure the widgets based on the current settings.
	void configure_widgets();
	//! \brief Instantiate the widgets for the display control.
	void create_widgets();

	//! \brief Configure all active display.
	void configure_displays();
	//! \brief Update all active displays with the current S-parameter data.
	void update_displays();

	//! \brief Close all active display windows.
	void close_displays();

	//! \brief Callback function for when a display mode selection is changed.
	static void cb_display_mode(Fl_Widget* widget, void* data);

private:

	//! \brief create display for mode
	display* create_display(display_mode mode, int W, int H);

	//! \brief Map of display mode to corresponding checkbox widget.
	std::map<display_mode, Fl_Check_Button*> display_mode_checkboxes_;

	//! \brief Map of displays by display mode.
	std::map<display_mode, display*> displays_;


};

extern display_control* display_control_; //!< Global pointer to the display control instance.
