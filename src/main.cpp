#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>

void client2server(int client_fd, std::string username) {
	std::string bye = username + " quit the chat.";
	std::string userMessage;
	while (true) {
		std::getline(std::cin, userMessage);
		if (userMessage == "/exit") {
			send(client_fd, bye.c_str(), bye.length(), 0);
			break;
		}
		userMessage = username + " : " + userMessage;
		size_t length = userMessage.length();
		send(client_fd, &length, sizeof(size_t), 0);
		send(client_fd, userMessage.c_str(), userMessage.length(), 0);
	}
	close(client_fd);
}

void server2client(int client_fd) {
	char buffer[1024] = {0};
	while (read(client_fd, buffer, 1024) > 0) {
		std::cout << buffer << std::endl;
		memset(buffer, 0, sizeof(buffer));
	}
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		std::cerr << "Usage : " << argv[0] << " <server ip> <server port> <username>" << std::endl;
		return 1;
	}

	int client_fd;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(std::stoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &(address.sin_addr)) <= 0) {
		perror("inet_pton failed (wrong ip ?)");
		exit(EXIT_FAILURE);
	}

	if (connect(client_fd, (struct sockaddr*)&address, addrlen) < 0) {
		perror("connect failed");
		exit(EXIT_FAILURE);
	}

	std::string username = argv[3];
	std::string hello = username + " entered the chat.";
	size_t length = hello.length();
	send(client_fd, &length, sizeof(size_t), 0);
	send(client_fd, hello.c_str(), length, 0);

	std::thread input(client2server, client_fd, username);
	std::thread receiving(server2client, client_fd);

	input.join();
	receiving.join();

	return 0;
}
