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
#include "zc_button_dialog.h"
#include "zc_drawing.h"

// Include FLTK widgets
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>

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
	int cw = w() - 2 * GAP;
	int ch = h() - HTEXT - GAP - Fl::scrollbar_size();
	scroll_ = new Fl_Scroll(cx, cy, cw, ch);
	scroll_->box(FL_FLAT_BOX);
	scroll_->type(Fl_Scroll::VERTICAL);
	end();
}

// Set the legend entries.
void display_legend::set_entries(const std::vector<legend_entry_t>& entries) {
	entries_ = entries;
	add_entry_groups();
	redraw();
}

// Add entry groups for the current entries.
void display_legend::add_entry_groups() {
	// Delete the existing entries in scroll_
	int num_children = scroll_->children();
	for (int i = num_children - 1; i >= 0; i--) {
		entry_group* entry = dynamic_cast<entry_group*>(scroll_->child(i));
		if (entry) {
			// use pointer as index number is not valid after deletion
			scroll_->remove(entry);
			Fl::delete_widget(entry);
		}
	}
	Fl::check(); // Ensure that the widgets are deleted before we add new ones.
	// Add the entry groups to the scroll widget, arranging them in rows and columns.
	int cx = x() + GAP;
	int cy = y() + GAP;
	const int WENTRY = WBUTTON / 2 + WLLABEL;
	for (size_t i = 0; i < entries_.size(); i++) {
		entry_group* entry = new entry_group(cx, cy, WENTRY, HBUTTON, entries_[i]);
		scroll_->add(*entry);
		// Add them in two columsn
		if (i % 2) {
			cx = x() + GAP;
			cy += HBUTTON;
		}
		else {
			cx += WENTRY + GAP;
		}
	}
	scroll_->end();
	scroll_->scroll_to(0, 0); // Scroll to the top after adding the entries.
	scroll_->redraw();
}

// Constructor for entry_group.
display_legend::entry_group::entry_group(int X, int Y, int W, int H, const legend_entry_t& entry) :
	Fl_Group(X, Y, W, H) {
	line_style_button* line_btn = new line_style_button(X, Y, WBUTTON / 2, H);
	line_btn->tooltip("Shows the line style for this dataset.");
	line_btn->box(FL_FLAT_BOX);
	line_btn->type(ZC_BUTTON_DIALOG_OUTPUT);
	line_btn->color(FL_WHITE);
	line_btn->value(entry.style);
	Fl_Box* label = new Fl_Box(X + WBUTTON / 2, Y, W - WBUTTON / 2, H);
	label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	label->label(entry.source.c_str());
	label->tooltip("Shows the source for this dataset.");
	end();
}