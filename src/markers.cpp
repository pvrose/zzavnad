#include "markers.hpp"

#include "display_control.hpp"

#include "zc_drawing.h"
#include "zc_settings.h"
#include "zc_fltk.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Image_Surface.H>

extern display_control* display_control_; //!< Global pointer to the display control instance.

//! \brief Constructor.
markers::markers(int x, int y, int w, int h, const char* label) :
	Fl_Group(x, y, w, h, label)
{
	box(FL_BORDER_BOX);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	create_widgets();
	load_settings();
	configure_widgets();
}

//! \brief Destructor.
markers::~markers(){
	save_settings();
}

//! \brief Load the previous settings for the display.
void markers::load_settings(){
	zc_settings settings;
	zc_settings markers_settings(&settings, "Markers");
	// Load the settings for each marker type.
	for (marker_type type = static_cast<marker_type>(0); type < MT_COUNT; type = static_cast<marker_type>(type + 1)) {
		zc_settings marker_settings(&markers_settings, marker_type_names_.at(type).c_str());
        marker_settings.get("Enabled", marker_settings_[type].enabled, false);
		marker_settings.get("Colour", marker_settings_[type].colour, FL_BLACK);
	}
}

//! \brief Save the current settings for the display.
void markers::save_settings() {
	zc_settings settings;
	zc_settings markers_settings(&settings, "Markers");
	// Save the settings for each marker type.
	for (marker_type type = static_cast<marker_type>(0); type < MT_COUNT; type = static_cast<marker_type>(type + 1)) {
		zc_settings marker_settings(&markers_settings, marker_type_names_.at(type).c_str());
		marker_settings.set("Enabled", this->marker_settings_[type].enabled);
		marker_settings.set("Colour", this->marker_settings_[type].colour);
	}
	settings.flush();
	display_control_->configure_displays();
	display_control_->update_displays();
}

//! \brief Configure the widgets based on the current settings.
void markers::configure_widgets() {
	for (marker_type type = static_cast<marker_type>(0); type < MT_COUNT; type = static_cast<marker_type>(type + 1)) {
		if (marker_checkboxes_.find(type) != marker_checkboxes_.end()) {
			marker_checkboxes_[type]->value(marker_settings_[type].enabled);
		}
		if (marker_colour_buttons_.find(type) != marker_colour_buttons_.end()) {
			(this->*marker_colour_draw_functions_.at(type))(marker_colour_buttons_.at(type));
		}
	}
}

//! \brief Instantiate the widgets for the display control.
void markers::create_widgets() {
	int cx = x() + GAP;
	int cy = y() + HTEXT;
	// One line per marker type, with a checkbox to enable/disable the marker and a button to select the colour.
	// Add a checkbox and colour button for each marker type.
	for (marker_type type = static_cast<marker_type>(0); type < MT_COUNT; type = static_cast<marker_type>(type + 1)) {
		marker_settings_t& settings = marker_settings_[type];
		cx += WLABEL;
		Fl_Check_Button* bn_enabled = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, marker_type_names_.at(type).c_str());
		bn_enabled->align(FL_ALIGN_LEFT);
		bn_enabled->value(settings.enabled);
		bn_enabled->callback(cb_marker_enabled, (void*)type);
		cx += HBUTTON + GAP;
		Fl_Button* btn_colour = new Fl_Button(cx, cy, HBUTTON, HBUTTON);
		btn_colour->color(settings.colour);
		btn_colour->callback(cb_marker_colour, (void*)type);
		cy += HBUTTON;
		cx = x() + GAP;
		// Store the widgets for later use.
		marker_checkboxes_[type] = bn_enabled;
		marker_colour_buttons_[type] = btn_colour;
	}
	resizable(nullptr);
	cy += GAP;
	size(w(), cy - y());
	end();
}

//! \brief Return whether the marker of the given type is enabled.
bool markers::is_marker_enabled(marker_type type) const {
	auto it = marker_settings_.find(type);
	return it != marker_settings_.end() ? it->second.enabled : false;
}

