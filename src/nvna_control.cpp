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

#include "nvna_control.hpp"

#include "calib_data.hpp"
#include "display_control.hpp"
#include "nvna_iface.hpp"
#include "source_control.hpp"
#include "sp_data.hpp"

// Include ZZACOMMON drawing constants
#include "zc_drawing.h"
#include "zc_filename_input.h"
#include "zc_fltk.h"
#include "zc_serial.h"
#include "zc_settings.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Fill_Dial.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Widget.H>

#include <cmath>
#include <exception>
#include <stdexcept>
#include <string>
#include <set>
#include <vector>

// Constructor for the nanoVNA control panel.
nvna_control::nvna_control(int X, int Y, int W, int H, const char* L)
    : Fl_Group(X, Y, W, H, L) {
    box(FL_BORDER_BOX);
    align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    create_widgets();
    load_default_settings();
    populate_nvna_options();
    configure_widgets();
}

// Destructor for the nanoVNA control panel.
nvna_control::~nvna_control() {
    save_current_settings();
}

// Create the widgets for the control panel.
void nvna_control::create_widgets() {
    int cx = x() + GAP;
    int cy = y() + HTEXT;

	// Add a group box for the frequency scansettings.
    const int HSCAN = HTEXT + 5 * HBUTTON + GAP;
	const int WSCAN = WLABEL + WBUTTON + GAP;
    int WALL = WSCAN;
    Fl_Group* scan_group = new Fl_Group(cx, cy, WSCAN, HSCAN, "Scan Settings");
    scan_group->box(FL_BORDER_BOX);
    scan_group->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    // Add the frequency unit dropdown
    cx += WLABEL;
    cy += HTEXT;
    choice_f_unit_ = new Fl_Choice(cx, cy, WBUTTON, HBUTTON, "Unit");
    choice_f_unit_->align(FL_ALIGN_LEFT);
	choice_f_unit_->add("Hz");
	choice_f_unit_->add("kHz");
	choice_f_unit_->add("MHz");
	choice_f_unit_->add("GHz");
	choice_f_unit_->callback(cb_freq_unit, this);
	choice_f_unit_->tooltip("Select the frequency unit for the input fields");
    
    cy += HBUTTON;   
    // Add the frequency input fields.
    ip_start_freq_ = new Fl_Float_Input(cx, cy, WBUTTON, HBUTTON, "Start");
    ip_start_freq_->align(FL_ALIGN_LEFT);
    ip_start_freq_->callback(cb_freq_input, (void*)&start_freq_);
    ip_start_freq_->tooltip("Set the start frequency for data acquisition");

    cy += HBUTTON;
    ip_stop_freq_ = new Fl_Float_Input(cx, cy, WBUTTON, HBUTTON, "Stop");
    ip_stop_freq_->align(FL_ALIGN_LEFT);
    ip_stop_freq_->callback(cb_freq_input, (void*)&stop_freq_);
    ip_stop_freq_->tooltip("Set the stop frequency for data acquisition");

    cy += HBUTTON;
    ip_step_freq_ = new Fl_Float_Input(cx, cy, WBUTTON, HBUTTON, "Step");
    ip_step_freq_->align(FL_ALIGN_LEFT);
    ip_step_freq_->callback(cb_freq_input, (void*)&step_freq_);
    ip_step_freq_->tooltip("Set the frequency step for data acquisition");

    cy += HBUTTON;

    // Add the "Acquire Data" button.
    btn_acquire_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Acquire");
    btn_acquire_->callback(cb_acquire, this); 
    btn_acquire_->tooltip("Acquire data from the nanoVNA");

	cx = x() + GAP + GAP;
	// Progress dial for data acquisition.
	dial_prog_ = new Fl_Fill_Dial(cx, cy, HBUTTON, HBUTTON);
    dial_prog_->minimum(0.0);
    dial_prog_->maximum(1.0);
    dial_prog_->color(FL_BACKGROUND_COLOR);
    dial_prog_->angles(180, 540);
    dial_prog_->box(FL_OVAL_BOX);

    scan_group->resizable(nullptr);
	scan_group->end();

    cy = scan_group->y() + scan_group->h();

	// Add a group box for the nanoVNA connection settings.
	const int HCONN = 3 * HBUTTON + HTEXT + GAP;
    const int WCONN = WSCAN + HBUTTON;
	WALL = std::max(WALL, WCONN);

    cx = x() + GAP;

	Fl_Group* conn_group = new Fl_Group(cx, cy, WCONN, HCONN, "Connection");
	conn_group->box(FL_BORDER_BOX);
  	conn_group->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    // Add the "Connect to nanoVNA" button and dropdowns for port and speed.
    cx = x() + GAP + WLABEL;
    cy += HTEXT;
    choice_nvna_port_ = new Fl_Choice(cx, cy, WBUTTON, HBUTTON, "Port");
	choice_nvna_port_->align(FL_ALIGN_LEFT);
    choice_nvna_port_->callback(cb_nvna_port, this);
    choice_nvna_port_->tooltip("Select the serial port for the nanoVNA connection");

	// Add the rescan ports button next to the port dropdown.
    cx += WBUTTON;

	btn_rescan_ports_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON, "@reload");
	btn_rescan_ports_->callback(cb_rescan_ports, this);
	btn_rescan_ports_->tooltip("Rescan for available nanoVNA ports");

	cx = x() + GAP + WLABEL;
	cy += HBUTTON;
    choice_nvna_speed_ = new Fl_Choice(cx, cy, WBUTTON, HBUTTON, "Speed");
	choice_nvna_speed_->align(FL_ALIGN_LEFT);
    choice_nvna_speed_->callback(cb_nvna_speed, this);
    choice_nvna_speed_->tooltip("Select the communication speed for the nanoVNA connection");

    cy += HBUTTON;
    btn_connect_nvna_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Connect");
    btn_connect_nvna_->callback(cb_nvna_connect, this);
    btn_connect_nvna_->tooltip("Connect to the nanoVNA");

    cy += HBUTTON + GAP;

	// End the connection group.
    conn_group->resizable();
	conn_group->end();

    // Start the calibration group.
	const int HCALIB = 7 * HBUTTON + HTEXT + 2 * GAP;
    const int WCALIB = WEDIT + 2 * GAP;
	WALL = std::max(WALL, WCALIB);

	cx = x() + GAP;
	calib_group_ = new Fl_Group(cx, cy, WCALIB, HCALIB, "Calibration");
    calib_group_->box(FL_BORDER_BOX);
    calib_group_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    cx = calib_group_->x() + GAP;
	cy += HTEXT;
    btn_calib_ok_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "Calibration Valid");    // Resize the control panel to fit the widgets.
    btn_calib_ok_->align(FL_ALIGN_RIGHT);
    btn_calib_ok_->when(FL_WHEN_CHANGED);
	btn_calib_ok_->callback(cb_calib_status_ignore, this);
	btn_calib_ok_->tooltip("Indicates whether the calibration data is valid. This indicates either no data loaded or does not cover the selected scan frequency range.");

	cy += HBUTTON;
    btn_calib_start_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Start");
    btn_calib_start_->callback(cb_calib_start, this);
	btn_calib_start_->tooltip("Start the calibration process. Follow the prompts to connect the calibration standards to the nanoVNA and acquire the necessary data.");
 
    cx += WBUTTON;
	btn_calib_clear_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Clear");
	btn_calib_clear_->callback(cb_calib_clear, this);
    btn_calib_clear_->tooltip("Clear existing calibration data.");

	cy += HBUTTON;
    cx = calib_group_->x() + GAP;

    btn_calib_open_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "Open");
	btn_calib_open_->callback(cb_calib_status_ignore, this);
	btn_calib_open_->when(FL_WHEN_CHANGED);
    btn_calib_open_->align(FL_ALIGN_RIGHT);
	btn_calib_open_->tooltip("Indicates whether the open circuit calibration data has been acquired.");

    cx += WBUTTON;

    btn_calib_thru_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "Through");
    btn_calib_thru_->callback(cb_calib_status_ignore, this);
	btn_calib_thru_->when(FL_WHEN_CHANGED);
    btn_calib_thru_->align(FL_ALIGN_RIGHT);
	btn_calib_thru_->tooltip("Indicates whether the through calibration data has been acquired.");

    cx = calib_group_->x() + GAP;
    cy += HBUTTON;

    btn_calib_short_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "Short");
    btn_calib_short_->callback(cb_calib_status_ignore, this);
	btn_calib_short_->when(FL_WHEN_CHANGED);
    btn_calib_short_->align(FL_ALIGN_RIGHT);
	btn_calib_short_->tooltip("Indicates whether the short circuit calibration data has been acquired.");

    cx += WBUTTON;

    btn_calib_isol_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "Isolation");
    btn_calib_isol_->callback(cb_calib_status_ignore, this);
	btn_calib_isol_->when(FL_WHEN_CHANGED);
    btn_calib_isol_->align(FL_ALIGN_RIGHT);
	btn_calib_isol_->tooltip("Indicates whether the isolation calibration data has been acquired.");

    cx = calib_group_->x() + GAP;
    cy += HBUTTON;

    btn_calib_load_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "Load");
    btn_calib_load_->callback(cb_calib_status_ignore, this);
	btn_calib_load_->when(FL_WHEN_CHANGED);
    btn_calib_load_->align(FL_ALIGN_RIGHT);
	btn_calib_load_->tooltip("Indicates whether the load calibration data has been acquired.");

    cx = calib_group_->x() + GAP;
    cy += HBUTTON + GAP;

	ip_calib_directory_ = new zc_filename_input(cx, cy, WEDIT, HBUTTON, "Directory");
    ip_calib_directory_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	ip_calib_directory_->type(zc_filename_input::DIRECTORY);
	ip_calib_directory_->tooltip("Select the directory to save or load calibration data.");

	cy += HBUTTON;
	btn_calib_save_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Save");
	btn_calib_save_->callback(cb_calib_save, this);
	btn_calib_save_->tooltip("Save the current calibration data to the selected directory.");

	cx += WBUTTON;
	btn_calib_read_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Read");
	btn_calib_read_->callback(cb_calib_read, this);
	btn_calib_read_->tooltip("Read calibration data from the selected directory.");

    calib_group_->resizable(nullptr);
    calib_group_->end();

    scan_group->size(WALL, scan_group->h());
    conn_group->size(WALL, conn_group->h());
    calib_group_->size(WALL, calib_group_->h());

    resizable(nullptr);
    size(WALL + 2 * GAP, HTEXT + HSCAN + HCONN + HCALIB + GAP);

    end();

}

