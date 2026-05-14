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

#include "calib_data.hpp"
#include "display.hpp"
#include "nvna_control.hpp"
#include "source_control.hpp"
#include "sp_data.hpp"

#include "zc_file_holder.h"
#include "zc_fltk.h"
#include "zc_status.h"

// include FLTK headers.
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>

// Include C++ standard library headers.
#include <cstdint>
#include <map>
#include <string>

// Include Windows headers for console colour support
#ifdef _WIN32
#include <windows.h>
#endif


// Declare global pointers.
display* display_;
display_control* display_control_;
nvna_control* nvna_control_;
source_control* source_control_;
sp_data* sp_data_;
calib_data* calib_data_;

// Externals included in zc_zzanvad.cpp
extern zc_file_holder* file_holder_;
extern std::string APP_NAME;
extern std::string APP_VERSION;



//! File holder customisation - control data
const std::map < uint8_t, file_control_t > FILE_CONTROL = {
	// ID, { filename, reference, read-only
	{ FILE_SETTINGS, { "ZZAVNAD.json", false, false, 0 }},
	{ FILE_STATUS, { "status.txt", false, false, 0}},
	{ FILE_ICON_ZZA, { "rose.png", true, true, 0}}
};

// Main function for the application.
int main(int argc, char** argv) {
#ifdef _WIN32
	// Enable Windows console colour support (ANSI escape sequences)
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);
	}
#endif

	file_holder_ = new zc_file_holder(argv[0], FILE_CONTROL);
    zc::customise_fltk();
	status_ = new zc_status(zc_status::HAS_CONSOLE, {});
    // Create the main data item for the S-parameter data.
    sp_data_ = new sp_data();
	// Create the main data item for the calibration data.
	calib_data_ = new calib_data();
    // Create the main application window.
    main_window* window = new main_window(800, 600, (APP_NAME + " " + APP_VERSION + " - VNA Analysis Software").c_str());
    // Run the FLTK event loop.
    int result = Fl::run();   
	// Clean up and exit.
	delete window;
	delete calib_data_;
	delete sp_data_;
	delete file_holder_;
	return result;
}