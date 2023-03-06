/*
 * This code was largely copied from
 * https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/client.c
 */

#include "ipc/call_guard.hpp"
#include "ipc/irq_server.hpp"

#include <array>
#include <future>

void client(char buf[16]) {
  sockaddr_in serv_addr{};
  hostent    *server;

  fd_factory fdf;

  auto host_fd = fdf.make_fd(call_guard(socket, AF_INET, SOCK_STREAM, 0));

  server = gethostbyname("localhost");
  if (server == nullptr) {
    fprintf(stderr, "%s is not a valid hostname\n", "localhost");
    std::exit(0);
  }

  std::memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);

  int portno         = std::atoi("3425");
  serv_addr.sin_port = htons(portno);

  call_guard(connect, host_fd->operator int(), (struct sockaddr *)&serv_addr,
             sizeof(serv_addr));

  // printf("send: ");
  //  fgets(buf.data(), buf.size() - 1 /* zero terminate to prevent overflow */,
  //       stdin);

  call_guard(write, host_fd->operator int(), buf, 16);
  sprintf(buf, " ");
  call_guard(read, host_fd->operator int(), buf, 16);

  printf("client read: %s\n", buf);

  sprintf(buf, "%s-2", buf);
  call_guard(write, host_fd->operator int(), buf, 16);

  sprintf(buf, " ");
  call_guard(read, host_fd->operator int(), buf, 16);

  printf("client read: %s\n", buf);
}

int main() {

  auto x = std::async(std::launch::async, [] {
    irq_server serv(3425, 5);
    while (serv.poll(1000))
      ;

    return true;
  });

  char buf[16];
  for (int i = 0; i < 10; ++i) {
    sprintf(buf, "test%d", i);
    client(buf);
  }
}