// Load the default frequency settings from the settings.
void nvna_control::load_default_settings() {
    // Load the frequency unit multiplier from settings.
    zc_settings settings;
    zc_settings nvna_settings(&settings, "nanoVNA Control");
    nvna_settings.get("Frequency Multiplier", frequency_xier_, 1e6); // Default to MHz
    // Load the start, stop, and step frequencies from settings.
    nvna_settings.get("Start Frequency", start_freq_, 1e6); // Default to 1 MHz
    nvna_settings.get("Stop Frequency", stop_freq_, 30e6); // Default to 30 MHz
    nvna_settings.get("Step Frequency", step_freq_, 50e3); // Default to 50 kHz
    nvna_settings.get<std::string>("Port", nvna_port_, ""); // Default to empty string (no port selected)
    nvna_settings.get("Speed", nvna_speed_, 115200); // Default to 115200 baud
}

// Save the current frequency settings to the settings.
void nvna_control::save_current_settings() const {
    zc_settings settings;
    zc_settings nvna_settings(&settings, "nanoVNA Control");
    nvna_settings.set("Frequency Multiplier", frequency_xier_);
    nvna_settings.set("Start Frequency", start_freq_);
    nvna_settings.set("Stop Frequency", stop_freq_);
    nvna_settings.set("Step Frequency", step_freq_);
    nvna_settings.set<std::string>("Port", nvna_port_);
    nvna_settings.set("Speed", nvna_speed_);
    settings.flush();
}

