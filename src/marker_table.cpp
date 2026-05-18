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
#include "marker_table.hpp"

#include "display.hpp"
#include "display_control.hpp"
#include "sp_data.hpp"

#include "zc_drawing.h"

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <string>

// Constructor for the marker table window.
marker_table::marker_table(int W, int H, const char* L) :
	Fl_Double_Window(W, H, L) {
	callback(cb_close, this);
	
	scroll_ = new Fl_Scroll(0, 0, W, H);
	scroll_->box(FL_FLAT_BOX);
	scroll_->begin();

	update_tables();
	resizable(scroll_);
	end();
	hide();
}

// Destructor for the marker table window.
marker_table::~marker_table() {
}

// Callback function for when the marker table window is closed.
void marker_table::cb_close(Fl_Widget* w, void* data) {
	display_control_->clear_data_markers();
	default_callback((Fl_Window*)w, data);
}

// Update the contents of the marker table based on the current marker values and display modes.
void marker_table::update_tables() {
	scroll_->clear();
	scroll_->redraw();
	tables_.clear();
	datasets_.clear();
	Fl_Group* current_group = Fl_Group::current();
	scroll_->begin();
	// Test each dataset in turn.
	for (int i = 0; i < sp_data_->get_dataset_count(); i++) {
		sp_data_entry* dataset = sp_data_->get_dataset(i);
		if (dataset->enabled) {
			datasets_[i] = dataset;
		}
	}
	int number_tables = datasets_.size();
	if (number_tables == 0) {
		end();
		Fl_Group::current(current_group);
		return;
	}
	int cy = HTEXT;
	for (auto dataset : datasets_) {
		marker_table_inner* table = new marker_table_inner(0, cy, w(), 100);
		table->init_table(dataset.first, dataset.second);
		tables_.push_back(table);
		int table_height = table->h();
		cy += table_height + HTEXT;
	}
	scroll_->end();
	Fl_Group::current(current_group);
	// Hide the window if there are no markers
	if (!display_control_ || !display_control_->has_markers()) {
		hide();
	}
	else {
		show();
	}
}

// Constructor for the inner marker table widget.
marker_table::marker_table_inner::marker_table_inner(int X, int Y, int W, int H, const char* L) :
	Fl_Table(X, Y, W, H, L) {
	end();
}

// Initisliase the table with the appropriate number of rows and columns for the markers and datasets.
void marker_table::marker_table_inner::init_table(int dataset_index, sp_data_entry* dataset) {
	dataset_ = dataset;
	std::string label = zc::terminal(dataset_->filename);
	if (label.empty()) label = dataset_->timestamp;
	copy_label(label.c_str());

	if (!display_control_)
		return;
	// Test each display in turn.
	for (display_mode mode = (display_mode)0; mode < DM_COUNT; mode = (display_mode)(mode + 1)) {
		display* disp = display_control_->get_display(mode);
		if (disp && disp->get_params().enabled) {
			displays_.push_back(disp);
		}
	}
	// If there are no displays, just return.
	if (displays_.empty()) {
		rows(0);
		cols(0);
		return;
	}
	rows(displays_.size());
	row_header(true);
	// Get the number of columns required for the markers.
	markers_ = display_control_->get_data_markers(dataset_index);
	int num_cols = markers_.size();
	cols(num_cols);
	col_header(true);
	col_width_all(WSMEDIT);
	col_header_height(HTEXT);
	row_height_all(HBUTTON);
	row_header_width(WBUTTON);

	// Resize the table to fit the contents.
	int table_height = col_header_height() + rows() * row_height(0) + Fl::scrollbar_size();
	int table_width = row_header_width() + cols() * col_width(0) + Fl::scrollbar_size();
	resize(x(), y(), table_width, table_height);
}

// Override the draw_cell method to customize the drawing of the table cells.
void marker_table::marker_table_inner::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
	// Only draw the cell contents if we're drawing a cell.
	switch (context) {
	case CONTEXT_CELL: {
		// Fill the cell background.
		fl_color(FL_BACKGROUND_COLOR);
		fl_rectf(X, Y, W, H);
		// Draw lines between the cells.
		fl_color(FL_FOREGROUND_COLOR);
		fl_line(X, Y, X + W, Y);
		fl_line(X, Y, X, Y + H);
		display* disp = displays_[R];
		// Get the marker value for this column.
		auto it = markers_.begin();
		std::advance(it, C);
		// Get the value to display based on the display mode.
		std::string value_str = disp->format_value(*it);
		// Draw the value in the cell.
		fl_draw(value_str.c_str(), X +1, Y +1, W -2, H -2, FL_ALIGN_CENTER);
		break;
	}
	case CONTEXT_ROW_HEADER: {
		// Fill the cell background.
		fl_color(FL_BACKGROUND_COLOR);
		fl_rectf(X, Y, W, H);
		// Draw lines between the cells.
		fl_color(FL_FOREGROUND_COLOR);
		fl_line(X, Y, X + W, Y);
		fl_line(X, Y, X, Y + H);
		display* disp = displays_[R];
		std::string disp_label = disp->get_params().serial_name;
		fl_draw(disp_label.c_str(), X +1, Y +1, W -2, H -2, FL_ALIGN_RIGHT);
		break;
	}
	case CONTEXT_COL_HEADER: {
		// Fill the cell background.
		fl_color(FL_BACKGROUND_COLOR);
		fl_rectf(X, Y, W, H);
		// Draw lines between the cells.
		fl_color(FL_FOREGROUND_COLOR);
		fl_line(X, Y, X + W, Y);
		fl_line(X, Y, X, Y + H);
		// Draw the marker frequency in the column header.
		char text[64];
		double mantissa;
		double exponent;
		uint32_t si_multiplier;
		auto it = markers_.begin();
		std::advance(it, C);
		zc_graph_::normalise(it->frequency, zc_graph_::SI_PREFIX, mantissa, exponent, si_multiplier);
		char si_utf8[5] = { 0 };
		fl_utf8encode(si_multiplier, si_utf8);
		snprintf(text, sizeof(text), "%d: %.3f %sHz", C + 1, mantissa, si_utf8);
		fl_draw(text, X +1, Y +1, W -2, H -2, FL_ALIGN_CENTER);
		break;
	}
	}
}

