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

#include "nvna_control.hpp"

// Include ZZACOMMON classes.
#include "zc_serial.h"
#include "zc_status.h"

// Include standard library headers.
#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>

//! Constructor for the nanoVNA interface. It initialises the connection to the nanoVNA on \p port with \p baud_rate.
//! Currently only NanoVNA-H is supported.
nvna_iface::nvna_iface(const std::string& port, int baud_rate) {
	serial_port_ = new zc_serial(port, baud_rate);
	if (serial_port_) {
		status_->misc_status(ST_NOTE, "Connected to nanoVNA on port %s at baud rate %d.", port.c_str(), baud_rate);
		write_command(""); // Send a carriage return to get the prompt.
		std::string version;
		write_command("version", version);
		status_->misc_status(ST_NOTE, "Version: %s", version.c_str());
	}
}

//! Destructor for the nanoVNA interface. It closes the connection to the nanoVNA.
nvna_iface::~nvna_iface() {
	if (serial_port_) {
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
			nvna_control_->update_progress(points_acquired / static_cast<double>(steps));
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
	uint32_t start_freq = static_cast<uint32_t>(start);
	uint32_t stop_freq = static_cast<uint32_t>(start + step * (steps - 1));
	snprintf(command, sizeof(command), "scan %d %d %d 7", start_freq, stop_freq, steps);
	std::string response;
	if (!write_command(command, response)) {
		status_->misc_status(ST_ERROR, "Failed to send command to nanoVNA.");
		return false;
	}
	// Scan the response from the nanoVNA and parse it to extract the S-parameter data.
	// The response is expected to be in the format:
	// "<freq1> <s11_real1> <s11_imag1> <s21_real1> <s21_imag1>\n"
	std::stringstream ss(response);
	while (ss.good()) {
		double freq, s11_real, s11_imag, s21_real, s21_imag;
		ss >> freq >> s11_real >> s11_imag >> s21_real >> s21_imag;
		if (ss.good()) {
			sp_point point;
			point.frequency = freq;
			point.sparams.s11 = std::complex<double>(s11_real, s11_imag);
			point.sparams.s21 = std::complex<double>(s21_real, s21_imag);
			data->insert(point);
		}
	}	
	return true;
}

//! Check if the nanoVNA interface is connected and ready for data acquisition.
bool nvna_iface::is_connected() const {
	return serial_port_ && serial_port_->is_connected();
}

// ! Write a command and extract the response.
// Return data will be <command><CR/LF><response><prompt>	
bool nvna_iface::write_command(const std::string& command, std::string& response) {
	serial_port_->write_line(command + "\r");
	// Read all the data until we get the prompt again.
	std::string buffer;
	while (true)
	{
		std::string data;
		if (serial_port_->read_any(data)) {
			buffer += data;
			size_t prompt_pos = buffer.find("ch> ");
			if (prompt_pos != std::string::npos) {
				// Check if the response starts with the command we sent. 
				// If so, remove it from the response.
				if (buffer.find(command) == 0) {
					buffer.erase(0, command.length());
					prompt_pos -= command.length(); // Adjust the prompt position if we removed the command.
				}
				// Remove any leading CR/LF from the response.
			    while (!buffer.empty() && (buffer[0] == '\r' || buffer[0] == '\n')) {
					buffer.erase(0, 1);
					prompt_pos--;
				}
				// Remove the prompt from the end of the response.
				buffer.erase(prompt_pos);
				// and return the response.
				response = buffer;
				return true;
			}
		}
		else {
			return false;
		}
	}
}
