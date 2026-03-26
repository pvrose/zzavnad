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
#include "display.hpp"
#include "nvna_control.hpp"
#include "source_control.hpp"
#include "sp_data.hpp"

// Include FLTK headers for the widgets used in the main window.
#include <FL/Fl_Box.H>

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

	// Using the larger of the two control panels - adjust both to have the same height.
	int max_h = std::max(source_control_->h(), nvna_control_->h());
	source_control_->size(source_control_->w(), max_h);
	nvna_control_->size(nvna_control_->w(), max_h);

    int max_w = cx - x() - GAP;

    // Add the display panel beneath both control panels with 16:9 aspect ratio.
	int display_width = source_control_->w() + nvna_control_->w();
    int display_height = display_width * 9 / 16;
    cx = x() + GAP;
    cy = source_control_->y() + source_control_->h();

    display_ = new display(cx, cy, display_width, display_height, "Display");
    // Calulate the total height of the window based on the control panels and display.
    // Resize the main window to fit the control panels and display.
	int total_height = source_control_->h() + display_->h() + 2 * GAP;
	int total_width = display_width + 2 * GAP;
	resizable(nullptr);
	size(total_width, total_height);
    
    end();
    show();

    // Set the global pointers to the main window's widgets so
    // that they can be accessed by other parts of the application.
    ::source_control_ = source_control_;
    ::nvna_control_ = nvna_control_;   
    ::display_ = display_;
};
    