#ifndef INCLUDE_EDVS_SERIALPORT
#define INCLUDE_EDVS_SERIALPORT

#include <iostream>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include "Device.h"

namespace Edvs
{
	enum Baudrate {
		B921k, B1000k, B2000k, B4000k
	};

	/** Actual reading and writing to the port */
	class SerialPort : public Device
	{
	public:
		struct IOException
		: public std::runtime_error {
			IOException(const std::string& msg)
			: std::runtime_error(msg) {}
		};
		/** Opens the serial port */
		SerialPort(Baudrate br, const std::string& port) {
		#ifdef VERBOSE
			std::cout << "Edvs: Openening port " << port << " with baud rate " << br << std::endl;
		#endif
			port_ = open(port.c_str(), O_RDWR /*| O_NOCTTY/ * | O_NDELAY*/);
			if (port_ == -1) {
				throw IOException("Unable to open port " + port);
			}
		//	fcntl(fd, F_SETFL, 0);
			struct termios settings;
			tcgetattr(port_, &settings);
			int rate = 0;
			switch(br) {
			case B921k: rate = B921600; break;
			case B1000k: rate = B1000000; break;
			case B2000k: rate = B2000000; break;
			case B4000k: rate = B4000000; break;
			default:
				throw IOException("Unkown baud rate");
			}
			cfsetispeed(&settings, rate); // set baud rates
			cfsetospeed(&settings, rate);
			settings.c_cflag = (settings.c_cflag & ~CSIZE) | CS8; // 8 bits
			settings.c_cflag |= CLOCAL | CREAD;
			settings.c_cflag |= CRTSCTS; // use hardware handshaking
			settings.c_iflag = IGNBRK;
			settings.c_oflag = 0;
			settings.c_lflag = 0;
			settings.c_cc[VMIN] = 1; // minimum number of characters to receive before satisfying the read.
			settings.c_cc[VTIME] = 5; // time between characters before satisfying the read.
			// write modified record of parameters to port
			tcsetattr(port_, TCSANOW, &settings);
		#ifdef VERBOSE
			std::cout << "Edvs: Device opened successfully!" << std::endl;
		#endif
		}
		/** Closes the serial port */
		~SerialPort() {
			close(port_);
		#ifdef VERBOSE
			std::cout << "Edvs: Device closed successfully!" << std::endl;
		#endif
		}
		/** Writes a command string to the serial port
		 * Warning: Command string must end with a '\n'!
		 */
		void WriteCommand(const std::string& str) {
			int n = ::write(port_, str.c_str(), str.length());
			if(n != (int)str.length()) {
				throw IOException("Could not write correct number of bytes!");
			}
		}
		/** Reads some data from the serial port */
		size_t ReadBinaryData(size_t n, char* data) {
			ssize_t actual = ::read(port_, data, n);
			if(actual < 0) {
				throw IOException("Could not read from port!");
			}
			return size_t(actual);
		}
	private:
		int port_;
	};

}

#endif
