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
#include "zc_file_viewer.h"
#include "zc_filename_input.h"
#include "zc_graph.h"
#include "zc_settings.h"
#include "zc_status.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Enumerations.H>
#include <FL/Fl.H>
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

// Constructor for the file source control panel.
source_control::file_source::file_source(int X, int Y, int W, int H, const char* L)
    : Fl_Group(X, Y, W, H, L) {
    int cx = x();
    int cy = y();

    btn_remove_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON, "@1+");
    btn_remove_->callback(cb_file_remove, this);
    btn_remove_->tooltip("Remove this data source");

    cx += HBUTTON;

    box_type_ = new Fl_Box(cx, cy, HBUTTON, HBUTTON);
	box_type_->tooltip("Shows the type of this data source");

    cx += HBUTTON;
    ip_filename_ = new zc_filename_input(cx, cy, WEDIT, HBUTTON);
    ip_filename_->callback(cb_file_input, this);
	ip_filename_->type(zc_filename_input::FILE);
    ip_filename_->tooltip("Select the file for this data source");

    // Alternate widgets when used as ACTIVE.
	box_nvna_ = new Fl_Box(cx, cy, WEDIT - HBUTTON, HBUTTON);
    box_nvna_->box(FL_DOWN_BOX);
	box_nvna_->color(FL_WHITE);
	box_nvna_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    box_nvna_->tooltip("Shows the timestamp of the Active data source from the nanoVNA");

    btn_keep_ = new Fl_Button(cx + WEDIT - HBUTTON, cy, HBUTTON, HBUTTON, "@square");
    btn_keep_->callback(cb_file_keep, this);
    btn_keep_->tooltip("Keep the current data for reference.");

    cx += ip_filename_->w();
    btn_line_l_ = new line_style_button(cx, cy, HBUTTON, HBUTTON);
    btn_line_l_->callback(cb_file_line, (void*)zc_graph::Y_LEFT);
    btn_line_l_->tooltip("Configure the line style for this data source");
    btn_line_l_->color(FL_WHITE);

    cx += HBUTTON;
    btn_line_r_ = new line_style_button(cx, cy, HBUTTON, HBUTTON);
    btn_line_r_->callback(cb_file_line, (void*)zc_graph::Y_RIGHT);
    btn_line_r_->tooltip("Configure the line style for this data source");
	btn_line_r_->color(FL_WHITE);

    cx += HBUTTON;
	btn_notes_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON, "?");
	btn_notes_->callback(cb_file_note, this);
	btn_notes_->tooltip("Edit notes for this data source");

	cx += HBUTTON;
    ckb_enable_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON);
    ckb_enable_->callback(cb_file_enable, this);
    ckb_enable_->tooltip("Enable/disable this data source");

    cx += HBUTTON;

    end();
};

// Destructor for the file source control panel.
source_control::file_source::~file_source() {
};

