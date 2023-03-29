#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <hostname> <port>" << std::endl;
    return 1;
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "ERROR: Failed to create socket" << std::endl;
    return 1;
  }

  struct hostent *server = gethostbyname(argv[1]);
  if (!server) {
    std::cerr << "ERROR: Failed to resolve hostname" << std::endl;
    close(sockfd);
    return 1;
  }

  struct sockaddr_in serv_addr;
  std::memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
  serv_addr.sin_port = htons(std::stoi(argv[2]));

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cerr << "ERROR: Failed to connect to server" << std::endl;
    close(sockfd);
    return 1;
  }

  std::string message;
  std::cout << "Enter message: ";
  std::getline(std::cin, message);

  int n = write(sockfd, message.c_str(), message.length());
  if (n < 0) {
    std::cerr << "ERROR: Failed to write to socket" << std::endl;
    close(sockfd);
    return 1;
  }

  char buffer[256];
  n = read(sockfd, buffer, sizeof(buffer) - 1);
  if (n < 0) {
    std::cerr << "ERROR: Failed to read from socket" << std::endl;
    close(sockfd);
    return 1;
  }

  buffer[n] = '\0';
  std::cout << "Received message: " << buffer << std::endl;

  close(sockfd);
  return 0;
}
