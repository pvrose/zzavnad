#include "markers.hpp"

#include "display_control.hpp"

#include "zc_drawing.h"
#include "zc_settings.h"
#include "zc_fltk.h"
#include "zc_utils.h"

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Image_Surface.H>

#include <string>

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
	zc_settings swr_settings(&markers_settings, "SWR");
	zc_settings frequency_settings(&markers_settings, "Frequency");
	zc_settings tdr_settings(&markers_settings, "TDR");
	// Load the SWR settings.
	swr_settings.get("Enabled", swr_marker_enabled_, false);
	swr_settings.get("Colour", swr_marker_colour_, FL_RED);
	swr_settings.get("Value", swr_marker_value_, 3.0);
	// Load the frequency settings.
	frequency_settings.get("Enabled", frequency_marker_enabled_, false);
	frequency_settings.get("Colour", frequency_marker_colour_, FL_RED);
	// Load the TDR settings.
	tdr_settings.get("Enabled", tdr_marker_enabled_, false);
	tdr_settings.get("Colour", tdr_marker_colour_, FL_RED);
	tdr_settings.get("Type", tdr_marker_type_distance_, true);
	tdr_settings.get<std::string>("Material", tdr_vf_material_, "Other");
	tdr_settings.get("Velocity Factor", tdr_vf_value_, 0.66);
}

//! \brief Save the current settings for the display.
void markers::save_settings() {
	zc_settings settings;
	zc_settings markers_settings(&settings, "Markers");
	zc_settings swr_settings(&markers_settings, "SWR");
	zc_settings frequency_settings(&markers_settings, "Frequency");
	zc_settings tdr_settings(&markers_settings, "TDR");
	// Save the SWR settings.
	swr_settings.set("Enabled", swr_marker_enabled_);
	if (swr_marker_enabled_) {
		swr_settings.set("Colour", swr_marker_colour_);
		swr_settings.set("Value", swr_marker_value_);
	}
	// Save the frequency settings.
	frequency_settings.set("Enabled", frequency_marker_enabled_);
	if (frequency_marker_enabled_) {
		frequency_settings.set("Colour", frequency_marker_colour_);
	}
	// Save the TDR settings.
	tdr_settings.set("Enabled", tdr_marker_enabled_);
	if (tdr_marker_enabled_) {
		tdr_settings.set("Colour", tdr_marker_colour_);
		tdr_settings.set("Type", tdr_marker_type_distance_);
		tdr_settings.set<std::string>("Material", tdr_vf_material_);
		tdr_settings.set("Velocity Factor", tdr_vf_value_);
	}
	settings.flush();
	display_control_->configure_displays();
	display_control_->update_displays();
}

//! \brief Configure the widgets based on the current settings.
void markers::configure_widgets() {
	char text[16];
	swr_marker_checkbox_->value(swr_marker_enabled_);
	if (swr_marker_enabled_) {
		swr_marker_colour_button_->color(swr_marker_colour_);
		draw_line_marker_button(swr_marker_colour_button_, swr_marker_colour_);
		snprintf(text, sizeof(text), "%0.2f", swr_marker_value_);
		swr_marker_value_input_->value(text);
		swr_marker_colour_button_->activate();
		swr_marker_value_input_->activate();
	}
	else {
		swr_marker_colour_button_->deactivate();
		swr_marker_value_input_->deactivate();
	}
	frequency_marker_checkbox_->value(frequency_marker_enabled_);
	if (frequency_marker_enabled_) {
		frequency_marker_colour_button_->color(frequency_marker_colour_);
		draw_block_marker_button(frequency_marker_colour_button_, frequency_marker_colour_);
		frequency_marker_colour_button_->activate();
	}
	else {
		frequency_marker_colour_button_->deactivate();
	}
	tdr_marker_checkbox_->value(tdr_marker_enabled_);
	if (tdr_marker_enabled_) {
		tdr_marker_colour_button_->color(tdr_marker_colour_);
		draw_line_marker_button(tdr_marker_colour_button_, tdr_marker_colour_);
		tdr_marker_type_select_->value(tdr_marker_type_distance_);
		if (tdr_marker_type_distance_) {
			tdr_vf_material_choice_->activate();
			if (tdr_vf_material_ == "Other") {
				tdr_vf_input_->activate();
			}
			else {
				tdr_vf_input_->deactivate();
			}
			snprintf(text, sizeof(text), "%0.2f", tdr_vf_value_);
			tdr_vf_input_->value(text);
			// Look in the choice menu to find the index of the material string.
			int index = tdr_vf_material_choice_->find_index(tdr_vf_material_.c_str());
			if (index != -1) {
				tdr_vf_material_choice_->value(index);
			}
			else {
				// If the material string is not found, select "Other" and activate the VF input.
				int other_index = tdr_vf_material_choice_->find_index("Other");
				if (other_index != -1) {
					tdr_vf_material_choice_->value(other_index);
					tdr_vf_input_->activate();
					snprintf(text, sizeof(text), "%0.2f", tdr_vf_value_);
					tdr_vf_input_->value(text);
				}
				else {
					// If "Other" is also not found, just deactivate the material choice and VF input.
					tdr_vf_material_choice_->deactivate();
					tdr_vf_input_->deactivate();
				}
			}
		}
		else {
			tdr_vf_material_choice_->deactivate();
			tdr_vf_input_->deactivate();
		}
		tdr_marker_colour_button_->activate();
		tdr_marker_type_select_->activate();
	}
	else {
		tdr_marker_colour_button_->deactivate();
		tdr_marker_type_select_->deactivate();
		tdr_vf_material_choice_->deactivate();
		tdr_vf_input_->deactivate();
	}
}

