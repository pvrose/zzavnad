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

#include <complex>

namespace display_modes {

	//! Display parameters for SWR mode.
	dm_params_t s11_swr_params = {
		"SWR",
		"SWR vs frequency",
		false,
		{ 1e6, 30e6, true, "Hz", zc_graph::axis_xier_t::SI_PREFIX, 30, true, 0.0F },
		{ 1.0F, 15.0F, false, "SWR", zc_graph::axis_xier_t::NONE, 30, true, 1.0F },
		{ 1.0F, 15.0F, false, "SWR", zc_graph::axis_xier_t::NONE, 30, true, 1.0F },
		"Standing Wave Ratio",
		"",
		1
	};

	class s11_swr : public display {

	public:

		s11_swr(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
			params_ = s11_swr_params;
			display_mode_ = DM_SWR;
			create();
		}

		void convert_sp_point(
			const sp_point& point,
			const sp_data_entry& dataset,
			zc_graph::coord& point_l,
			zc_graph::coord& point_r) const override
		{
			point_l.x = point.frequency;
			double s11_mag = std::abs(point.sparams.s11);
			point_l.y = (1 + s11_mag) / (1 - s11_mag); // SWR calculation from S11 magnitude.
		}

	};

};