// Configure the widgets based on the current scenario.
void nvna_control::configure_widgets() {
    // Set the frequency unit radio buttons based on the current multiplier.
	// Convert the multiplier to power of 1000 to set choice value.
	int unit_index = log10(frequency_xier_) / 3; // 0 for Hz, 1 for kHz, 2 for MHz, 3 for GHz
	choice_f_unit_->value(unit_index);
    // Update the frequency input fields with the current settings.
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%g", start_freq_ / frequency_xier_);
    ip_start_freq_->value(buffer);
	snprintf(buffer, sizeof(buffer), "%g", stop_freq_ / frequency_xier_);
    ip_stop_freq_->value(buffer);
	snprintf(buffer, sizeof(buffer), "%g", step_freq_ / frequency_xier_);
    ip_step_freq_->value(buffer);

    // Disable the acquire button if the nanoVNA is not connected.
    if (!nvna_enabled_) {
        btn_acquire_->deactivate();
		btn_connect_nvna_->activate();
    } else {
        btn_acquire_->activate();
		btn_connect_nvna_->deactivate();
    }

	calib_data::calib_status target_status;
	if (sp_data_->get_number_ports() == 1) {
		target_status = calib_data::CALIB_VALID1; // For 1-port calibration, we only need open, short, and load.
	}
	else if (sp_data_->get_number_ports() == 2) {
		target_status = calib_data::CALIB_VALID2; // For 2-port calibration, we need open, short, load, and through.
	}
	std::string calib_directory = calib_data_->get_calibration_directory();
	ip_calib_directory_->value(calib_directory.c_str());
    // Configure the calibration widgets.
    if (!nvna_enabled_ && calib_data_->get_calibration_status() < target_status) {
		calib_group_->deactivate();
    }
    else {
        calib_group_->activate();
		// Update the calibration check buttons based on the current calibration data status.   
        btn_calib_open_->value(calib_data_->get_calibration_status() & calib_data::CALIB_OPEN);
        btn_calib_short_->value(calib_data_->get_calibration_status() & calib_data::CALIB_SHORT);
        btn_calib_load_->value(calib_data_->get_calibration_status() & calib_data::CALIB_LOAD);
        if (sp_data_->get_number_ports() == 2) {
            btn_calib_thru_->activate();
            btn_calib_thru_->value(calib_data_->get_calibration_status() & calib_data::CALIB_THROUGH);
			btn_calib_isol_->activate();
            btn_calib_isol_->value(calib_data_->get_calibration_status() & calib_data::CALIB_ISOLATION);
        }
        else {
            btn_calib_thru_->deactivate();
            btn_calib_isol_->deactivate();
        }
        btn_calib_ok_->value(calib_data_->get_calibration_status() & calib_data::CALIB_CONVERTED);
        if (calib_directory.empty()) {
            btn_calib_save_->deactivate();
            btn_calib_read_->deactivate();
		}
        else {
            btn_calib_save_->activate();
            btn_calib_read_->activate();
        }
    }
}

