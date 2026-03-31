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

#include "nvna_iface.hpp"

// Include ZZACOMMON classes.
#include "zc_serial.h"
#include "zc_status.h"

// Include boost asio for cross-platform serial port access.
#include <boost/asio.hpp>

// Include standard library headers.
#include <exception>
#include <stdexcept>
#include <string>

//! Constructor for the nanoVNA interface. It initialises the connection to the nanoVNA on \p port with \p baud_rate.
//! Currently only NanoVNA-H is supported.
nvna_iface::nvna_iface(const std::string& port, int baud_rate) {
	try {
		boost::asio::io_service io;
		serial_port_ = new boost::asio::serial_port(io, port);
		serial_port_->set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
	}
	catch (boost::system::system_error& e) {
		status_->misc_status(ST_ERROR, "Failed to open serial port: %s", e.what());
	}
}

//! Destructor for the nanoVNA interface. It closes the connection to the nanoVNA.
nvna_iface::~nvna_iface() {
	if (serial_port_) {
		try {
			serial_port_->close();
		}
		catch (boost::system::system_error& e) {
			status_->misc_status(ST_ERROR, "Failed to close serial port: %s", e.what());
		}
		delete serial_port_;
	}
}

//! Acquire S-parameter data from the nanoVNA and store it in the provided data entry.
//! \param data The set of data entries in which to store the acquired data.
//! \param start The start frequency for the acquisition in Hz.
//! \param step The frequency step for the acquisition in Hz.
//! \param steps The number of frequency points to acquire.
//! \return True if the acquisition was successful, false otherwise.
bool nvna_iface::acquire_data(sp_data_set* data, double start, double step, int steps) {
	status_->progress(steps, 0, "Acquiring data from nanoVNA...", "points");
	int points_acquired = 0;
	// Acquire the data in batches of 101 points.
	int batch_size = steps > 101 ? 101 : steps;
	double current_start = start;
	while (points_acquired < steps) {
		if (acquire_data_batch(data, current_start, step, batch_size)) {
			points_acquired += batch_size;
			current_start += step * batch_size;
			status_->progress(points_acquired, 0);
		}
		else {
			status_->misc_status(ST_ERROR, "Failed to acquire data from nanoVNA.");
			status_->progress("Data acquisition failed.", 0);
			return false;
		}
		int remaining_points = steps - points_acquired;
		batch_size = remaining_points > 101 ? 101 : remaining_points;
	}
	return true;
}

// ! Acquire one batch of S-parameter data from the nanoVNA.
//! \param data The set of data entries in which to store the acquired data.
//! \param start The start frequency for the acquisition in Hz.
//! \param step The frequency step for the acquisition in Hz.
//! \param steps The number of frequency points to acquire.
//! \return True if the acquisition was successful, false otherwise.
//! This function sends the appropriate command to the nanoVNA to acquire a batch of S-parameter data
//!	and parses the response to store it in the provided data entry.
bool nvna_iface::acquire_data_batch(sp_data_set* data, double start, double step, int steps) {
	// Send the command to acquire data from the nanoVNA.
	// "scan <start_freq> <stop_freq> <points> <flags>"
	char command[100];
	snprintf(command, sizeof(command), "scan %e %e %d 7\n", start, start + step * (steps - 1), steps);
	boost::asio::write(*serial_port_, boost::asio::buffer(command, strlen(command)));
	// Read the response from the nanoVNA and parse it to extract the S-parameter data.
	// The response is expected to be in the format:
	// "<freq1> <s11_real1> <s11_imag1> <s21_real1> <s21_imag1>\n"
	// Put the reads in a loop to read all the points.
	try {
		for (int i = 0; i < steps; ++i) {
			std::string response;
			size_t len = boost::asio::read_until(*serial_port_, boost::asio::dynamic_buffer(response), '\n');
			
			double frequency, s11_real, s11_imag, s21_real, s21_imag;
			if (sscanf(response.c_str(), "%lf %lf %lf %lf %lf", &frequency, &s11_real, &s11_imag, &s21_real, &s21_imag) == 5) {
				sp_point point;
				point.frequency = frequency;
				point.sparams.s11 = std::complex<double>(s11_real, s11_imag);
				point.sparams.s21 = std::complex<double>(s21_real, s21_imag);
				data->insert(point);
			}
			else {
				status_->misc_status(ST_ERROR, "Failed to parse response from nanoVNA: %s", response.c_str());
				return false;
			}
		}
	}
	catch (boost::system::system_error& e) {
		status_->misc_status(ST_ERROR, "Failed to read from serial port: %s", e.what());
		return false;
	}
	return true;
}
