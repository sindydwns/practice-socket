#include <iostream>
#include <sstream>
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

// for parser
#include "practice-parser/parser/PatternSequenceGroup.hpp"
#include "practice-parser/parser/PatternWord.hpp"
#include "practice-parser/parser/PatternReadUntil.hpp"
#include "practice-parser/parser/PatternOptionGroup.hpp"
#include "practice-parser/parser/PatternReadAll.hpp"
#include "practice-parser/parser/PatternEqual.hpp"
#include "practice-parser/parser/Parser.hpp"

#define SIZE 42
#define TIMEOUT 3

const struct timespec timer = { TIMEOUT, 0 };
class Client
{
public:
	ParseStream stream;
	std::string res;
	size_t resIdx;
};

Parser *createParser()
{
	PatternSequenceGroup *startline = new PatternSequenceGroup("startline");
    startline->addPattern(new PatternWord("method"));
    startline->addPattern(new PatternWord("route"));
    startline->addPattern(new PatternWord("http-version"));
    PatternSequenceGroup *header = new PatternSequenceGroup("header");
    header->addPattern(new PatternReadUntil(":", "\n", "key"));
    header->addPattern(new PatternReadUntil("\n", "value"));
    PatternOptionGroup *headers = new PatternOptionGroup(0, 999, "headers");
    headers->addPattern(header);
    // PatternReadAll *body = new PatternReadAll("body");
    PatternSequenceGroup *req = new PatternSequenceGroup("req");
    req->addPattern(startline);
    req->addPattern(new PatternReadUntil("\n", "nextline"));
    req->addPattern(headers);
    req->addPattern(new PatternReadUntil("\n", "nextline"));
    // req->addPattern(body);
	return new Parser(req);
}

int main() {

	Parser *parser = createParser();

    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[SIZE] = {0};
    
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
			Client *client = reinterpret_cast<Client*>(eventList[i].udata);

			// -- accept client --

			if (fd == server_fd) {
				int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
				std::cout << "new_socket [" << new_socket << "]" << std::endl;
				
				Client *new_client = new Client();
				new_client->stream = parser->makeStream();
				new_client->res = "";
				new_client->resIdx = 0;

				struct kevent clientEvent;
				EV_SET(&clientEvent, new_socket, EVFILT_READ, EV_ADD, 0, 0, new_client);
				kevent(kq, &clientEvent, 1, NULL, 0, &timer);
				continue;
			}

			// -- client act --

			if (eventList[i].filter == EVFILT_READ) {
				memset(buffer, 0, SIZE);
				int valread = read(fd, buffer, SIZE - 1);
				if (valread > 0) {
					std::string buff = std::string(buffer);
					if (client->stream.next(buff) == false) {
						client->res = "HTTP/1.1 200 OK\n";
						if (client->stream.isState(VALID)) {
							std::stringstream ss;
							std::string str = client->stream.getResult().toString();
							ss << "Content-Type: text/html\n";
							ss << "Content-Length: " << str.size() << "\n";
							ss << "Connection: keep-alive\n";
							client->res += ss.str();
							client->res += "\r\n";
							client->res += str;
						}
						if (client->stream.isState(INVALID)) {
							std::stringstream ss;
							std::string str = "req is invalid\n";
							ss << "Content-Type: text/html\n";
							ss << "Content-Length: " << str.size() << "\n";
							ss << "Connection: keep-alive\n";
							client->res += "\r\n";
							client->res += str;
						}
						struct kevent clientEvent;
						EV_SET(&clientEvent, fd, EVFILT_READ, EV_DELETE, 0, 0, client);
						EV_SET(&clientEvent, fd, EVFILT_WRITE, EV_ADD, 0, 0, client);
						kevent(kq, &clientEvent, 1, NULL, 0, &timer);
					}
				}
				std::cout << fd << ": " << buffer << "[" << valread << "]" << std::endl;
				if (valread == 0) {
					std::cout << fd << " closed" << std::endl;
					delete client;
					close(fd);
				}
			}

			if (eventList[i].filter == EVFILT_WRITE) {
				std::string &res = client->res;
				if (res.size() - client->resIdx > 0) {
					size_t i = client->resIdx;
					ssize_t written = write(fd, &(res.at(i)), res.size() - i);
					if (written > 0) client->resIdx += written;
				}
				if (res.size() == client->resIdx) {
					struct kevent clientEvent;
					EV_SET(&clientEvent, fd, EVFILT_WRITE, EV_DELETE, 0, 0, client);
					kevent(kq, &clientEvent, 1, NULL, 0, &timer);
				}
			}

			// timer is not working... why?
			if (eventList[i].filter == EVFILT_TIMER) {
				std::cout << fd << " timeover" << std::endl;
				delete client;
				close(fd);
			}
		}
	}
	delete parser;
    return 0;
}