// Configure the widgets based on the current settings for this data source.
void source_control::file_source::configure_widgets() {
    sp_data_entry* entry = (sp_data_entry*)user_data();
    if (entry == nullptr) {
        // This is the SPARE data source. We can only use it to add a new file data source.
        btn_remove_->hide();
		box_type_->label("@+"); // Show a plus symbol to indicate this is the spare data source.
        ip_filename_->show();
        ip_filename_->button()->label("@fileopen"); // Show a file open symbol to indicate we want to select a file.
		ip_filename_->tooltip("Click to add a new file data source.");
		ip_filename_->value("Add file...");
        switch (sp_data_->get_number_ports()) {
        case 1:
            ip_filename_->pattern("S1P files\t*.s1p");
            ip_filename_->title("Select an S1P file to add as a data source");
            break;
        case 2:
            ip_filename_->pattern("S2P files\t*.s2p");
            ip_filename_->title("Select an S2P file to add as a data source");
            break;
        default:
            break;
        }
		box_nvna_->hide();
		btn_keep_->hide();
        btn_notes_->hide();
        btn_line_l_->hide();
        btn_line_r_->hide();
        ckb_enable_->hide();
    }
    else {
        switch (entry->source) {
        case SPDS_ACTIVE:
            // This is the active data source for the nanoVNA.
            // TRemove is not availabe
            btn_remove_->hide();
            box_type_->label("@search"); // Show a search symbol to indicate this data source is from the nanoVNA.
            ip_filename_->hide();
			box_nvna_->show();
            box_nvna_->copy_label(("nanoVNA " + entry->timestamp).c_str());
			btn_keep_->show();
            btn_notes_->show();
            btn_line_l_->show();
            btn_line_l_->value(entry->line_style_l);
            btn_line_r_->show();
			btn_line_r_->value(entry->line_style_r);
            ckb_enable_->show();
            ckb_enable_->value(entry->enabled);
            break;
        case SPDS_FILE:
            // This is a file data source.
            btn_remove_->show();
            box_type_->label("@filenew"); // Show a file symbol to indicate this data source is from a file.
            ip_filename_->show();
            ip_filename_->button()->label("@filesave"); // Indicates that we can save the data.
			ip_filename_->tooltip("Click to change the file for this data source.");
            ip_filename_->value(entry->filename.c_str());
            switch (sp_data_->get_number_ports()) {
            case 1:
                ip_filename_->pattern("S1P files\t*.s1p");
                ip_filename_->title("Select an S1P file to add as a data source");
                break;
            case 2:
                ip_filename_->pattern("S2P files\t*.s2p");
                ip_filename_->title("Select an S2P file to add as a data source");
                break;
            default:
                break;
            }
            box_nvna_->hide();
            btn_keep_->hide();
            btn_notes_->show();
            btn_line_l_->show();
			btn_line_l_->value(entry->line_style_l);
            btn_line_r_->show();
			btn_line_r_->value(entry->line_style_r);
            ckb_enable_->show();
            ckb_enable_->value(entry->enabled);
            break;
        case SPDS_KEPT:
            // This is a kept data source that was previously acquired from the nanoVNA.
            btn_remove_->show();
            box_type_->label("@circle"); // Show a circle symbol to indicate this data source is a kept dataset.
            ip_filename_->show();
            ip_filename_->button()->label("@filesave"); // Indicates that we can keep the data.
			ip_filename_->tooltip("Click to save the current data for this data source.");
            ip_filename_->value(entry->filename.c_str());
            switch (sp_data_->get_number_ports()) {
            case 1:
                ip_filename_->pattern("S1P files\t*.s1p");
                ip_filename_->title("Select an S1P file to add as a data source");
                break;
            case 2:
                ip_filename_->pattern("S2P files\t*.s2p");
                ip_filename_->title("Select an S2P file to add as a data source");
                break;
            default:
                break;
            }
            box_nvna_->hide();
			btn_keep_->hide();
            btn_notes_->show();
            btn_line_l_->show();
			btn_line_l_->value(entry->line_style_l);
            btn_line_r_->show();
			btn_line_r_->value(entry->line_style_r);
            ckb_enable_->show();
            ckb_enable_->value(entry->enabled);
            break;
        default:
            // This should not happen, but if it does, hide all the widgets for this data source.
            btn_remove_->hide();
            box_type_->label(""); // No symbol for unknown data source type.
            ip_filename_->hide();
			box_nvna_->hide();
			btn_keep_->hide();
            btn_notes_->hide();
            btn_line_l_->hide();
            btn_line_r_->hide();
            ckb_enable_->hide();
            break;
        }
    }
};

// Constructor for the source control panel.
source_control::source_control(int X, int Y, int W, int H, const char* L)
    : Fl_Group(X, Y, W, H, L) {
    box(FL_BORDER_BOX);
    align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    create_widgets();
    configure_widgets();
};

// Destructor for the source control panel.
source_control::~source_control() {
};

