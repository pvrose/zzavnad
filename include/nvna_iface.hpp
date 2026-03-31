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

//! \file nvna_iface.hpp
//! \brief Interface for the nanoVNA device.
//! This interface helps in connecting the nanoVNA to the software and acquiring data from it.

//! Currently only NanoVNA-H is supported.

//! \note The nanoVNA-H uses the same USB serial interface as the original NanoVNA, but with a different protocol for data acquisition.

//! \todo Impelemnt calibration. For now assume that the NanoVNA-H is calibrated.

// Include S-parameter data structures.
#include "sp_data.hpp"

// Include boost asio for cross-platform serial port access.
#include <boost/asio.hpp>

class nvna_iface {

public:

	//! Constructor for the nanoVNA interface. 
	//! It initialises the connection to the nanoVNA on \p port with \p baud_rate.
	nvna_iface(const std::string& port, int baud_rate);

	//! Destructor for the nanoVNA interface. It closes the connection to the nanoVNA.
	~nvna_iface();

	//! Acquire S-parameter data from the nanoVNA and store it in the provided data entry.
	//! \param data The set of data entries in which to store the acquired data.
	//! \param start The start frequency for the acquisition in Hz.
	//! \param stop The stop frequency for the acquisition in Hz.
	//! \param steps The number of frequency points to acquire.
	//! \return True if the acquisition was successful, false otherwise.
	bool acquire_data(sp_data_set* data, double start, double stop, int steps);

private:

	//! Acquire one batch of S-parameter data from the nanoVNA.
	//! \param data The set of data entries in which to store the acquired data.
	//! \param start The start frequency for the acquisition in Hz.
	//! \param step The frequency step for the acquisition in Hz.
	//! \param steps The number of frequency points to acquire.
	//! \return True if the acquisition was successful, false otherwise.
	bool acquire_data_batch(sp_data_set* data, double start, double step, int steps);

	//! Serial port for communication with the nanoVNA.
	boost::asio::serial_port* serial_port_;

};