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

#include "source_control.hpp"

// Include ZZAVNAD data structures.
#include "display.hpp"
#include "sp_data.hpp"

// Include ZZACOMMON drawing constants
#include "zc_drawing.h"
#include "zc_filename_input.h"
#include "zc_settings.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Scroll.H>

const int WCONTROL = 5 * HBUTTON + WEDIT; //!< Width of the controls for each data source


// Constructor for the file source control panel.
source_control::file_source::file_source(int X, int Y, int W, int H, const char* L)
    : Fl_Group(X, Y, W, H, L) {
    int cx = x();
    int cy = y();

    btn_remove_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON, "X");
    btn_remove_->callback(cb_file_remove, this);
    btn_remove_->tooltip("Remove this data source");

    cx += HBUTTON;
    ip_filename_ = new zc_filename_input(cx, cy, WEDIT, HBUTTON);
    ip_filename_->callback(cb_file_input, this);
	ip_filename_->title("Select Data File S1P or S2P");
	ip_filename_->pattern("S-parameter files\t*.{s1p,s2p}");
	ip_filename_->type(zc_filename_input::FILE);
    ip_filename_->tooltip("Select the file for this data source");

    cx += ip_filename_->w();
    ckb_enable_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON);
    ckb_enable_->callback(cb_file_enable, this);
    ckb_enable_->tooltip("Enable/disable this data source");

    cx += HBUTTON;
    btn_line_l_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON);
    btn_line_l_->callback(cb_file_line, (void*)zc_graph::Y_LEFT);
    btn_line_l_->tooltip("Configure the line style for this data source");

    cx += HBUTTON;
    btn_line_r_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON);
    btn_line_r_->callback(cb_file_line, (void*)zc_graph::Y_RIGHT);
    btn_line_r_->tooltip("Configure the line style for this data source");

    cx+= HBUTTON;

    end();
};

// Destructor for the file source control panel.
source_control::file_source::~file_source() {
};

// Configure the widgets based on the current settings for this data source.
void source_control::file_source::configure_widgets() {
    if (data_entry_ == nullptr) {
        // No data entry associated with this file source, so disable all widgets.
        ip_filename_->value("");
        ip_filename_->button()->deactivate();
        ckb_enable_->value(0);
        ckb_enable_->deactivate();
        btn_line_l_->deactivate();
        btn_line_r_->deactivate();
        btn_remove_->deactivate();
        return;
    }
    switch(source_) {
    case SP_DATA_SOURCE_FILE:
        // Set the filename input widget's value to the filename for this data source.
        ip_filename_->value(data_entry_->filename.c_str());
        ckb_enable_->value(data_entry_->enabled);
        // Activate the remove button and filename input for file data sources, 
        // but deactivate the line configuration button if the data source is disabled.
        btn_remove_->activate();
        ip_filename_->button()->activate();
        if (data_entry_->enabled) {
            btn_line_l_->activate();
            btn_line_r_->activate();
        } else {
            btn_line_l_->deactivate();
            btn_line_r_->deactivate();
        }
        // Set the line button's color to the colour for this data source.
        configure_line_button(btn_line_l_, data_entry_->line_style_l);
        configure_line_button(btn_line_r_, data_entry_->line_style_r);
        break;
    case SP_DATA_SOURCE_VNA:
        ip_filename_->value("nanoVNA");
        ip_filename_->button()->deactivate();
        ckb_enable_->value(data_entry_->enabled);
        btn_remove_->deactivate();
        if (data_entry_->enabled) {
            btn_line_l_->activate();
            btn_line_r_->activate();
        } else {
            btn_line_l_->deactivate();
            btn_line_r_->deactivate();
        }
        configure_line_button(btn_line_l_, data_entry_->line_style_l);
        configure_line_button(btn_line_r_, data_entry_->line_style_l);
        break;
    default:
        break;
    }
};

// Constructor for the source control panel.
source_control::source_control(int X, int Y, int W, int H, const char* L)
    : Fl_Group(X, Y, W, H, L) {
    box(FL_BORDER_BOX);
    align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    create_widgets();
    load_default_settings();
    configure_widgets();
};

// Destructor for the source control panel.
source_control::~source_control() {
    save_current_settings();
};

