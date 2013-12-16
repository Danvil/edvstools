
#include <Edvs/edvs_impl.h>
#include <iostream>

int main(int argc, char** argv)
{
	int sp = edvs_serial_open("/tty/USB0", 4000000);
	if(sp == -1) {
		std::cout << "ERROR edvs_serial_open" << std::endl;
		return 0;
	}

	const unsigned int N = 1024;
	unsigned char buffer[N];

	while(true) {
		// user quit
		char c;
		std::cin >> c;
		if(c == 'q') {
			break;
		}
		// read
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

	if(edvs_serial_close(sp) != 0) {
		std::cout << "ERROR edvs_serial_close" << std::endl;
		return 0;
	}

	return 1;
}