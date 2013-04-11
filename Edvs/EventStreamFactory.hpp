#ifndef INCLUDE_EDVS_EVENTSTREAMFACTORY_HPP
#define INCLUDE_EDVS_EVENTSTREAMFACTORY_HPP

#include "EventStream.hpp"
#include <string>

namespace Edvs
{

	enum Baudrate
	{
		B921k, B1000k, B2000k, B4000k
	};

	/** Edvs connection over serial port
	 * The device is opened on object construction and closed on object destruction.
	 * Please assure that you now how shared_ptr works!
	 * @param baudrate Edvs baudrate
	 * @param port Link to serial port, e.g. /dev/ttyUSB0
	 * @return link to device
	 */
	EventStreamHandle OpenSerialStream(Baudrate baudrate, std::string port="/dev/ttyUSB0");

	/** Edvs connection over network socket
	 * Please assure that you now how shared_ptr works!
	 * The device is opened on object construction and closed on object destruction.
	 * @param address Edvs network addresse of the form ip:port
	 * @return link to device
	 */
	EventStreamHandle OpenNetworkStream(const std::string& address);

	/** Reads edvs events from a file */
	EventStreamHandle OpenFileStream(const std::string& filename);

	/** Opens event stream based on URI
	 * Automatically decides connection type based on URI.
	 */
	EventStreamHandle OpenURI(const std::string& uri);


}

#endif
