
#include <Edvs/edvs_impl.h>
#include <thread>
#include <iostream>
#include <signal.h>

void read_loop(int sp, bool* is_running) {
	std::cout << "Read loop starting..." << std::endl;
	const unsigned int N = 1024;
	unsigned char buffer[N];
	while(*is_running) {
		ssize_t n = edvs_serial_read(sp, buffer, N);
		if(n > 0 && is_running) {
			// std::cout << "Received " << n << " bytes:" << std::endl;
			// std::cout << "Raw: ";
			for(int i=0; i<n; i++) {
				std::cout << buffer[i];
			}
			std::cout << std::endl;
			// std::cout << "Int: ";
			// for(int i=0; i<n; i++) {
			// 	std::cout << static_cast<int>(buffer[i]) << " ";
			// }
			// std::cout << std::endl;
		}
	}
	std::cout << "Read loop finished." << std::endl;
}

int main(int argc, char** argv)
{

	int sp = edvs_serial_open("/dev/ttyUSB0", 4000000);
	if(sp == -1) {
		std::cout << "ERROR edvs_serial_open" << std::endl;
		return 0;
	}

    std::cout << "Running..." << std::endl;

	bool* is_running = new bool;
	*is_running = true;

	std::thread rec(&read_loop, sp, is_running);

	while(true) {
		// user quit
		std::string str;
		std::cin >> str;
		if(str.length() == 1 && str[0] == 'q') {
			break;
		}
		// send
		if(str.length() > 0) {
			str += "\n";
			edvs_serial_write(sp, str.data(), str.length());
			std::cout << "Sent '" << str << "'" << std::endl;
		}
	}

	// sending '??' to get out of read...
	*is_running = false;
	edvs_serial_write(sp, "??\n", 3);
	rec.join();

	if(edvs_serial_close(sp) != 0) {
		std::cout << "ERROR edvs_serial_close" << std::endl;
		return 0;
	}

    std::cout << "Quit." << std::endl;

	return 1;
}