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

#include <Fl/Enumerations.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
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
	update_tables();
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
	clear();
	tables_.clear();
	datasets_.clear();
	Fl_Group* current_group = Fl_Group::current();
	begin();
	// Test each dataset in turn.
	for (int i = 0; i < sp_data_->get_dataset_count(); i++) {
		sp_data_entry* dataset = sp_data_->get_dataset(i);
		if (dataset->enabled) {
			datasets_.push_back(dataset);
		}
	}
	int number_tables = datasets_.size();
	if (number_tables == 0) {
		end();
		Fl_Group::current(current_group);
		return;
	}
	int table_height = h() / number_tables - HTEXT;
	int cy = HTEXT;
	for (int i = 0; i < number_tables; i++) {
		marker_table_inner* table = new marker_table_inner(0, cy, w(), table_height);
		table->init_table(datasets_[i]);
		tables_.push_back(table);
		cy += table_height + HTEXT;
	}
	end();
	Fl_Group::current(current_group);
	// Hide the window if there are no markers
	if (!display_control_ || display_control_->get_data_markers().empty()) {
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
void marker_table::marker_table_inner::init_table(sp_data_entry* dataset) {
	dataset_ = dataset;
	std::string label = zc::terminal(dataset->filename);
	if (label.empty()) label = dataset->timestamp;
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
	int num_cols = display_control_->get_data_markers().size();
	cols(num_cols);
	col_header(true);
	// Add the marker frequencies to the table for use in drawing the column headers and finding the closest data point to each marker.
	marker_freqs_.assign(display_control_->get_data_markers().begin(), display_control_->get_data_markers().end());
	for (double& marker_value : marker_freqs_) {
		sp_data_set* data_set = &dataset->data;
		auto closest_it = data_set->lower_bound({ marker_value, {} });
		// Get the closer between this value and the previous value if any
		if (closest_it != data_set->begin()) {
			auto prev_it = std::prev(closest_it);
			if (std::abs(prev_it->frequency - marker_value) < std::abs(closest_it->frequency - marker_value)) {
				closest_it = prev_it;
			}
		}
		marker_value = closest_it->frequency;
	}
	col_width_all(WSMEDIT);
	col_header_height(HTEXT);
	row_height_all(HBUTTON);
	row_header_width(WBUTTON);
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
		double marker_value = marker_freqs_[C];
		// Now get the value in the dataset closest to the marker frequency.
		sp_data_set* data_set = &dataset_->data;
		auto closest_it = data_set->lower_bound({ marker_value, {} });
		// Get the closer between this value and the previous value if any
		if (closest_it != data_set->begin()) {
			auto prev_it = std::prev(closest_it);
			if (std::abs(prev_it->frequency - marker_value) < std::abs(closest_it->frequency - marker_value)) {
				closest_it = prev_it;
			}
		}
		// Get the value to display based on the display mode.
		std::string value_str = disp->format_value(*closest_it);
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
		snprintf(text, sizeof(text), "%.6f MHz", marker_freqs_[C] / 1e6);
		fl_draw(text, X +1, Y +1, W -2, H -2, FL_ALIGN_CENTER);
		break;
	}
	}
}

