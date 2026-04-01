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

#include "display_control.hpp"
#include "nvna_iface.hpp"
#include "sp_data.hpp"

// Include ZZACOMMON drawing constants
#include "zc_drawing.h"
#include "zc_serial.h"
#include "zc_settings.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Group.H>
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

	scan_group->end();

    
    cy = scan_group->y() + scan_group->h();

	// Add a group box for the nanoVNA connection settings.
	const int HCONN = 3 * HBUTTON + HTEXT + GAP;
    const int WCONN = WSCAN + HBUTTON;

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

    // Resize the control panel to fit the widgets.
    resizable(nullptr);
    size(WCONN + 2 * GAP, HTEXT + HSCAN + HCONN + GAP);

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
    settings.flush();
}

// Configure the widgets based on the current scenario.
void nvna_control::configure_widgets() {
    // Set the frequency unit radio buttons based on the current multiplier.
	// Convert the multiplier to power of 1000 to set choice value.
	int unit_index = log10(frequency_xier_) / 3; // 0 for Hz, 1 for kHz, 2 for MHz, 3 for GHz
	choice_f_unit_->value(unit_index);
    // Update the frequency input fields with the current settings.
    ip_start_freq_->value(std::to_string(start_freq_ / frequency_xier_).c_str());
    ip_stop_freq_->value(std::to_string(stop_freq_ / frequency_xier_).c_str());
    ip_step_freq_->value(std::to_string(step_freq_ / frequency_xier_).c_str());

    // Disable the acquire button if the nanoVNA is not connected.
    if (!nvna_enabled_) {
        btn_acquire_->deactivate();
		btn_connect_nvna_->activate();
    } else {
        btn_acquire_->activate();
		btn_connect_nvna_->deactivate();
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

// Callback function for the "Acquire Data" button.
void nvna_control::cb_acquire(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    if (control->nvna_enabled_) {
        auto data = sp_data_->get_dataset(0); // Get the first dataset (NVNA).
        if (data) {
            control->acquire_data_from_nvna(&data->data);
			display_control_->configure_displays(); // Update the displays with the new data.
            display_control_->update_displays();
        }
    }
}

// Callback function for the frequency unit radio buttons.
void nvna_control::cb_freq_unit(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    control->frequency_xier_ = (double)(intptr_t)widget->user_data();
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