//! \brief Return the base colour for the marker of the given type.
Fl_Color markers::marker_color(marker_type type) const {
	auto it = marker_settings_.find(type);
	return it != marker_settings_.end() ? it->second.colour : FL_BLACK;
}

//! \brief Callback for marker enabled/disabled.
void markers::cb_marker_enabled(Fl_Widget* widget, void* data) {
	Fl_Check_Button* cb_enabled = (Fl_Check_Button*)widget;
	markers* that = zc::ancestor_view<markers>(widget);
	marker_type type = (marker_type)(uintptr_t)data;
	that->marker_settings_[type].enabled = cb_enabled->value();
	that->save_settings();
	that->configure_widgets();
}

//! \brief Callback for marker colour changed.
void markers::cb_marker_colour(Fl_Widget* widget, void* data) {
	Fl_Button* btn_colour = (Fl_Button*)widget;
	markers* that = zc::ancestor_view<markers>(widget);
	marker_type type = (marker_type)(uintptr_t)data;
	Fl_Color current_colour = that->marker_settings_[type].colour;
	uint8_t ir, ig, ib;
	Fl::get_color(current_colour, ir, ig, ib);
	double dr = ir / 255.0;
	double dg = ig / 255.0;
	double db = ib / 255.0;
	char title[128];
	snprintf(title, sizeof(title), "Select colour for %s marker", that->marker_type_names_.at(type).c_str());
	fl_color_chooser(title, dr, dg, db);
	Fl_Color new_color = fl_rgb_color((uint8_t)(dr * 255), (uint8_t)(dg * 255), (uint8_t)(db * 255));
	if (new_color != that->marker_settings_[type].colour) {
		that->marker_settings_[type].colour = new_color;
		that->save_settings();
		that->configure_widgets();
	}
}

//! \brief Draw button for the SWR marker type.
void markers::draw_swr_marker_button(Fl_Button* button) {
	// Create the drawing surface - origin will be top-left of the widget
	Fl_Image_Surface* image_surface = new Fl_Image_Surface(button->w(), button->h());
	Fl_Surface_Device::push_current(image_surface);
	// Draw the background
	fl_color(color());
	fl_rectf(0, 0, button->w(), button->h());
	// Set the line end coordinates.
	int x1 = button->w() / 10;
	int y1 = button->h() / 2;
	int x2 = button->w() - button->w() / 10;
	int y2 = button->h() / 2;
	// Set the line colour, width and style.
	fl_color(marker_settings_.at(MT_SWR).colour);
	fl_line_style(FL_SOLID, 1);
	fl_line(x1, y1, x2, y2);
	fl_line_style(0); // reset to default line style
	//fl_font(0, 8);
	//fl_draw("SWR", button->w() / 2 - fl_width("SWR") / 2, button->h() / 2 - fl_height() / 2);
	Fl_RGB_Image* image = image_surface->image();
	Fl_Surface_Device::pop_current();
	delete image_surface;

	button->label(nullptr);
	button->image(image);
}

//! \brief Draw button for the frequency marker type.
//! 
//! The button will show the text "MHz" in the marker colour on a 
//! background that is a lighter version of the marker colour to 
//! ensure the text is visible.
void markers::draw_frequency_marker_button(Fl_Button* button) {
	// Create the drawing surface - origin will be top-left of the widget
	Fl_Image_Surface* image_surface = new Fl_Image_Surface(button->w(), button->h());
	Fl_Surface_Device::push_current(image_surface);
	// Draw the background
	Fl_Color marker_colour = marker_settings_.at(MT_FREQUENCY).colour;
	Fl_Color bg_colour = fl_color_average(FL_WHITE, marker_colour, 0.875F);
	fl_color(bg_colour);
	fl_rectf(0, 0, button->w(), button->h());
	fl_color(marker_colour);
	//fl_draw("MHz", button->w() / 2 - fl_width("MHz") / 2, button->h() / 2 - fl_height() / 2);
	Fl_RGB_Image* image = image_surface->image();
	Fl_Surface_Device::pop_current();
	delete image_surface;
	button->label(nullptr);
	button->image(image);
}

