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
#include "zc_file_holder.h"
#include "zc_fltk.h"
#include "zc_utils.h"

// Include the main groups.
#include "display_control.hpp"
#include "marker_table.hpp"
#include "markers.hpp"
#include "nvna_control.hpp"
#include "source_control.hpp"
#include "sp_data.hpp"

// Include FLTK headers for the widgets used in the main window.
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_PNG_Image.H>

// Include C++ standard library headers.
#include <algorithm>

// Constructor for the main application window.
main_window::main_window(int W, int H, const char* L)
    : Fl_Double_Window(W, H, L) {
    callback(cb_exit, this);
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
    // Add the help panel across the top of the window.
    help_group_ = new Fl_Group(cx, cy, 3 * HBUTTON, HBUTTON);
    cx = help_group_->x();
    Fl_Box* dummy = new Fl_Box(cx, cy, HBUTTON, HBUTTON);
    dummy->box(FL_NO_BOX);
    cx += dummy->w() + GAP;
    Fl_Button* btn_open_html = new Fl_Button(cx, cy, HBUTTON, HBUTTON);
    btn_open_html->callback(cb_open_html, this);
    btn_open_html->tooltip("Open the help documentation in HTML format.");
    std::string fn_html = file_holder_->get_filename(FILE_ICON_ZZA);
    Fl_PNG_Image* img_html = new Fl_PNG_Image(fn_html.c_str());
    Fl_Image* icon = img_html->copy(HBUTTON, HBUTTON);
    btn_open_html->image(icon);
    cx += HBUTTON;
    Fl_Button* btn_open_pdf = new Fl_Button(cx, cy, HBUTTON, HBUTTON);
    btn_open_pdf->callback(cb_open_pdf, this);
    btn_open_pdf->tooltip("Open the help documentation in PDF format.");
    std::string fn_pdf = file_holder_->get_filename(FILE_ICON_PDF);
    Fl_PNG_Image* img_pdf = new Fl_PNG_Image(fn_pdf.c_str());
    Fl_Image* icon_pdf = img_pdf->copy(HBUTTON, HBUTTON);
    btn_open_pdf->image(icon_pdf);
    help_group_->end();
    help_group_->resizable(dummy);

    cy += help_group_->h() + GAP;
    cx = x() + GAP;

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
	// Add the markers control beneath the display control panel.
	cy += display_control_->h();
	markers_ = new markers(cx, cy, source_control_->w(), 100, "Markers");

    // Set the global pointers to the main window's widgets so
    // that they can be accessed by other parts of the application.
    ::source_control_ = source_control_;
    ::nvna_control_ = nvna_control_;
    ::display_control_ = display_control_;

	// Adjust the heights of the control panels to match each other.
    int h1 = source_control_->y() +source_control_->h() + display_control_->h() + markers_->h();
	int h2 = nvna_control_->y() + nvna_control_->h();
    if (h2 > h1) {
        markers_->size(markers_->w(), h2 - display_control_->h() - source_control_->h() - source_control_->y());
	}
    else if (h1 > h2) {
        nvna_control_->size(nvna_control_->w(), h1);
    }

	int total_width = source_control_->w() + nvna_control_->w() + 2 * GAP;
	int total_height = std::max(h1, h2) + 2 * GAP;
    help_group_->size(total_width - 3 * GAP, help_group_->h());

	resizable(nullptr);
	size(total_width, total_height);
    
    end();
    show();

};
    
// Callback function for the "Exit" menu item.
void main_window::cb_exit(Fl_Widget* widget, void* data) {
    main_window* win = (main_window*)widget;
	win->display_control_->close_displays();
	marker_table_->hide();
	default_callback((Fl_Window*)widget, data);
};

// Callback function for the "Open HTML" button.
void main_window::cb_open_html(Fl_Widget* widget, void* data) {
    std::string full_path = file_holder_->get_directory(FDD_DOCUMENTS) +
        "userguide/html/index.html";
    open_help_file(full_path);
}

// Callback function for the "Open PDF" button.
void main_window::cb_open_pdf(Fl_Widget* widget, void* data) {
    std::string full_path = file_holder_->get_directory(FDD_DOCUMENTS) +
        "userguide/ZZAVNAD.pdf";
    open_help_file(full_path);
}

void main_window::open_help_file(const std::string& full_filename) {
#ifdef _WIN32
	HINSTANCE result = ShellExecute(NULL, "open", full_filename.c_str(), NULL, NULL, SW_SHOWNORMAL);
	if ((intptr_t)result <= 32) {
		printf("ZZAVNAD: Error opening HTML %s. Error code: %d", 
			full_filename.c_str(),
			(int)(intptr_t)result);
	}
#else 
	std::string cmd = "xdg-open \"" + full_filename + "\"";
	int res = system(cmd.c_str());
	if (res != 0) {
		printf("ZZAVNAD: Error opening HTML %s. Error code: %d", 
			full_filename.c_str(),
			res);
	}
#endif
}
