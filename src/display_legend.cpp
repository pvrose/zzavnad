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

#include "display_legend.hpp"

#include "source_control.hpp"

// Include zzacommon items.
#include "zc_drawing.h"

// Include FLTK widgets
#include <FL/Enumerations.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>

// Include C++ standard library headers.
#include <string>
#include <vector>

// Constructor for the display legend.
display_legend::display_legend(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L) {
	box(FL_BORDER_BOX);
	align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	create_widgets();
}

// Destructor for the display legend.
display_legend::~display_legend() {
}

// Create the widgets for the display legend.
void display_legend::create_widgets() {
	int cx = x() + GAP;
	int cy = y() + HTEXT;
	for (int i = 0; i <= NUM_FILE_SOURCES; i++) {
		line_style_button* line_btn = new line_style_button(cx, cy, WBUTTON / 2, HBUTTON);
		line_btn->tooltip("Shows the line style for this dataset.");
		line_style_buttons_.push_back(line_btn);
		cx += WBUTTON / 2;
		Fl_Box* label = new Fl_Box(cx, cy, WLABEL, HBUTTON);
		label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		labels_.push_back(label);
		cy += HBUTTON;
		cx = x() + GAP;
	}
	end();
}

// Set the legend entries.
void display_legend::set_entries(const std::vector<legend_entry_t>& entries) {
	for (size_t i = 0; i < line_style_buttons_.size(); i++) {
		line_style_button* line_btn = line_style_buttons_[i];
		if (i < entries.size()) {
			line_btn->show();
			line_btn->value(entries[i].style);
			labels_[i]->copy_label(entries[i].source.c_str());
		} else {
			line_btn->hide();
			line_btn->value(zc_line_style());
			line_btn->box(FL_FLAT_BOX);
			labels_[i]->copy_label("");
		}
	}
}
