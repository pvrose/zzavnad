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
#pragma once

#include "display.hpp"
#include "sp_data.hpp"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Widget.H>

#include <vector>

//! \brief Mrker table class for displaying marker information in a tabular format.
//! 
//! This class shows for each marker in each dataset displayed in the graphs,
//! a representation of the marker value in a format appropriate to the display mode.
//!
//! There will be one table per dataset.
//! 
//! Each table will have a column for each marker, and a row for each display mode.
//! 
class marker_table : public Fl_Double_Window {

	class marker_table_inner : public Fl_Table {

	public:
		//! Constructor for the inner marker table widget.
		marker_table_inner(int X, int Y, int W, int H, const char* L = nullptr);

		//! Override the draw_cell method to customize the drawing of the table cells.
		void draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) override;

		//! Destructor for the inner marker table widget.
		~marker_table_inner() {
		};

		//! Initialise the table with the appropriate number of rows and columns for the markers and datasets.
		void init_table(sp_data_entry* dataset);

	private:
		//! The displays that being reported on.
		std::vector<display*> displays_ = {};

		//! The marker frequencies adjusted for the dataset.
		std::vector<double> marker_freqs_ = {};

		sp_data_entry* dataset_ = nullptr; //!< The dataset that this table is reporting on.

    };

public:
	//! Constructor for the marker table window.
	marker_table(int W, int H, const char* L = nullptr);

	//! Destructor for the marker table window.
	~marker_table();

	//! Callback function for when the marker table window is closed.
	static void cb_close(Fl_Widget* w, void* data);

	//! Update the contents of the marker table based on the current marker values and display modes.
	void update_tables();

	//! Pointer to the inner marker table widget.
	std::vector<marker_table_inner*> tables_ = {};

	//! The datasets that are being reported on.
	std::vector<sp_data_entry*> datasets_ = {};

};

extern marker_table* marker_table_; //!< Global pointer to the marker table window instance.

