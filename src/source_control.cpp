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
#include "display_control.hpp"
#include "sp_data.hpp"

// Include ZZACOMMON drawing constants
#include "zc_drawing.h"
#include "zc_filename_input.h"
#include "zc_graph.h"
#include "zc_settings.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Widget.H>

// C++ standard library headers.
#include <array>
#include <string>

const int WCONTROL = 6 * HBUTTON + WEDIT; //!< Width of the controls for each data source

bool source_control::file_source::show_add_ = false; //!< Static variable to control whether the add button is shown for file sources.

// Constructor for the file source control panel.
source_control::file_source::file_source(int X, int Y, int W, int H, const char* L)
    : Fl_Group(X, Y, W, H, L) {
    int cx = x();
    int cy = y();

    btn_add_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON, "@+");
    btn_add_->callback(cb_file_source_add, this);
    btn_add_->tooltip("Add a source file");
    cx += HBUTTON;

    btn_remove_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON, "@1+");
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
    ckb_enable_->callback(cb_file_enable, (void*)&file_index_);
    ckb_enable_->tooltip("Enable/disable this data source");

    cx += HBUTTON;
    btn_line_l_ = new line_style_button(cx, cy, HBUTTON, HBUTTON);
    btn_line_l_->callback(cb_file_line, (void*)zc_graph::Y_LEFT);
    btn_line_l_->tooltip("Configure the line style for this data source");

    cx += HBUTTON;
    btn_line_r_ = new line_style_button(cx, cy, HBUTTON, HBUTTON);
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
        ckb_enable_->value(false);
        ckb_enable_->deactivate();
        btn_line_l_->deactivate();
        btn_line_r_->deactivate();
        btn_remove_->deactivate();
        btn_add_->show();
        if (show_add_) {
            btn_add_->activate();
            show_add_ = false;
        } else {
            btn_add_->deactivate();
		}
        return;
    }
    switch(source_) {
    case SP_DATA_SOURCE_FILE:
        // Set the filename input widget's value to the filename for this data source.
        ip_filename_->value(data_entry_->filename.c_str());
        ckb_enable_->activate();
        ckb_enable_->value(data_entry_->enabled);
        // Activate the remove button and filename input for file data sources, 
        // but deactivate the line configuration button if the data source is disabled.
        btn_remove_->activate();
        btn_remove_->show();
		btn_add_->deactivate();
		btn_add_->show();
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
        ckb_enable_->activate();
        ckb_enable_->value(data_entry_->enabled);
        btn_remove_->deactivate();
		btn_add_->deactivate();
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

    // Add the nanoVNA controls.
    nvna_source_ = new file_source(cx, cy, WCONTROL, HBUTTON);
    nvna_source_->type(SP_DATA_SOURCE_VNA);
	sp_data_entry* nvna_entry = sp_data_->get_dataset(0);
	nvna_entry->source = SP_DATA_SOURCE_VNA;
	nvna_entry->enabled = false;
	nvna_source_->set_entry(nvna_entry, -1); // The first dataset is reserved for the nanoVNA data source.

    cy += HBUTTON;

    // Add the group to contain the file data source controls. The individual file source controls will be added to this group dynamically when the user adds file data sources.
    file_group_ = new Fl_Group(cx, cy, WCONTROL, (1 + NUM_FILE_SOURCES) * HBUTTON);
    file_group_->box(FL_FLAT_BOX);
    // Add the individual file source controls to the file group.
    for (int i = 0; i < NUM_FILE_SOURCES; i++) {
        file_source* fs = new file_source(cx, cy, WCONTROL, HBUTTON);
        fs->type(SP_DATA_SOURCE_FILE);
        fs->set_entry(nullptr, -1); // No data entry associated with this file source yet.
		file_sources_[i] = fs;
        cy += HBUTTON;
    }
    file_group_->resizable(nullptr);
    file_group_->end();

	cy += GAP;
	cx += file_group_->w() + GAP;

    resizable(nullptr);
    // adjust width to fit 
    size(cx - x(), cy - y());

    end();

};

