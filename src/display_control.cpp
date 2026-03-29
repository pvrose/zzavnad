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
	// Create the widgets for the display control.
	create_widgets();
	// Load the previous settings for the display control.
	load_settings();
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
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		dm_params_t& params = display_mode_params_.at(mode);
		zc_settings mode_settings(&dc_settings, params.serial_name);
		bool enabled;
		mode_settings.get("Enabled", enabled, false);
		params.enabled = enabled;
		if (enabled) {
			mode_settings.get("Left", params.left, 0);
			mode_settings.get("Top", params.top, 0);
			mode_settings.get("Width", params.width, 400);
			mode_settings.get("Height", params.height, 300);
			mode_settings.get("Title", params.title, std::string(""));
		}
	}
}

// Save the current settings for the display control.
void display_control::save_settings() {
	zc_settings settings;
	zc_settings dc_settings(&settings, "Display");
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		dm_params_t params = display_mode_params_.at(mode);
		zc_settings mode_settings(&dc_settings, params.serial_name);
		mode_settings.set("Enabled", params.enabled);
		if (params.enabled) {
			mode_settings.set("Left", params.left);
			mode_settings.set("Top", params.top);
			mode_settings.set("Width", params.width);
			mode_settings.set("Height", params.height);
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
		dm_params_t& params = display_mode_params_.at(mode);
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

	// Create windows for each enabled display mode.
	// Ensure we have no parent window currently set.
	Fl_Group* save_current = Fl_Group::current();
	Fl_Group::current(nullptr);
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		dm_params_t& params = display_mode_params_[mode];
		if (params.enabled) {
			params.window = new display(params.left, params.top, params.width, params.height, params.title.c_str());
			params.window->set_display_mode(mode);
			params.window->show();
		}
	}
	// Restore the previous parent window.
	Fl_Group::current(save_current);
}

// Configure all active display windows based on the current settings.
void display_control::configure_displays() {
	for (auto& pair : display_mode_params_) {
		display_mode mode = pair.first;
		dm_params_t& params = pair.second;
		if (params.window != nullptr) {
			params.window->configure_graph();
		}
	}
}

// Update all active display windows with the current S-parameter data.
void display_control::update_displays() {
	for (auto& pair : display_mode_params_) {
		dm_params_t& params = pair.second;
		if (params.window != nullptr) {
			params.window->update_graph();
		}
	}
}

// Callback function for when a display mode selection is changed.
void display_control::cb_display_mode(Fl_Widget* widget, void* data) {
	display_mode mode = (display_mode)(uintptr_t)data;
	Fl_Check_Button* cb_mode = (Fl_Check_Button*)widget;
	bool enabled = cb_mode->value();
	dm_params_t& params = display_mode_params_.at(mode);
	if (enabled) {
		// Create and enable the display window for this mode.
		if (params.window == nullptr) {
			params.window = new display(params.left, params.top, params.width, params.height, params.title.c_str());
			params.window->set_display_mode(mode);
		}
		params.window->configure_graph();
		params.window->update_graph();
		params.window->show();
	}
	else {
		// Disable and hide the display window for this mode.
		if (params.window != nullptr) {
			params.window->hide();
		}
	}
}

// Configure the widgets based on the current settings for the display control.
void display_control::configure_widgets() {
	for (display_mode mode = static_cast<display_mode>(0); mode < DM_COUNT; mode = static_cast<display_mode>(mode + 1)) {
		dm_params_t& params = display_mode_params_.at(mode);
		Fl_Check_Button* cb_mode = display_mode_checkboxes_.at(mode);
		cb_mode->value(params.enabled);
		if (params.enabled && params.window != nullptr) {
			cb_mode->value(1);
			params.window->show();
		}
		else if (params.window != nullptr) {
			cb_mode->value(0);
			params.window->hide();
		}
	}
}

// Close all active display windows.
void display_control::close_displays() {
	for (auto& pair : display_mode_params_) {
		dm_params_t& params = pair.second;
		if (params.window != nullptr) {
			params.window->hide();
			delete params.window;
			params.window = nullptr;
		}
	}
}