// Create the widgets for the control panel.
void source_control::create_widgets() {
    int cx = x() + WLABEL + WLABEL;
    int cy = y() + GAP;

    // Add the dropdown to select the data type for this session.
    choice_data_type_ = new Fl_Choice(cx, cy, WSMEDIT, HBUTTON, "VNA ports");
	choice_data_type_->add("1-port");
	choice_data_type_->add("2-port");
    choice_data_type_->align(FL_ALIGN_LEFT);
	choice_data_type_->callback(cb_data_type, this);
	choice_data_type_->tooltip("Select the data type for this session.\n This will affect how the data can be displayed and analyzed.");
	choice_data_type_->value(sp_data_->get_number_ports() - 1);

    cx = x() + GAP;
	cy += HBUTTON + GAP;

    // Add the spare data source control for adding new file data sources. 
	spare_source_ = new file_source(cx, cy, WCONTROL, HBUTTON);
	spare_source_->user_data(nullptr); // This is the spare data source, so we set the user data to nullptr to indicate this.

    // Add the active nanoVNA data source control.
	cy += HBUTTON;
    nanovna_source_ = new file_source(cx, cy, WCONTROL, HBUTTON);
    nanovna_source_->user_data(sp_data_->get_dataset(NANO_VNA_INDEX));

    cy += HBUTTON;

    // Add an Fl_Scroll to conatin all the remaining data source controls.
	int WSCROLL = WCONTROL + Fl::scrollbar_size();
	file_group_ = new Fl_Scroll(cx, cy, WSCROLL, HBUTTON * 8);
	file_group_->box(FL_FLAT_BOX);
    file_group_->type(Fl_Scroll::VERTICAL);

	cy += file_group_->h() + GAP;
	cx += file_group_->w() + GAP;

    resizable(nullptr);
    // adjust width to fit 
    size(cx - x(), cy - y());

    end();

};

// Configure the widgets based on the current settings.
void source_control::configure_widgets() {
	// Update the two fixed data source controls for the spare and nanoVNA data sources.
	spare_source_->configure_widgets();
	nanovna_source_->user_data(sp_data_->get_dataset(NANO_VNA_INDEX));
	nanovna_source_->configure_widgets();

	// Remove all existing file data source controls from the file group.
    for (int i = file_group_->children() - 1; i >= 0; i--) {
        file_source* child = dynamic_cast<file_source*>(file_group_->child(i));
        if (child != nullptr) {
            file_group_->remove(*child);
            delete child;
        }
	}
    int num_file_sources = sp_data_->get_dataset_count() - 1;

	// Add a file source control for each file data source in the data manager.
    int cy = 0;
    for (int i = 1; i < sp_data_->get_dataset_count(); i++) {
        sp_data_entry* entry = sp_data_->get_dataset(i);
        if (entry != nullptr && entry->source != SPDS_ACTIVE) {
            file_source* fs = new file_source(file_group_->x(), file_group_->y() + cy, WCONTROL, HBUTTON);
            fs->user_data(entry);
            fs->configure_widgets();
            file_group_->add(fs);
            cy += HBUTTON;
        }
    }
    redraw();
};

// A data source has been changed so data needs to reflect this.
void source_control::data_source_changed() {
    // Update the display to reflect the changed data source.
    if (display_control_ != nullptr) {
        display_control_->configure_widgets();
        display_control_->configure_displays();
        display_control_->update_displays();
    }
}

void source_control::file_source::cb_file_input(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = (source_control::file_source*)data;
    zc_filename_input* filename_input = zc::ancestor_view<zc_filename_input>(widget);
    if (file_source != nullptr) {
		sp_data_entry* entry = (sp_data_entry*)file_source->user_data();
		std::string save_filename = filename_input->value();
        if (save_filename != entry->filename) {
			// The filename has changed, so we need to update the data source with the new filename and save the data if this is an existing file data source.
			status_->misc_status(ST_NOTE, "Saving %s as %s", entry->filename.c_str(), save_filename.c_str());
			entry->filename = save_filename;
        }
        if (entry != nullptr) {
            switch (entry->source)
            {
            case SPDS_FILE:
                // An existing file, so update it.
                sp_data_->store_data_to_file(entry);
                break;
            case SPDS_KEPT:
                // This is a kept dataset, so we want to save the current data for this data source.
                entry->filename = ((Fl_Input*)widget)->value();
                sp_data_->store_data_to_file(entry);
                entry->source = SPDS_FILE;
            default:
                break;
            }
		}
		else {
			// This is the spare data source, so we want to add a new file data source.
            std::string filename = filename_input->value();
			int new_index = sp_data_->add_dataset(SPDS_FILE);
			entry = sp_data_->get_dataset(new_index);
            entry->filename = filename;
            sp_data_->read_data_from_file(entry);
            filename_input->value("");
        }
    }
	// Update the display to reflect the changed data source.
	source_control* control = zc::ancestor_view<source_control>(file_source);
    if (control != nullptr) {
        control->configure_widgets();
        control->data_source_changed();
	}
}

