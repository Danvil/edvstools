#ifndef INCLUDE_EDVS_DEVICE
#define INCLUDE_EDVS_DEVICE

#include <string>

namespace Edvs
{

	class Device
	{
	public:
		virtual ~Device() {}

		/** Writes a command string to the socket
		 * Warning: Command string must end with a '\n'!
		 */
		virtual void WriteCommand(const std::string& str) = 0;

		/** Reads some data from the serial port */
		virtual size_t ReadBinaryData(size_t n, char* data) = 0;
	};

}

#endif
