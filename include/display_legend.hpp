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

//! \file display_legend.hpp
//! \brief Provides a legend for use with the display windows.

//! Include zzacommon items.
#include "zc_button_dialog.h"
#include "zc_graph_axis.h"
#include "zc_line_style.h"

//! Include FLTK headers for the widgets used in the display legend.
#include <FL/Fl_Group.H>

//! Include C++ standard library headers.
#include <cstdint>
#include <string>
#include <vector>

//! Temporary class for a line-style button
typedef zc_button_dialog<zc_line_style_dialog, zc_line_style> line_style_button;

// Forward declarations
enum display_mode : uint8_t;
class Fl_Scroll;

//! \brief Data needed to display the legend for a display.
struct legend_entry_t {
	zc_graph_axis::tick_direction_t tick_direction; //!< Y-axis for this entry.
	zc_line_style style; //!< Line style to show for this entry.
	std::string source; //!< Source for this entry.
};

class display_legend : public Fl_Group {
public:
	//! Constructor for the display legend.
	//! \param X The x-coordinate of the legend.
	//! \param Y The y-coordinate of the legend.
	//! \param W The width of the legend.
	//! \param H The height of the legend.
	//! \param L The label for the legend.
	display_legend(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor for the display legend.
	~display_legend();
	//! Create the widgets for the display legend.
	void create_widgets();
	//! Set the display mode for the legend.
	void set_entries(const std::vector<legend_entry_t>& entries);

private:
	Fl_Scroll* scroll_; //!< Scroll widget to contain the legend entries.

	// Group to contain a label and line style button for each entry.
	class entry_group : public Fl_Group {
	public:
		entry_group(int X, int Y, int W, int H, const legend_entry_t& entry);
	};

	void add_entry_groups();

	std::vector<legend_entry_t> entries_;
};