// Load the previous settings for the control panel. This also loads known files.
void source_control::load_default_settings() {
    // Load the file data for known files.
    int num_files = sp_data_->get_dataset_count();
	int file_source_ix = 0;
    for (int i = 0; i < num_files; i++) {
        sp_data_entry* entry = sp_data_->get_dataset(i);
        if (entry->source == SP_DATA_SOURCE_FILE) {
            // The file sources are currently empty. Add the known ones.
            if (file_source_ix < NUM_FILE_SOURCES) {
                file_sources_[file_source_ix]->set_entry(entry, i);
                file_source_ix++;
            }
            else {
                // No more file source controls available to add this data source to, so stop loading known files.
                break;
            }
        }
    }
};

// Save the current settings for the control panel.
void source_control::save_current_settings() {
};

// Configure the widgets based on the current settings.
void source_control::configure_widgets() {
	file_source::show_add_ = true;
    // Configure the nanoVNA line button based on the current colour and thickness settings.
    nvna_source_->configure_widgets();
    // Configure the file data source controls based on the current settings for each file data source.
    for (int i = 0; i < NUM_FILE_SOURCES; i++) {
        source_control::file_source* file_source = file_sources_[i];
        file_source->configure_widgets();
    }
};

// Configure a linestyle button based on the given colour and thickness.
// TODO: Can we add a visual indication of the line thickness to the button? 
// Maybe by drawing a line on the button itself?
void source_control::configure_line_button(line_style_button* button, zc_line_style line_style) {

    // Set the button's label to indicate the line thickness.
    button->value(line_style);
}

// A data source has been changed so data needs to reflect this.
// \param source The data source that has been changed. 
// \note If \p source is nullptr, all data sources should be reloaded.
void source_control::data_source_changed(file_source* source) {
    // Update the display to reflect the changed data source.
    if (display_control_ != nullptr) {
        display_control_->configure_displays();
        display_control_->update_displays();
    }
}

// Callback function to add a new file data source when the add button is clicked.
void source_control::cb_file_source_add(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
	source_control* control = zc::ancestor_view<source_control>(file_source);
    if (file_source != nullptr) {
        // Find the first file source control that does not have a data entry associated with it and associate it with a new file dataset.
        for (int i = 0; i < NUM_FILE_SOURCES; i++) {
            if (control->file_sources_[i]->data_entry_ == nullptr) {
				int new_ix = sp_data_->add_dataset();
                sp_data_entry* new_entry = sp_data_->get_dataset(new_ix);
				new_entry->source = SP_DATA_SOURCE_FILE;
                new_entry->enabled = false;
                control->file_sources_[i]->set_entry(new_entry, new_ix);
                break;
            }
        }
    }
	control->configure_widgets();
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
		int file_index = *(int*)data;
        if (file_index >= 0) {
            // Data is from a file source, so reload the data for this file.
            sp_data_->read_data_from_file(file_index);
            source_control* control = zc::ancestor_view<source_control>(file_source);
            if (control != nullptr) {
                control->data_source_changed(file_source);
            }
        }
        else {
			// Data has altready been reloaded for this source,
            // so just update the display to reflect the changed enabled state.
			source_control* control = zc::ancestor_view<source_control>(file_source);
            if (control != nullptr) {
                control->data_source_changed(file_source);
			}
        }
    }
}

// Callback function to open the line configuration dialog when the line button is clicked.
void source_control::cb_file_line(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    if (file_source != nullptr) {
		zc_line_style* line_style = nullptr;
        switch ((zc_graph::y_axis_t)(intptr_t)data) {
		case zc_graph::Y_LEFT:
			line_style = &file_source->data_entry_->line_style_l;
            break;
		case zc_graph::Y_RIGHT:
			line_style = &file_source->data_entry_->line_style_r;
			break;
        }
		line_style_button* line_button = (line_style_button*)widget;
		*line_style = line_button->value();
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
		// Reset the file source control to have no data entry associated with it.
		file_source->set_entry(nullptr, -1);
		// Updatethe display to reflect the removed data source.
		control->data_source_changed(file_source);
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