// Create the widgets for the control panel.
void source_control::create_widgets() {
    int cx = x() + GAP;
    int cy = y() + HTEXT;
    int maxx = cx;
 
    // Add the "Add File" button to add new file data sources.
    btn_add_file_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Add File");
    btn_add_file_->callback(cb_file_add, this);
    btn_add_file_->tooltip("Add a new file data source");

    cx += WBUTTON;

    // Add the "Clear Files" button to remove all file data sources.
    btn_clear_files_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Clear Files");
    btn_clear_files_->callback(cb_file_clear, this);
    btn_clear_files_->tooltip("Remove all file data sources");

    cx += WBUTTON;

    // Add the "Clear Undisplayed Files" button to remove all file data sources that are currently not enabled for display.
    btn_clear_undisplayed_files_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Clear Unused");
    btn_clear_undisplayed_files_->callback(cb_file_clear_undisplayed, this);
    btn_clear_undisplayed_files_->tooltip("Remove all file data sources that are not currently enabled for display");

    cx += WBUTTON;

    maxx = std::max(maxx, cx + GAP);

    cy += HBUTTON + GAP;
    cx = x() + GAP;

    // Add the nanoVNA controls.
    nvna_source_ = new file_source(cx, cy, WCONTROL, HBUTTON);
    nvna_source_->type(SP_DATA_SOURCE_VNA);
	sp_data_entry* nvna_entry = sp_data_->get_dataset(0);
	nvna_entry->source = SP_DATA_SOURCE_VNA;
	nvna_entry->enabled = false;
	nvna_source_->set_entry(nvna_entry); // The first dataset is reserved for the nanoVNA data source.

    cy += HBUTTON;
    maxx = std::max(maxx, cx + WCONTROL + GAP);

    // Add the group to contain the file data source controls. The individual file source controls will be added to this group dynamically when the user adds file data sources.
    file_group_ = new Fl_Group(cx, cy, WCONTROL, 6 * HBUTTON);
    file_group_->box(FL_FLAT_BOX);
    // The file source controls will be added to this group dynamically 
    // when the user adds file data sources.
    file_group_->resizable(nullptr);
    file_group_->end();

    resizable(nullptr);
    // adjust width to fit 
    size(maxx - x(), h());

    end();

};

// Load the previous settings for the control panel.
void source_control::load_default_settings() {
};

// Save the current settings for the control panel.
void source_control::save_current_settings() {
};

// Configure the widgets based on the current settings.
void source_control::configure_widgets() {
    // Configure the nanoVNA line button based on the current colour and thickness settings.
    nvna_source_->configure_widgets();
    // Configure the file data source controls based on the current settings for each file data source.
    for (int i = 0; i < file_group_->children(); i++) {
        source_control::file_source* file_source = (source_control::file_source*)(file_group_->child(i));
        file_source->configure_widgets();
    }
    if (file_group_->children() >= 5) {
        btn_add_file_->deactivate();
    }
    else {
        btn_add_file_->activate();
    }
};

// Add a new file data source to the control panel with the given filename, colour, and thickness.
void source_control::add_file(sp_data_entry* entry) {
    file_group_->begin();
    // Add a new file source control to the file group.
    file_source* new_fs = new file_source(file_group_->x(), file_group_->y() + file_group_->children() * HBUTTON, WCONTROL, HBUTTON);
    new_fs->type(SP_DATA_SOURCE_FILE);
    new_fs->set_entry(entry);
    file_group_->end();
};

// Configure a linestyle button based on the given colour and thickness.
// TODO: Can we add a visual indication of the line thickness to the button? 
// Maybe by drawing a line on the button itself?
void source_control::configure_line_button(Fl_Button* button, zc_graph_line_t line_style) {
    // Set the label colour to the given colour.
    button->labelcolor(line_style.colour);
    // Set the button's label to indicate the line thickness.
    button->labelfont(FL_BOLD);
    button->copy_label(std::to_string(line_style.thickness).c_str());
};

// A data source has been changed so data needs to reflect this.
// \param source The data source that has been changed. 
// \note If \p source is nullptr, all data sources should be reloaded.
void source_control::data_source_changed(file_source* source) {
    // Update the display to reflect the changed data source.
    if (display_ != nullptr) {
        display_->configure_graph();
        display_->update_graph();
    }
}
 
// Callback functions for the widgets in the control panel.
void source_control::cb_file_add(Fl_Widget* widget, void* data) {
    source_control* control = zc::ancestor_view<source_control>(widget);
    int index = sp_data_->add_dataset();
    sp_data_entry* entry = sp_data_->get_dataset(index);
    entry->filename = "";
    entry->line_style_l = zc_graph_line_t{FL_BLUE, 2, FL_SOLID};
    entry->line_style_r = zc_graph_line_t{fl_lighter(FL_BLUE), 2, FL_SOLID};
    entry->enabled = false;
    control->add_file(entry);
    control->configure_widgets();
    control->redraw();
}

