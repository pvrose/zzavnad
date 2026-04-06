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

	//! Display parameters for raw S11 mode.
	const dm_params_t s11_rx_params = {
		"S11 R+jX",
		"S11 Resistance and Reactance vs frequency",
		true,
		{ 1e6, 30e6, true, "Hz", zc_graph::axis_xier_t::SI_PREFIX, 30, true, 0.0F },
		{ 0.0F, 1000.0F, true, "\xCE\xA9(R)", zc_graph::axis_xier_t::SI_PREFIX, 30, true, 0.0F },
		{  -1000.0F, 1000.0F, true, "\xCE\xA9(X)", zc_graph::axis_xier_t::SI_PREFIX, 30, true },
		"S11 Resistance",
		"S11 Reactance",
		1
	};

	class s11_rx : public display {

	public:

		s11_rx(int W, int H, const char* L = nullptr) :
			display(W, H, L)
		{
			params_ = s11_rx_params;
			display_mode_ = DM_S11;
			create();
		}

		void convert_sp_point(
			const sp_point& point,
			const sp_data_entry& dataset,
			zc_graph::coord& point_l,
			zc_graph::coord& point_r) const override
		{
			std::complex<double> ONE(1.0, 0.0);
			point_l.x = point.frequency;
			std::complex<double> s11 = point.sparams.s11;
			// Get Z0 from the dataset.
			std::complex<double> z0 = dataset.z0;
			std::complex<double> z = z0 * (ONE + s11) / (ONE - s11); // Convert S11 to impedance.
			point_l.y = z.real(); // Resistance
			point_r.x = point.frequency;
			point_r.y = z.imag(); // Reactance
		}

	};
};