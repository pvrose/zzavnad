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

// Include zzacommon items.
#include "zc_drawing.h"
#include "zc_utils.h"

// Include the main groups.
#include "display_control.hpp"
#include "nvna_control.hpp"
#include "source_control.hpp"
#include "sp_data.hpp"

// Include FLTK headers for the widgets used in the main window.
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>

// Include C++ standard library headers.
#include <algorithm>

// Constructor for the main application window.
main_window::main_window(int W, int H, const char* L)
    : Fl_Double_Window(W, H, L) {
    create_widgets();
}; 

// Destructor for the main application window.
main_window::~main_window() {
};

// Create the widgets for the main window.
void main_window::create_widgets() {
    int cx = x() + GAP;
    int cy = y() + GAP;
    int maxx = cx;
    // Add the source control panel on the left.
    source_control_ = new source_control(cx, cy, 100, 100, "Sources");
    // Add the nanoVNA control panel to the right of the source control panel.
    cx += source_control_->w();
    nvna_control_ = new nvna_control(cx, cy, 100, 100, "nanoVNA");
    // Calulate the total width of the control panels.
    cx += nvna_control_->w();
    // Add the displa control beneath the source control panel.
	cy += source_control_->h();
	cx = x() + GAP;
	display_control_ = new display_control(cx, cy, source_control_->w(), 100, "Displays");

	// Adjust the heights of the control panels to match each other.
    int h1 = source_control_->h() + display_control_->h();
	int h2 = nvna_control_->h();
    if (h2 > h1) {
		display_control_->size(display_control_->w(), h2 - source_control_->h());
	}
    else if (h1 > h2) {
        nvna_control_->size(nvna_control_->w(), h1);
    }

	int total_width = source_control_->w() + nvna_control_->w() + 2 * GAP;
	int total_height = std::max(h1, h2) + 2 * GAP;

	resizable(nullptr);
	size(total_width, total_height);
    
    end();
    show();

    // Set the global pointers to the main window's widgets so
    // that they can be accessed by other parts of the application.
    ::source_control_ = source_control_;
    ::nvna_control_ = nvna_control_;   
    ::display_control_ = display_control_;
};
    