void source_control::cb_file_input(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    source_control* control = zc::ancestor_view<source_control>(file_source);
    if (file_source != nullptr) {
        std::string filename = ((Fl_Input*)widget)->value();
        file_source->data_entry_->filename = filename; 
    }
}

// Callback function to enable/disable a file data source when the filename input or enable checkbox is changed.
void source_control::cb_file_enable(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    if (file_source != nullptr) {
        // Update the enabled state based on the checkbox value.
        file_source->data_entry_->enabled = ((Fl_Check_Button*)widget)->value();
        // Reconfigure the widgets for this data source based on the new settings.
        file_source->configure_widgets();
        // Update data
        source_control* control = zc::ancestor_view<source_control>(file_source);
        if (control != nullptr) {
            control->data_source_changed(file_source);
        }   
    }
}

// Callback function to open the line configuration dialog when the line button is clicked.
void source_control::cb_file_line(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    if (file_source != nullptr) {
		zc_graph_line_t line_style;
        switch ((zc_graph::y_axis_t)(intptr_t)data) {
		case zc_graph::Y_LEFT:
			line_style = file_source->data_entry_->line_style_l;
            break;
		case zc_graph::Y_RIGHT:
			line_style = file_source->data_entry_->line_style_r;
			break;
        }
        // TODO: Implement line configuration dialog to allow the user to select the colour and thickness for this data source.
        line_style.colour = FL_RED; // Placeholder for testing
        line_style.thickness = 3; // Placeholder for testing
		line_style.style = FL_SOLID; // Placeholder for testing
        file_source->configure_widgets();
        // Update data
        source_control* control = zc::ancestor_view<source_control>(file_source);
        if (control != nullptr) {
            control->data_source_changed(file_source);
        }
    }
}

// Callback function to remove a file data source when the remove button is clicked.
void source_control::cb_file_remove(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    source_control* control = zc::ancestor_view<source_control>(widget);
    if (file_source != nullptr && control != nullptr) {
        // Remove the dataset associated with this file source from the data manager.
        sp_data_->remove_dataset(file_source->data_entry_);
        // Remove the file source control from the file group.
        control->file_group_->remove(file_source);
        // Resize the file group to account for the removed file source control.
        control->file_group_->resize(control->file_group_->x(), control->file_group_->y(), control->file_group_->w(), control->file_group_->h() - HBUTTON); 
        // Update the control panel to reflect the removed data source.
        control->configure_widgets();
    }
}

// Callback function to clear all file data sources when the "Clear Files" button is clicked.
void source_control::cb_file_clear(Fl_Widget* widget, void* data) {
    source_control* control = zc::ancestor_view<source_control>(widget);
    if (control != nullptr) {
        // Clear all file datasets from the data manager.
        sp_data_->clear_file_datasets();
        // Remove all file source controls from the file group.
        control->file_group_->clear();
        // Resize the file group to account for the removed file source controls.
        control->file_group_->resize(control->file_group_->x(), control->file_group_->y(), control->file_group_->w(), 0); 
        // Update the control panel to reflect the removed data sources.
        control->configure_widgets();
    }
}   

// Callback function to clear all undisplayed file data sources when the "Clear Unused" button is clicked.
void source_control::cb_file_clear_undisplayed(Fl_Widget* widget, void* data) {
    source_control* control = zc::ancestor_view<source_control>(widget);
    if (control != nullptr) {
        // Clear all undisplayed file datasets from the data manager.
        sp_data_->clear_undisplayed_file_datasets();
        // Remove all file source controls from the file group that are not enabled for display.
        for (int i = control->file_group_->children() - 1; i >= 0; i--) {
            source_control::file_source* file_source = (source_control::file_source*)(control->file_group_->child(i));
            if (!file_source->data_entry_->enabled) {
                control->file_group_->remove(file_source);
                // Resize the file group to account for the removed file source control.
                control->file_group_->resize(control->file_group_->x(), control->file_group_->y(), control->file_group_->w(), control->file_group_->h() - HBUTTON);
            }
        }
        // Update the control panel to reflect the removed data sources.
        control->configure_widgets();
    }
}