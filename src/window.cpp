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
    maxx = std::max(maxx, cx + nvna_control_->w() + GAP);
    // Add the display panel beneath both control panels with 16:9 aspect ratio.
    cy += std::max(source_control_->h(), nvna_control_->h()) + GAP;
    int display_width = maxx - x() - GAP;
    int display_height = display_width * 9 / 16;
    display_ = new display(x() + GAP, cy, display_width, display_height, "Display");
     // Calulate the total height of the window based on the control panels and display.    // Resize the main window to fit the control panels and display with some padding.
    resize(x(), y(), maxx - x() + GAP, cy + display_height - y() + GAP);

    end();
    show();

    ::source_control_ = source_control_;
    ::nvna_control_ = nvna_control_;   
    ::display_ = display_;
};
    