// Callback function to keep the latest dataset acquired from the nanoVNA.
void source_control::file_source::cb_file_keep(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    if (file_source != nullptr) {
        sp_data_entry* entry = (sp_data_entry*)file_source->user_data();
        if (entry != nullptr && entry->source == SPDS_ACTIVE) {
			// Get a new dataset entry for the active data source to allow new data to be acquired from the nanoVNA.
			int new_index = sp_data_->add_dataset(SPDS_KEPT);
			sp_data_entry* new_entry = sp_data_->get_dataset(new_index);
			// Copy salient data from the active dataset to the new kept dataset.
			new_entry->source = SPDS_KEPT;
			new_entry->data = entry->data;
			new_entry->valid_ports = entry->valid_ports;
			new_entry->filename = "Kept " + entry->timestamp + ".s1p";
			entry->timestamp = ""; // Clear the timestamp for the active dataset to indicate this is now ready for new data to be acquired.
            // Update the display to reflect the changed data source.
            source_control* control = zc::ancestor_view<source_control>(file_source);
			// Update the control panel to reflect the changed data source.
			control->configure_widgets();
            if (control != nullptr) {
                control->data_source_changed();
            }
        }
    }
}

// Callback function to enable/disable a file data source when the filename input or enable checkbox is changed.
void source_control::file_source::cb_file_enable(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    if (file_source != nullptr) {
		sp_data_entry* entry = (sp_data_entry*)file_source->user_data();
        // Update the enabled state based on the checkbox value.
        entry->enabled = ((Fl_Check_Button*)widget)->value();
        // Reconfigure the widgets for this data source based on the new settings.
        file_source->configure_widgets();
        // Update data
		source_control* control = zc::ancestor_view<source_control>(file_source);
		control->data_source_changed();
    }
}

// Callback function to open the line configuration dialog when the line button is clicked.
void source_control::file_source::cb_file_line(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    if (file_source != nullptr) {
		zc_line_style* line_style = nullptr;
		sp_data_entry* entry = (sp_data_entry*)file_source->user_data();
        switch ((zc_graph::y_axis_t)(intptr_t)data) {
		case zc_graph::Y_LEFT:
			line_style = &entry->line_style_l;
            break;
		case zc_graph::Y_RIGHT:
			line_style = &entry->line_style_r;
			break;
        }
		line_style_button* line_button = (line_style_button*)widget;
		*line_style = line_button->value();
        file_source->configure_widgets();
        // Update data
        source_control* control = zc::ancestor_view<source_control>(file_source);
        if (control != nullptr) {
            control->data_source_changed();
        }
    }
}

// Callback function to remove a file data source when the remove button is clicked.
void source_control::file_source::cb_file_remove(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    source_control* control = zc::ancestor_view<source_control>(widget);
    if (file_source != nullptr && control != nullptr) {
        // Remove the dataset associated with this file source from the data manager.
        sp_data_->remove_dataset((sp_data_entry*)file_source->user_data());
        // Update the control panel to reflect the removed data source.
        control->configure_widgets();
        // Updatethe display to reflect the removed data source.
        control->data_source_changed();
    }
}

// Callback function to open the notes dialog when the notes button is clicked.
void source_control::file_source::cb_file_note(Fl_Widget* widget, void* data) {
    source_control::file_source* file_source = zc::ancestor_view<source_control::file_source>(widget);
    if (file_source != nullptr) {
        sp_data_entry* entry = (sp_data_entry*)file_source->user_data();
        // Open a dialog to edit the notes for this data source.
		zc_file_viewer* note_viewer = new zc_file_viewer(400, 300, "Edit Notes");
		note_viewer->type(zc_file_viewer::VT_DATA);
        note_viewer->set_data(&entry->notes);
    }
}

// Callback function for data type selection dropdown.
void source_control::cb_data_type(Fl_Widget* widget, void* data) {
    source_control* control = zc::ancestor_view<source_control>(widget);
    if (control != nullptr) {
        int selected = ((Fl_Choice*)widget)->value();
        sp_data_->set_number_ports(selected + 1);
        control->configure_widgets();
        control->data_source_changed();
    }
}