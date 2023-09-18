#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <vector>
#include <algorithm>
#include <string.h>

#define SIZE 42

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[SIZE] = {0};
	std::vector<int> clientSockets;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

	fd_set read_fds;
	int max_fd = server_fd;

	while (1) {
		FD_ZERO(&read_fds);
		FD_SET(server_fd, &read_fds);
		for (size_t i = 0; i < clientSockets.size(); i++) {
			int sock = clientSockets[i];
			FD_SET(sock, &read_fds);
		}


		int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
		if (activity < 0) {
			std::cout << "error" << std::endl;
		}

		if (FD_ISSET(server_fd, &read_fds)) {
			int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
			if (new_socket > max_fd) {
				max_fd = new_socket;
			}
			clientSockets.push_back(new_socket);
		}

		for (size_t i = 0; i <= clientSockets.size(); i++) {
			int sock = clientSockets[i];
			if (FD_ISSET(sock, &read_fds)) {
				memset(buffer, 0, SIZE);
				int valread = read(sock, buffer, SIZE - 1);
				std::cout << sock << ": " << buffer << "[" << valread << "]" << std::endl;
				if (valread == 0) {
					clientSockets.erase(std::find(clientSockets.begin(), clientSockets.end(), sock));
					std::cout << sock << " closed" << std::endl;
					close(sock);
				}
				send(sock, buffer, valread, 0);
			}
		}

	}
    return 0;
}
