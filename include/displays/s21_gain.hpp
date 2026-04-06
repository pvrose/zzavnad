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

//! Include the base class header file
#include "display.hpp"
#include "display_control.hpp"
#include "sp_data.hpp"	

#include "zc_graph.h"
#include "zc_utils.h"

#include <cmath>
#include <complex>

namespace display_modes {

	//! Display parameters for raw S11 mode.
	const dm_params_t s21_gain_params = {
	    "S21 Gain",
		"S21 Gain (dB magnitude and phase) vs frequency",
		true,
		{ 1e6, 30e6, true, "Hz", zc_graph::axis_xier_t::SI_PREFIX, 30, true, 0.0F },
		{ -60.0F, 10.0F, true, "dB", zc_graph::axis_xier_t::NONE, 30, true },
		{ -180.0F, 180.0F, true, "degree", zc_graph::axis_xier_t::NONE, 60, true, -180.0F, 180.0F },
		"S21 Gain (dB)",
		"S21 Phase",
		2
	};

	class s21_gain : public display {

	public:

		s21_gain(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
			params_ = s21_gain_params;
			display_mode_ = DM_S21_GAIN;
			create();
		}

		void convert_sp_point(
			const sp_point& point,
			const sp_data_entry& dataset,
			zc_graph::coord& point_l,
			zc_graph::coord& point_r) const override
		{
			point_l.x = point.frequency;
			std::complex<double> s21 = point.sparams.s21;
			point_l.y = 20 * std::log10(std::abs(s21)); // S21 gain in dB
			point_r.x = point.frequency;
			point_r.y = std::arg(s21) * zc::RADIAN_DEGREE; // S21 phase in degrees
		}

	};
};