// Populate the nanoVNA port and speed dropdowns with available options.
void nvna_control::populate_nvna_options() {
    std::set<std::string> ports = zc_serial::available_ports(true);
    choice_nvna_port_->clear();
	// Index to set the configured port as the default selection.
	int index = 0;
	bool port_found = false;
    for (const auto& port : ports) {
        choice_nvna_port_->add(port.c_str());
        if (port == nvna_port_) {
            choice_nvna_port_->value(index);
            port_found = true;
        }
        ++index;
    }  
	// If no port was found, select the first available port.
	if (!port_found && !ports.empty()) {
		choice_nvna_port_->value(0);
		nvna_port_ = choice_nvna_port_->text(0);
	}
    // Add common baud rates to the speed dropdown.
    const std::vector<int> baud_rates = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    choice_nvna_speed_->clear();
    index = 0;
	bool speed_found = false;
    for (int baud : baud_rates) {
        choice_nvna_speed_->add(std::to_string(baud).c_str());
        if (baud == nvna_speed_) {
            choice_nvna_speed_->value(index);
            speed_found = true;
        }
        ++index;
    }
	// If no speed was found, select the first available speed.
	if (!speed_found && !baud_rates.empty()) {
		choice_nvna_speed_->value(0);
		nvna_speed_ = baud_rates[0];
	}
}

// Acquire data from the nanoVNA and store it in the provided data entry.
void nvna_control::acquire_data_from_nvna(sp_data_set* data) {
    if (nvna_enabled_ && nvna_interface_ != nullptr) {
        data-> clear(); // Clear any existing data in the dataset before acquiring new data.
        // Acquire data from start frequency to stop frequency with the specified step and store it in the dataset.
        // Number of steps = (stop_freq_ - start_freq_) / step_freq_ + 1
        int num_steps = static_cast<int>((stop_freq_ - start_freq_) / step_freq_) + 1;
        nvna_interface_->acquire_data(data, start_freq_, step_freq_, num_steps);
    }
}