//! \brief Instantiate the widgets for the display control.
void markers::create_widgets() {
	int cx = x() + GAP + WLABEL;
	int cy = y() + HTEXT;
	// SWR marker widgets.
	swr_marker_checkbox_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "SWR");
	swr_marker_checkbox_->callback(cb_marker_enabled, (void*)&swr_marker_enabled_);
	swr_marker_checkbox_->align(FL_ALIGN_LEFT);
	swr_marker_checkbox_->tooltip("Enable/disable the SWR marker.");
	cx += HBUTTON + GAP;
	swr_marker_colour_button_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON);
	swr_marker_colour_button_->callback(cb_marker_colour, (void*)&swr_marker_colour_);
	swr_marker_colour_button_->tooltip("Select the colour of the SWR marker.");
	cx += HBUTTON + GAP;
	swr_marker_value_input_ = new Fl_Input(cx, cy, WBUTTON, HBUTTON);
	swr_marker_value_input_->callback(cb_swr_marker_value, (void*)&swr_marker_value_);
	swr_marker_value_input_->tooltip("Set the value of the SWR marker.");
	
	cx = x() + GAP + WLABEL;
	cy += HBUTTON;
	frequency_marker_checkbox_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "Frequency");
	frequency_marker_checkbox_->callback(cb_marker_enabled, (void*)&frequency_marker_enabled_);
	frequency_marker_checkbox_->align(FL_ALIGN_LEFT);
	frequency_marker_checkbox_->tooltip("Enable/disable the frequency marker.");
	cx += HBUTTON + GAP;
	frequency_marker_colour_button_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON);
	frequency_marker_colour_button_->callback(cb_marker_colour, (void*)&frequency_marker_colour_);
	frequency_marker_colour_button_->tooltip("Select the colour of the frequency marker.");

	cx = x() + GAP + WLABEL;
	cy += HBUTTON;
	tdr_marker_checkbox_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "TDR");
	tdr_marker_checkbox_->callback(cb_marker_enabled, (void*)&tdr_marker_enabled_);
	tdr_marker_checkbox_->align(FL_ALIGN_LEFT);
	tdr_marker_checkbox_->tooltip("Enable/disable the TDR marker.");
	cx += HBUTTON + GAP;
	tdr_marker_colour_button_ = new Fl_Button(cx, cy, HBUTTON, HBUTTON);
	tdr_marker_colour_button_->callback(cb_marker_colour, (void*)&tdr_marker_colour_);
	tdr_marker_colour_button_->tooltip("Select the colour of the TDR marker.");
	cx += HBUTTON + GAP;
	tdr_marker_type_select_ = new Fl_Check_Button(cx, cy, HBUTTON, HBUTTON, "Dist.");
	tdr_marker_type_select_->callback(cb_tdr_marker_type, (void*)&tdr_marker_type_distance_);
	tdr_marker_type_select_->align(FL_ALIGN_TOP);
	tdr_marker_type_select_->tooltip("Select the type (Distance/Delay) of the TDR marker.");
	cx += HBUTTON;
	tdr_vf_material_choice_ = new Fl_Choice(cx, cy, WBUTTON, HBUTTON, "Material");
	tdr_vf_material_choice_->align(FL_ALIGN_TOP);
	tdr_vf_material_choice_->callback(cb_tdr_vf_material, (void*)&tdr_vf_material_);
	for (auto& material : tdr_vf_materials_) {
		tdr_vf_material_choice_->add(zc::escape_menu(material.first).c_str());
	}
	tdr_vf_material_choice_->tooltip("Select the material for the TDR marker velocity factor.");
	cx += WBUTTON + GAP;
	tdr_vf_input_ = new Fl_Input(cx, cy, WBUTTON, HBUTTON, "VF");
	tdr_vf_input_->callback(cb_tdr_vf_value, (void*)&tdr_vf_value_);
	tdr_vf_input_->align(FL_ALIGN_TOP);
	tdr_vf_input_->tooltip("Set the velocity factor for the TDR marker (material = OTHER).");

	resizable(nullptr);
	cy += GAP;
	size(w(), cy - y());
	end();
}

