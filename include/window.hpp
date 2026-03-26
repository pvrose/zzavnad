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

#include <FL/Fl_Double_Window.H>

// Forwrad declaration of main groups.
class display;
class nvna_control;
class source_control;
// Forward declaration of FLTK widgets.

//! \brief The main application window.
class main_window : public Fl_Double_Window {
public:
    //! Constructor for the main application window.
    //! \param W The width of the window.
    //! \param H The height of the window.
    //! \param L The label for the window.
    main_window(int W, int H, const char* L = nullptr);

    //! Destructor for the main application window.
    ~main_window();

    private:
    //! Create the widgets for the main window.
    void create_widgets();

    // Widgets for the main window.
    display* display_; //!< The main display for plotting the S-parameter data.
    source_control* source_control_; //!< The control panel for selecting data sources and configuring their display settings.
    nvna_control* nvna_control_; //!< The control panel for configuring the nanoVNA data acquisition settings.

};