// Update acquisition progress on the progress dial.
void nvna_control::update_progress(double progress) {
    if (progress < 0.0) progress = 0.0;
    if (progress > 1.0) progress = 1.0;
    dial_prog_->value(progress);
    dial_prog_->redraw();
	Fl::check(); // Process FLTK events to update the UI.
}

// Run calibration process with the nanoVNA.
void nvna_control::run_calibration() {
    calib_data::calib_status target_status;
    if (sp_data_->get_number_ports() == 1) {
        target_status = calib_data::CALIB_VALID1; // For 1-port calibration, we only need open, short, and load.
    }
    else if (sp_data_->get_number_ports() == 2) {
        target_status = calib_data::CALIB_VALID2; // For 2-port calibration, we need open, short, load, and through.
    }
    bool user_cancelled = false;
    // Keep fetching calibration data until we have all the required data or the user cancels.
    while ((calib_data_->get_calibration_status() & target_status) != target_status) {
        if (!nvna_enabled_) {
            fl_alert("nanoVNA is not connected. Please connect to the nanoVNA to continue calibration.");
            return;
        }
        if ((calib_data_->get_calibration_status() & calib_data::CALIB_OPEN) == 0) {
            if (fl_choice("Please connect the OPEN calibration standard to the nanoVNA and click OK to acquire the open circuit calibration data.", "Cancel", "OK", nullptr) == 0) {
                user_cancelled = true;
                break;
            }
			int entry_index = sp_data_->add_dataset(SPDS_CALIB_O);
			sp_data_entry* open_entry = sp_data_->get_dataset(entry_index);
            acquire_data_from_nvna(&open_entry->data);
            calib_data_->set_calibration_data(SPDS_CALIB_O, open_entry);
			configure_widgets(); // Update the calibration status check buttons after acquiring data.
        }
        else if ((calib_data_->get_calibration_status() & calib_data::CALIB_SHORT) == 0) {
            if (fl_choice("Please connect the SHORT calibration standard to the nanoVNA and click OK to acquire the short circuit calibration data.", "Cancel", "OK", nullptr) == 0) {
                user_cancelled = true;
                break;
            }
            int entry_index = sp_data_->add_dataset(SPDS_CALIB_S);
            sp_data_entry* short_entry = sp_data_->get_dataset(entry_index);
            acquire_data_from_nvna(&short_entry->data);
            calib_data_->set_calibration_data(SPDS_CALIB_S, short_entry);
			configure_widgets(); // Update the calibration status check buttons after acquiring data.   
        }
        else if ((calib_data_->get_calibration_status() & calib_data::CALIB_LOAD) == 0) {
            if (fl_choice("Please connect the LOAD calibration standard to the nanoVNA and click OK to acquire the load calibration data.", "Cancel", "OK", nullptr) == 0) {
                user_cancelled = true;
                break;
            }
            int entry_index = sp_data_->add_dataset(SPDS_CALIB_L);
            sp_data_entry* load_entry = sp_data_->get_dataset(entry_index);
            acquire_data_from_nvna(&load_entry->data);
            calib_data_->set_calibration_data(SPDS_CALIB_L, load_entry);
			configure_widgets(); // Update the calibration status check buttons after acquiring data.
        }
        else if ((calib_data_->get_calibration_status() & calib_data::CALIB_THROUGH) == 0) {
            if (fl_choice("Please connect port 1 to port 2 for through calibration) and click OK to acquire the through calibration data.", "Cancel", "OK", nullptr) == 0) {
                user_cancelled = true;
                break;
            }
            int entry_index = sp_data_->add_dataset(SPDS_CALIB_T);
            sp_data_entry* thru_entry = sp_data_->get_dataset(entry_index);
            acquire_data_from_nvna(&thru_entry->data);
            calib_data_->set_calibration_data(SPDS_CALIB_T, thru_entry);
			configure_widgets(); // Update the calibration status check buttons after acquiring data.
        }

        else if ((calib_data_->get_calibration_status() & calib_data::CALIB_ISOLATION) == 0) {
            if (fl_choice("Please disconnect port 2 (or connect a load to it) and click OK to acquire the isolation calibration data.", "Cancel", "OK", nullptr) == 0) {
                user_cancelled = true;
                break;
            }
            int entry_index = sp_data_->add_dataset(SPDS_CALIB_I);
            sp_data_entry* isol_entry = sp_data_->get_dataset(entry_index);
            acquire_data_from_nvna(&isol_entry->data);
            calib_data_->set_calibration_data(SPDS_CALIB_I, isol_entry);
			configure_widgets(); // Update the calibration status check buttons after acquiring data.
        }
    }
}

