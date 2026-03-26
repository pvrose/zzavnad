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

#include <FL/Fl_Group.H>

#include <string>

// Forward declaration of FLTK widgets.
class Fl_Button;
class Fl_Choice;
class Fl_Float_Input;
class Fl_Radio_Round_Button;

//! \file nvna_control.hpp
//! \brief Provides the nanoVNA control interface for acquiring S-parameter data from a nanoVNA.

//! This class provides the control panel for interacting with a nanoVNA device.
class nvna_control : public Fl_Group {
public:
    //! Constructor for the nanoVNA control panel.
    //! \param X The x-coordinate of the control panel.
    //! \param Y The y-coordinate of the control panel.
    //! \param W The width of the control panel.
    //! \param H The height of the control panel.
    //! \param L The label for the control panel.
    nvna_control(int X, int Y, int W, int H, const char* L = nullptr);

    //! Destructor for the nanoVNA control panel.
    ~nvna_control();

    //! Enable the nanoVNA for data acquisition.
    void enable_nvna() { nvna_enabled_ = true; }

    //! Disable the nanoVNA for data acquisition.
    void disable_nvna() { nvna_enabled_ = false; }

    protected:
    //! Callback function for the "Acquire Data" button.
    static void cb_acquire(Fl_Widget* widget, void* data);
    //! Callback function for the frequency unit radio buttons.
    static void cb_freq_unit(Fl_Widget* widget, void* data);
    //! Callback function that copies frequency input to variables when the input fields are changed.
    static void cb_freq_input(Fl_Widget* widget, void* data);
    //! Callback function for the nanoVNA port selection dropdown.
    static void cb_nvna_port(Fl_Widget* widget, void* data);
    //! Callback function for the nanoVNA speed selection dropdown.
    static void cb_nvna_speed(Fl_Widget* widget, void* data);
    //! Callback function for the nanoVNA connect button.
    static void cb_nvna_connect(Fl_Widget* widget, void* data);

    private:

    //! Create the widgets for the control panel.
    void create_widgets();
    //! Load the default frequency settings into the input fields.
    void load_default_settings();
    //! Save the current frequency settings.
    void save_current_settings();
    //! Configure the widgets based on the current scenario.
    void configure_widgets();

    //! Populate the nanoVNA port and speed dropdowns with available options.
    void populate_nvna_options();
     
    Fl_Float_Input* ip_start_freq_; //!< Input for the start frequency.
    Fl_Float_Input* ip_stop_freq_;  //!< Input for the stop frequency.
    Fl_Float_Input* ip_step_freq_;  //!< Input for the frequency step.

    // Radio buttons for frequency units.
    Fl_Radio_Round_Button* rb_hz_;   //!< Radio button for Hz.
    Fl_Radio_Round_Button* rb_khz_;  //!< Radio button for kHz.
    Fl_Radio_Round_Button* rb_mhz_;  //!< Radio button for MHz.

    Fl_Button* btn_acquire_; //!< Button to acquire data from the nanoVNA.

    Fl_Choice* choice_nvna_port_; //!< Dropdown to select the nanoVNA serial port
    Fl_Choice* choice_nvna_speed_; //!< Dropdown to select the nanoVNA baud rate

    Fl_Button* btn_connect_nvna_; //!< Button to connect to the nanoVNA

    //! Frequency unit multiplier based on the selected radio button.
    double frequency_xier_;

    double start_freq_; //!< Start frequency in Hz.
    double stop_freq_;  //!< Stop frequency in Hz.
    double step_freq_;  //!< Frequency step in Hz.

    //! nanoVNA enabled flag to track whether the nanoVNA is currently enabled for data acquisition.
    //! \todo: Disable until code for acquiring data from nanoVNA is implemented.
    bool nvna_enabled_ = false;

    std::string nvna_port_; //!< Currently selected nanoVNA serial port
    int nvna_speed_; //!< Currently selected nanoVNA baud rate

};

extern nvna_control* nvna_control_; //!< Global pointer to the nanoVNA control panel instance, used for callbacks and interactions with other parts of the application.