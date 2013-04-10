#ifndef INCLUDE_EDVS_NETSOCKET
#define INCLUDE_EDVS_NETSOCKET

#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include "Device.hpp"

namespace Edvs
{
	class NetSocket : public Device
	{
	public:
		struct Exception
		: public std::runtime_error
		{
			Exception(const std::string& msg)
			: std::runtime_error(msg) {}
		};

		NetSocket(const std::string& address)
		{
			sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd_ == -1)
			{
				throw Exception("create socket error");
			}

			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(atoi(address.substr(address.find(":")+1).c_str()));

			int i = inet_pton(AF_INET, address.substr(0, address.find(":")).c_str(), &addr.sin_addr);

			if(i<=0)
			{
				throw Exception("inet p_ton error");
			}

			if(connect(sockfd_, (struct sockaddr*)&addr, sizeof(addr)))
			{
				throw Exception("connect error");
			}

			std::cout << address.substr(0, address.find(":")) << std::endl;
		}

		~NetSocket()
		{
			int r = shutdown(sockfd_, SHUT_RDWR);
			if (r != 0)
			{
				throw Exception("socket shutdown error!");	
			}
			r = close(sockfd_);
			if (r != 0)
			{
				throw Exception("socket close error!");	
			}					
		}

		/** Writes a command string to the socket
		 * Warning: Command string must end with a '\n'!
		 */
		void sendCommand(const std::string& str) {

			int n = send(sockfd_, str.c_str(), str.length(), 0);
			if(n != (int)str.length()) {
				throw Exception("Could not write correct number of bytes!");
			}
		}

		/** Reads some data from the socket */
		size_t read(size_t n, char* data) {
			ssize_t actual = recv(sockfd_, data, n, 0);
			if(actual < 0) {
				throw Exception("Could not read from socket!");
			}
			return size_t(actual);
		}

		int sockfd_;
	};
}

#endif
