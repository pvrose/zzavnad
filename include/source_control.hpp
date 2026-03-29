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

//! \file source_control.hpp
//! \brief Provides the main control panel for selecting data to view.
//! It allows the user to select whether to acquire data from a nanoVNA or
//! to load data from a number of files.
//! It will also allow the user to specify which datasets to display and
//! allows the user to identify the data using colour and line thickness.

#pragma once

#include "zc_graph.h"

#include "zc_line_style.h"
#include "zc_button_dialog.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

// C++ standard library includes
#include <array>
#include <cstdint>
#include <string>
#include <vector>

// Forward declaration of FLTK widgets.
class Fl_Button;
class Fl_Check_Button;
class Fl_Choice;

// Forward declaration of zzacommon widgets
class zc_filename_input;

// Line-style button
typedef zc_button_dialog<zc_line_style_dialog, zc_line_style> line_style_button;

// Forward declaration of the S-parameter data structures.
struct sp_data_entry; 
enum sp_data_source : uint8_t;

const int NUM_FILE_SOURCES = 5; //!< Maximum number of file data sources that can be added to the control panel.

//! \brief The main control panel for selecting data to view.
class source_control : public Fl_Group {
public:
    //! Constructor for the source control panel.
    //! \param X The x-coordinate of the control panel.
    //! \param Y The y-coordinate of the control panel.
    //! \param W The width of the control panel.
    //! \param H The height of the control panel.
    //! \param L The label for the control panel.
    source_control(int X, int Y, int W, int H, const char* L = nullptr);

    //! Destructor for the source control panel.
    ~source_control();

    protected:
    // Callback functions for the widgets in the control panel.
	static void cb_file_source_add(Fl_Widget* widget, void* data);
    static void cb_file_input(Fl_Widget* widget, void* data);
    static void cb_file_enable(Fl_Widget* widget, void* data);
    static void cb_file_line(Fl_Widget* widget, void* data);
    static void cb_file_remove(Fl_Widget* widget, void* data);
    static void cb_file_clear(Fl_Widget* widget, void* data);
    static void cb_file_clear_undisplayed(Fl_Widget* widget, void* data);

    private:
    //! Create the widgets for the control panel.
    void create_widgets();
    //! Load the previous settings for the control panel.
    void load_default_settings();
    //! Save the current settings for the control panel.
    void save_current_settings();
    //! Configure the widgets based on the current settings.
    void configure_widgets();

    //! Configure a linestyle button based on the given colour and thickness.
    static void configure_line_button(line_style_button* button, zc_line_style line_style);

    // Widgets for the control panel.
    //! Group to select and configure the file data sources.
    class file_source : public Fl_Group {
    public:
        file_source(int X, int Y, int W, int H, const char* L = nullptr);
        ~file_source();

        void configure_widgets();

        void type(sp_data_source source) { source_ = source; }

        sp_data_source type() const { return source_; }

        void set_entry(sp_data_entry* entry, int index) { 
            data_entry_ = entry; 
            file_index_ = index;
            configure_widgets(); 
        }

        zc_filename_input* ip_filename_; //!< Widget to input the filename for this data source
        line_style_button* btn_line_l_; //!< Button to open line configuration dialog for left axis data.
        line_style_button* btn_line_r_; //!< Button to open line configuration dialog for right axis data.
        Fl_Check_Button* ckb_enable_; //!< Checkbox to enable/disable this data source
        Fl_Button* btn_remove_; //!< Button to remove this data source from the control panel
		Fl_Button* btn_add_; //!< Button to add a new file data source to the control panel    
        sp_data_entry* data_entry_ = nullptr; //!< Pointer to the data entry associated with this file source.
 
        static bool show_add_;  //!< Indicates whether to show the add button. 

    private:
        sp_data_source source_ = static_cast<sp_data_source>(0); 
             //!< The source type for this data source (e.g. file or nanoVNA)

        int file_index_ = -1; //!< The index into the list of file sources.

    };

    //! A data source has been changed so data needs to reflect this.
    //! \param source The data source that has been changed. 
    //! \note If \p source is nullptr, all data sources should be reloaded.
    void data_source_changed(file_source* source);

    file_source* nvna_source_; //!< Control for the nanoVNA data source

    Fl_Group* file_group_; //!< Group to contain the file data source controls
	std::array<file_source*, NUM_FILE_SOURCES> file_sources_; //!< Array to hold the file data source controls

};

extern source_control* source_control_; //!< Global pointer to the source control instance used by the application.

    