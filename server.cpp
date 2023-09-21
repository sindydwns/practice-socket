#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <vector>
#include <algorithm>
#include <string.h>

// for kqueue
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

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

	int kq = kqueue();
	struct kevent serverEvent;
	EV_SET(&serverEvent, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(kq, &serverEvent, 1, NULL, 0, NULL);

	while (1) {
		struct kevent eventList[1024];
		int nev = kevent(kq, NULL, 0, eventList, 1024, NULL);
		for (int i = 0; i < nev; i++) {
			int fd = eventList[i].ident;

			if (fd == server_fd) {
				int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
				clientSockets.push_back(new_socket);
				std::cout << "new_socket [" << new_socket << "]" << std::endl;
				
				struct kevent clientEvent;
				EV_SET(&clientEvent, new_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
				kevent(kq, &clientEvent, 1, NULL, 0, NULL);
			}
			else {
				memset(buffer, 0, SIZE);
				int valread = read(fd, buffer, SIZE - 1);
				std::cout << fd << ": " << buffer << "[" << valread << "]" << std::endl;
				if (valread == 0) {
					clientSockets.erase(std::find(clientSockets.begin(), clientSockets.end(), fd));
					std::cout << fd << " closed" << std::endl;
					close(fd);
				}
				else {
					send(fd, buffer, valread, 0);
				}
			}
		}
	}
    return 0;
}
