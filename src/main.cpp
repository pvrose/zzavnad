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

#include "window.hpp"

#include "display.hpp"
#include "nvna_control.hpp"
#include "source_control.hpp"
#include "sp_data.hpp"

#include "zc_file_holder.h"

// include FLTK headers.
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>


// Declare global pointers.
display* display_;
display_control* display_control_;
nvna_control* nvna_control_;
source_control* source_control_;
sp_data* sp_data_;

// Externals included in zc_zzanvad.cpp
extern zc_file_holder* file_holder_;
extern std::string APP_NAME;
extern std::string APP_VERSION;

//! File holder customisation - control data
const std::map < uint8_t, file_control_t > FILE_CONTROL = {
	// ID, { filename, reference, read-only
	{ FILE_SETTINGS, { APP_NAME + ".json", false, false, 0 }},
	{ FILE_STATUS, { "status.txt", false, false, 0}},
	{ FILE_ICON_ZZA, { "rose.png", true, true, 0}}
};

// Customise FLTK feature
static void customise_fltk() {
	// Set default font size for all widgets
	FL_NORMAL_SIZE = 10;
	// FLTK 1.4 default contrast algorithm
	fl_contrast_mode(FL_CONTRAST_CIELAB);
#ifndef _WIN32
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER, "Courier New");
	Fl::set_font(FL_COURIER_BOLD, "Courier New Bold");
	Fl::set_font(FL_COURIER_ITALIC, "Courier New Italic");
	Fl::set_font(FL_COURIER_BOLD_ITALIC, "Courier New Bold Italic");
	// Use liberation fonts as closest to Windows fonts
	Fl::set_font(FL_TIMES, "Liberation Serif");
	Fl::set_font(FL_TIMES_BOLD, "Liberation Serif Bold");
	Fl::set_font(FL_TIMES_ITALIC, "Liberation Serif Italic");
	Fl::set_font(FL_TIMES_BOLD_ITALIC, "Liberation Serif Bold Italic");
	// Fl::set_font(FL_HELVETICA,            "Liberation Sans");
	// Fl::set_font(FL_HELVETICA_BOLD,       "Liberation Sans Bold");
	// Fl::set_font(FL_HELVETICA_ITALIC,     "Liberation Sans Italic");
	// Fl::set_font(FL_HELVETICA_BOLD_ITALIC,"Liberation Sans Bold Italic");	
#else 
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER, " Courier New");
	Fl::set_font(FL_COURIER_BOLD, "BCourier New");
	Fl::set_font(FL_COURIER_ITALIC, "ICourier New");
	Fl::set_font(FL_COURIER_BOLD_ITALIC, "PCourier New");
#endif
	// Default message properties
	fl_message_size_ = FL_NORMAL_SIZE;
	fl_message_font_ = 0;
	fl_message_title_default(APP_NAME.c_str());
	// Default scrollbar
	Fl::scrollbar_size(10);
}


// Main function for the application.
int main(int argc, char** argv) {
	file_holder_ = new zc_file_holder(argv[0], FILE_CONTROL);
    customise_fltk();
    // Create the main data item for the S-parameter data.
    sp_data_ = new sp_data();
    // Create the main application window.
    main_window window(800, 600, (APP_NAME + " " + APP_VERSION + " - VNA Analysis Software").c_str());
    // Run the FLTK event loop.
    return Fl::run();   
}