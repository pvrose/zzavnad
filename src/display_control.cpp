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
#include "display_control.hpp"

#include "display.hpp"
#include "displays/s11_swr.hpp"
#include "displays/s11_raw.hpp"
#include "displays/s11_rx.hpp"
#include "displays/s11_ma.hpp"
#include "displays/s21_gain.hpp"
#include "displays/s11_rj.hpp"

#include "zc_drawing.h"
#include "zc_settings.h"
#include "zc_status.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Enumerations.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

// Include C++ headers
#include <algorithm>
#include <cstdarg>
#include <string>

// Constructor for the display control class.
display_control::display_control(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L) {
	box(FL_BORDER_BOX);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Load the previous settings for the display control.
	load_settings();
	// Create the widgets for the display control.
	create_widgets();
	configure_widgets();
	configure_displays();
	update_displays();
	end();
	show();
}

// Destructor for the display control class.
display_control::~display_control() {
	// Save the current settings for the display control.
	save_settings();
	// Close any open display windows.
	close_displays();
}

// Load the previous settings for the display control.
void display_control::load_settings() {
	zc_settings settings;
	zc_settings dc_settings(&settings, "Display");
	// Ensure we have no parent window currently set when creating the display windows.
	Fl_Group* save_current = Fl_Group::current();
	Fl_Group::current(nullptr);
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		display* window = create_display(mode, 400, 300);
		dm_params_t& params = window->get_params();
		zc_settings mode_settings(&dc_settings, params.serial_name);
		bool enabled;
		mode_settings.get("Enabled", enabled, false);
		params.enabled = enabled;
		int width, height;
		mode_settings.get("Width", width, 400);
		mode_settings.get("Height", height, 400);
		window->size(width, height);
	}
	Fl_Group::current(save_current);
}

// Save the current settings for the display control.
void display_control::save_settings() {
	zc_settings settings;
	zc_settings dc_settings(&settings, "Display");
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		display* window = displays_.at(mode);
		dm_params_t& params = window->get_params();
		zc_settings mode_settings(&dc_settings, params.serial_name);
		mode_settings.set("Enabled", params.enabled);
		if (params.enabled) {
			mode_settings.set("Width", window->w());
			mode_settings.set("Height", window->h());
		}
	}
	settings.flush();
}

// Create the widgets for the display control.
void display_control::create_widgets() {
	int cx = x() + GAP;
	int cy = y() + HTEXT;
	// Work out how many columns of checkboxes we can fit in the width of the control.
	int WCOL = WLABEL + HBUTTON + GAP;
	int NCOL = std::max(1, (w() - GAP) / WCOL);
	// Now calculate how many rows we need to fit all the display modes.
	int NROW = (DM_COUNT + NCOL - 1) / NCOL;
	// Add a checkbox for each display mode.
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		dm_params_t& params = (displays_.at(mode))->get_params();
		Fl_Check_Button* cb_mode = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, params.serial_name.c_str());
		cb_mode->callback(cb_display_mode, (void*)mode);
		cb_mode->tooltip(("Enable display window for " + params.serial_name).c_str());
		cb_mode->align(FL_ALIGN_RIGHT);
		display_mode_checkboxes_[mode] = cb_mode;
		cx += WCOL;
		if ((mode + 1) % NCOL == 0) {
			cx = x() + GAP;
			cy += HBUTTON;
		}
	}
	resizable(nullptr);
	size(w(), NROW * HBUTTON + HTEXT + GAP);
	end();
	show();

	// Create windows for each enabled display mode.
	// Ensure we have no parent window currently set.
	Fl_Group* save_current = Fl_Group::current();
	Fl_Group::current(nullptr);
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		display* window = displays_.at(mode);
		if (window != nullptr && window->get_params().enabled) {
			window->show();
		}
	}
	// Restore the previous parent window.
	Fl_Group::current(save_current);
}

// Configure all active display windows based on the current settings.
void display_control::configure_displays() {
	for (auto& pair : displays_) {
		display_mode mode = pair.first;
		display* window = pair.second;
		if (window != nullptr && window->get_params().enabled) {
			window->configure_graph();
		}
	}
}

// Update all active display windows with the current S-parameter data.
void display_control::update_displays() {
	for (auto& pair : displays_) {
		display_mode mode = pair.first;
		display* window = pair.second;
		if (window != nullptr && window->get_params().enabled) {
			window->update_graph();
		}
	}
}

// Callback function for when a display mode selection is changed.
void display_control::cb_display_mode(Fl_Widget* widget, void* data) {
	display_mode mode = (display_mode)(uintptr_t)data;
	Fl_Check_Button* cb_mode = (Fl_Check_Button*)widget;
	bool enabled = cb_mode->value();
	display* window = display_control_->displays_.at(mode);
	dm_params_t& params = window->get_params();
	params.enabled = enabled;
	if (enabled) {
		window->configure_graph();
		window->update_graph();
		window->show();
	}
	else {
		// Disable and hide the display window for this mode.
		window->hide();
	}
}

// Configure the widgets based on the current settings for the display control.
void display_control::configure_widgets() {
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		display* window = displays_.at(mode);
		dm_params_t& params = window->get_params();
		Fl_Check_Button* cb_mode = display_mode_checkboxes_.at(mode);
		bool number_match = false;
		if (params.number_ports > sp_data_->get_number_ports()) {
			params.enabled = false;
		}
		else {
			number_match = true;
		}
		cb_mode->value(params.enabled);
		if (!number_match) {
			cb_mode->deactivate();
		}
		else {
			cb_mode->activate();
		}
		if (params.enabled && number_match) {
			window->show();
		}
		else {
			window->hide();
		}
	}
}

// Close all active display windows.
void display_control::close_displays() {
	for (auto& pair : displays_) {
		display* window = pair.second;
		if (window != nullptr) {
			window->hide();
		}
	}
}

// Create the display for the mode
display* display_control::create_display(display_mode mode, int W, int H) {
	display* window = nullptr;
	switch (mode) {
		case DM_SWR:
			window = new display_modes::s11_swr(W, H);
			break;
		case DM_S11:
			window = new display_modes::s11_raw(W, H);
			break;
		case DM_S11_RX:
			window = new display_modes::s11_rx(W, H);
			break;
		case DM_S11_MA:
			window = new display_modes::s11_ma(W, H);
			break;
		case DM_S11_RJ:
			window = new display_modes::s11_rj(W, H);
			break;
		case DM_S21_GAIN:
			window = new display_modes::s21_gain(W, H);
			break;
		default:
			break;
	}
	// Call create() after the derived class is fully constructed to avoid pure virtual function calls
	window->create();
	window->copy_label(window->get_params().title.c_str());
	displays_[mode] = window;
	return window;
}