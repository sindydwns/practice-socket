#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char* message = "GET / HTTP/1.1\nHost: localhost:8080\nAccept: */*\n\n";

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // 서버의 IP 주소 설정
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        return -1;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // 서버에 메시지 전송
    send(sock, message, strlen(message), 0);
    std::cout << "Message sent" << std::endl;

    // 서버로부터의 응답 수신
	// char buffer[1024] = {0};
	// size_t total = 0;
	// while (1) {
	// 	memset(buffer, 0, 1024);
	// 	// int size = read(sock, buffer, 1023);
	// 	// total += size;
	// 	// if (size > 0) {
	// 	// 	std::cout << buffer << "[" << size << "]" << std::endl;
	// 	// }
	// 	// if (size <= 0 || total >= strlen(message)) {
	// 	// 	break;
	// 	// }
	// }
	std::cout << std::endl;
    while (1) {}
    close(sock);

    return 0;
}
