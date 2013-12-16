
#include <Edvs/edvs_impl.h>
#include <thread>
#include <iostream>

void read_loop(int sp, bool* is_running) {
	std::cout << "Read loop starting..." << std::endl;
	const unsigned int N = 1024;
	unsigned char buffer[N];
	while(*is_running) {
		ssize_t n = edvs_serial_read(sp, buffer, N);
		std::cout << "Received " << n << " bytes:" << std::endl;
		std::cout << "Raw: ";
		for(int i=0; i<n; i++) {
			std::cout << buffer[i];
		}
		std::cout << std::endl;
		std::cout << "Int: ";
		for(int i=0; i<n; i++) {
			std::cout << static_cast<int>(buffer[i]);
		}
		std::cout << std::endl;
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

	bool is_running = true;

	std::thread rec(&read_loop, sp, &is_running);

	while(true) {
		// user quit
		std::string str;
		std::cin >> str;
		if(str.length() == 1 && str[0] == 'q') {
			break;
		}
		// send
		if(str.length() > 0) {
			edvs_serial_write(sp, str.data(), str.length());
		}
	}

	is_running = false;

    std::cout << "Quit." << std::endl;

	if(edvs_serial_close(sp) != 0) {
		std::cout << "ERROR edvs_serial_close" << std::endl;
		return 0;
	}

	return 1;
}