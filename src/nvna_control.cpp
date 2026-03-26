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

// Include the S-parameter data structures.
#include "sp_data.hpp"

// Include ZZACOMMON drawing constants
#include "zc_drawing.h"
#include "zc_serial.h"
#include "zc_settings.h"
#include "zc_utils.h"

// Include FLTK headers for the widgets used in the control panel.
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Radio_Round_Button.H>

#include <string>

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
    int cy = y() + GAP;
    // Add the frequency unit radio buttons.
    Fl_Group* freq_unit_group = new Fl_Group(cx, cy, WBUTTON * 3, HBUTTON);
    rb_hz_ = new Fl_Radio_Round_Button(cx, cy, WBUTTON, HBUTTON, "Hz");
    rb_hz_->callback(cb_freq_unit, (void*)(intptr_t)1);
    rb_hz_->tooltip("Set frequency unit to Hz");
    cx += WBUTTON;
    rb_khz_ = new Fl_Radio_Round_Button(cx, cy, WBUTTON, HBUTTON, "kHz");
    rb_khz_->callback(cb_freq_unit, (void*)(intptr_t)1e3);
    rb_khz_->tooltip("Set frequency unit to kHz");
    cx += WBUTTON;
    rb_mhz_ = new Fl_Radio_Round_Button(cx, cy, WBUTTON, HBUTTON, "MHz");
    rb_mhz_->callback(cb_freq_unit, (void*)(intptr_t)1e6);
    rb_mhz_->tooltip("Set frequency unit to MHz");
    freq_unit_group->end();

    int maxx = cx + WBUTTON + GAP;

    cy += HBUTTON + GAP;   
    cx = x() + GAP + WLABEL;
    // Add the frequency input fields.
    ip_start_freq_ = new Fl_Float_Input(cx, cy, WBUTTON, HBUTTON, "Start Freq");
    ip_start_freq_->align(FL_ALIGN_LEFT);
    ip_start_freq_->callback(cb_freq_input, (void*)&start_freq_);
    ip_start_freq_->tooltip("Set the start frequency for data acquisition");

    cy += HBUTTON + GAP;
    ip_stop_freq_ = new Fl_Float_Input(cx, cy, WBUTTON, HBUTTON, "Stop Freq");
    ip_stop_freq_->align(FL_ALIGN_LEFT);
    ip_stop_freq_->callback(cb_freq_input, (void*)&stop_freq_);
    ip_stop_freq_->tooltip("Set the stop frequency for data acquisition");

    cy += HBUTTON + GAP;
    ip_step_freq_ = new Fl_Float_Input(cx, cy, WBUTTON, HBUTTON, "Step Freq");
    ip_step_freq_->align(FL_ALIGN_LEFT);
    ip_step_freq_->callback(cb_freq_input, (void*)&step_freq_);
    ip_step_freq_->tooltip("Set the frequency step for data acquisition");

    cy += HBUTTON + GAP;
    // Add the "Acquire Data" button.
    btn_acquire_ = new Fl_Button(x() + GAP, cy, WBUTTON, HBUTTON, "Acquire Data");
    btn_acquire_->callback(cb_acquire, this); 
    btn_acquire_->tooltip("Acquire data from the nanoVNA");

    maxx = std::max(maxx, btn_acquire_->x() + btn_acquire_->w() + GAP);
    
    cy += HBUTTON + GAP;

    // Add the "Connect to nanoVNA" button and dropdowns for port and speed.
    cx = x() + GAP;
    choice_nvna_port_ = new Fl_Choice(cx, cy, WBUTTON, HBUTTON, "Port");
    choice_nvna_port_->callback(cb_nvna_port, this);
    choice_nvna_port_->tooltip("Select the serial port for the nanoVNA connection");
    cx += WBUTTON;
    choice_nvna_speed_ = new Fl_Choice(cx, cy, WBUTTON, HBUTTON, "Speed");
    choice_nvna_speed_->callback(cb_nvna_speed, this);
    choice_nvna_speed_->tooltip("Select the communication speed for the nanoVNA connection");
    cx += WBUTTON;
    btn_connect_nvna_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Connect");
    btn_connect_nvna_->callback(cb_nvna_connect, this);
    btn_connect_nvna_->tooltip("Connect to the nanoVNA");

    maxx = std::max(maxx, btn_connect_nvna_->x() + btn_connect_nvna_->w() + GAP);
    cy += HBUTTON + GAP;

    // Resize the control panel to fit the widgets.
    resizable(nullptr);
    resize(x(), y(), maxx - x(), cy - y());

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
void nvna_control::save_current_settings() {
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
    if (frequency_xier_ == 1) {
        rb_hz_->value(true);
        rb_khz_->value(false);
        rb_mhz_->value(false);
    }
    else if (frequency_xier_ == 1e3) {
        rb_hz_->value(false);
        rb_khz_->value(true);
        rb_mhz_->value(false);
    }
    else if (frequency_xier_ == 1e6) {
        rb_hz_->value(false);
        rb_khz_->value(false);
        rb_mhz_->value(true);
    } else {
        // Default to MHz if an unrecognized multiplier is found.
        frequency_xier_ = 1e6;
        rb_hz_->value(false);
        rb_khz_->value(false);
        rb_mhz_->value(true);
    }
    // Update the frequency input fields with the current settings.
    ip_start_freq_->value(std::to_string(start_freq_ / frequency_xier_).c_str());
    ip_stop_freq_->value(std::to_string(stop_freq_ / frequency_xier_).c_str());
    ip_step_freq_->value(std::to_string(step_freq_ / frequency_xier_).c_str());

    // Disable the acquire button if the nanoVNA is not connected.
    if (!nvna_enabled_) {
        btn_acquire_->deactivate();
    } else {
        btn_acquire_->activate();
    }
}

// Populate the nanoVNA port and speed dropdowns with available options.
void nvna_control::populate_nvna_options() {
    zc_serial serial;
    std::set<std::string> ports = serial.available_ports(true);
    choice_nvna_port_->clear();
    for (const auto& port : ports) {
        choice_nvna_port_->add(port.c_str());
    }  
    // Add common baud rates to the speed dropdown.
    const std::vector<int> baud_rates = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    choice_nvna_speed_->clear();
    for (int baud : baud_rates) {
        choice_nvna_speed_->add(std::to_string(baud).c_str());
    }
}

// Callback function for the "Acquire Data" button.
void nvna_control::cb_acquire(Fl_Widget* widget, void* data) {
    nvna_control* control = zc::ancestor_view<nvna_control>(widget);
    if (control->nvna_enabled_) {
        // TODO: Implement data acquisition from nanoVNA.
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
    // TODO: Implement nanoVNA connection logic.
}