// Callback function for the "Acquire Data" button.
void nvna_control::cb_acquire(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    if (control->nvna_enabled_) {
        auto data = sp_data_->get_dataset(0); // Get the first dataset (NVNA).
        if (data) {
            control->acquire_data_from_nvna(&data->data);
			data->timestamp = zc::now(true, "%Y-%m-%d-%H-%M-%S"); // Set the timestamp to the current time.
			display_control_->configure_displays(); // Update the displays with the new data.
            display_control_->update_displays();
			source_control_->configure_widgets(); // Update the source control to reflect the new data.
        }
    }
}

// Callback function for the frequency unit radio buttons.
void nvna_control::cb_freq_unit(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
	int unit_index = ((Fl_Choice*)widget)->value();
	control->frequency_xier_ = pow(1000, unit_index); // Set the multiplier based on the selected unit.
    control->configure_widgets();
}

// Callback function that copies frequency input to variables when the input fields are changed.
void nvna_control::cb_freq_input(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    double* freq_ptr = (double*)data;
    const char* input_value = ((Fl_Float_Input*)widget)->value();
    try {
        double freq = std::stod(input_value) * control->frequency_xier_;
        if (freq < 0) {
            throw std::invalid_argument("Frequency cannot be negative");
        }
        *freq_ptr = freq;
    } catch (const std::exception& e) {
        // If the input is invalid, reset the field to the current value.
        control->configure_widgets();
    }
}

// Callback function for the nanoVNA port selection dropdown.
void nvna_control::cb_nvna_port(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    control->nvna_port_ = ((Fl_Choice*)widget)->text();
}

// Callback function for the nanoVNA speed selection dropdown.
void nvna_control::cb_nvna_speed(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    control->nvna_speed_ = std::stoi(((Fl_Choice*)widget)->text());
}

// Callback function for the nanoVNA connect button.
void nvna_control::cb_nvna_connect(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
	control->nvna_interface_ = new nvna_iface(control->nvna_port_, control->nvna_speed_);
    if (control->nvna_interface_->is_connected()) {
        control->enable_nvna();
    } else {
        control->disable_nvna();
	}
	control->configure_widgets();
}

// Callback function for the nanoVNA rescan ports button.
void nvna_control::cb_rescan_ports(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    control->populate_nvna_options();
	control->configure_widgets();
}

// Callback function for the calibration start button.
void nvna_control::cb_calib_start(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    control->run_calibration();
    control->configure_widgets();
}

// Callback function for the calibration clear button.
void nvna_control::cb_calib_clear(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    calib_data_->clear_calibration_data();
    control->configure_widgets();
}

// Callback function for the calibration save button.
void nvna_control::cb_calib_save(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    std::string directory = control->ip_calib_directory_->value();
    if (!directory.empty()) {
        calib_data_->save_calibration_data(directory);
    }
}

// Callback function for the calibration read button.
void nvna_control::cb_calib_read(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    std::string directory = control->ip_calib_directory_->value();
    if (!directory.empty()) {
        calib_data_->load_calibration_data(directory);
        control->configure_widgets();
    }
}

// Callback function for the calibration status check buttons (open, short, load, through, isolation).
void nvna_control::cb_calib_status_ignore(Fl_Widget* widget, void* data) {
	// This callback is used for the calibration status check buttons to ignore user clicks on them.
	// The status of these buttons is determined by the calibration data and should not be changed by user interaction.
	nvna_control* control = zc::ancestor_view<nvna_control>(widget);
	control->configure_widgets(); // Reset the button states to reflect the actual calibration status.
}