//! \brief Callback for marker enabled/disabled.
void markers::cb_marker_enabled(Fl_Widget* widget, void* data) {
	Fl_Check_Button* cb_enabled = (Fl_Check_Button*)widget;
	markers* that = zc::ancestor_view<markers>(widget);
	*((bool*)data) = cb_enabled->value();
	that->save_settings();
	that->configure_widgets();
}

//! \brief Callback for marker colour changed.
void markers::cb_marker_colour(Fl_Widget* widget, void* data) {
	Fl_Button* btn_colour = (Fl_Button*)widget;
	markers* that = zc::ancestor_view<markers>(widget);
	Fl_Color* colour = (Fl_Color*)data;
	uint8_t ir, ig, ib;
	Fl::get_color(*colour, ir, ig, ib);
	double dr = ir / 255.0;
	double dg = ig / 255.0;
	double db = ib / 255.0;
	char title[128];
	snprintf(title, sizeof(title), "Select colour for marker");
	fl_color_chooser(title, dr, dg, db);
	Fl_Color new_colour = fl_rgb_color((uint8_t)(dr * 255), (uint8_t)(dg * 255), (uint8_t)(db * 255));
	if (new_colour != *colour) {
		*colour = new_colour;
		that->save_settings();
		that->configure_widgets();
	}
}

//! \brief Callback for SWR marker value.
void markers::cb_swr_marker_value(Fl_Widget* widget, void* data) {
	Fl_Input* input_value = (Fl_Input*)widget;
	markers* that = zc::ancestor_view<markers>(widget);
	double* value = (double*)data;
	try {
		*value = std::stod(input_value->value());
		that->save_settings();
		that->configure_widgets();
	}
	catch (const std::exception&) {
		// Invalid input, reset to the previous value.
		input_value->value(std::to_string(*value).c_str());
	}
	that->save_settings();
	that->configure_widgets();
}

//! \brief Callback for TDR marker type: distance or time.
void markers::cb_tdr_marker_type(Fl_Widget* widget, void* data) {
	Fl_Check_Button* cb_type = (Fl_Check_Button*)widget;
	markers* that = zc::ancestor_view<markers>(widget);
	bool* type_distance = (bool*)data;
	*type_distance = cb_type->value();
	that->save_settings();
	that->configure_widgets();
}

//! \brief Callback for TDR velocity factor material selection.
void markers::cb_tdr_vf_material(Fl_Widget* widget, void* data) {
	Fl_Choice* choice_material = (Fl_Choice*)widget;
	markers* that = zc::ancestor_view<markers>(widget);
	std::string* material = (std::string*)data;
	*material = choice_material->text(choice_material->value());
	// Set the velocity factor value to the default for the selected material if it's not "Other".
	if (*material != "Other") {
		auto it = that->tdr_vf_materials_.find(*material);
		if (it != that->tdr_vf_materials_.end()) {
			that->tdr_vf_value_ = it->second;
		}
	}
	that->save_settings();
	that->configure_widgets();
}

//! \brief Callback for TDR velocity factor value.
void markers::cb_tdr_vf_value(Fl_Widget* widget, void* data) {
	Fl_Input* input_vf = (Fl_Input*)widget;
	markers* that = zc::ancestor_view<markers>(widget);
	double* vf_value = (double*)data;
	try {
		*vf_value = std::stod(input_vf->value());
		that->save_settings();
		that->configure_widgets();
	}
	catch (const std::exception&) {
		// Invalid input, reset to the previous value.
		input_vf->value(std::to_string(*vf_value).c_str());
	}
	that->save_settings();
	that->configure_widgets();
}

//! \brief Draw button for the SWR marker type.
void markers::draw_line_marker_button(Fl_Button* button, Fl_Color& colour) {
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
	fl_color(colour);
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
void markers::draw_block_marker_button(Fl_Button* button, Fl_Color& colour) {
	// Create the drawing surface - origin will be top-left of the widget
	Fl_Image_Surface* image_surface = new Fl_Image_Surface(button->w(), button->h());
	Fl_Surface_Device::push_current(image_surface);
	// Draw the background
	Fl_Color bg_colour = fl_color_average(FL_WHITE, colour, 0.875F);
	fl_color(bg_colour);
	fl_rectf(0, 0, button->w(), button->h());
	fl_color(colour);
	//fl_draw("MHz", button->w() / 2 - fl_width("MHz") / 2, button->h() / 2 - fl_height() / 2);
	Fl_RGB_Image* image = image_surface->image();
	Fl_Surface_Device::pop_current();
	delete image_surface;
	button->label(nullptr);
	button->image(image);
}

