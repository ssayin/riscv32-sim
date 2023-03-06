/*
 * This code was largely copied from
 * https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/client.c
 */

#include "ipc/call_guard.hpp"
#include "ipc/fd.hpp"
#include <array>

void client(std::string_view hostname, std::string_view port);

void client(std::string_view hostname, std::string_view port) {
  sockaddr_in serv_addr{};
  hostent    *server;
  fd_factory  fdf;

  auto host_fd = fdf.make_fd(call_guard(socket, AF_INET, SOCK_STREAM, 0));

  server = gethostbyname(hostname.data());
  if (server == nullptr) {
    fprintf(stderr, "%s is not a valid hostname\n", hostname.data());
    std::exit(0);
  }

  std::memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);

  int portno         = std::atoi(port.data());
  serv_addr.sin_port = htons(portno);

  call_guard(connect, host_fd->operator int(),
             std::bit_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr));

  std::array<char, 256> buf{};
  printf("send: ");
  fgets(buf.data(), buf.size() - 1 /* zero terminate to prevent overflow */,
        stdin);

  call_guard(write, host_fd->operator int(), buf.data(), strlen(buf.data()));
  std::fill(buf.begin(), buf.end(), 0);
  call_guard(read, host_fd->operator int(), buf.data(), 255);

  printf("read %s\n", buf.data());
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "USAGE: %s <hostname> <port>\n", argv[0]);
    std::exit(0);
  }

  client(argv[1], argv[2]);

  return 0;
}
