#ifndef OASIS_NET_TCP_H
#define OASIS_NET_TCP_H

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

namespace oasis {
namespace net {

class TcpConnection {
private:
  int connfd;
  struct sockaddr_in client_addr;
  int client_addr_len;
  bool is_closed = false;

public:
  TcpConnection(int cfd, struct sockaddr_in addr, int len)
      : connfd(cfd), client_addr(addr), client_addr_len(len) {}

  bool isClosed() const { return is_closed; }

  int read(const char *buf, int size) {
    const int result = recv(connfd, (void *)buf, size, 0);
    if (result == 0) {
      is_closed = true;
    }
    return result;
  }

  int readExact(char *buf, int size) {
    int result = recv(connfd, buf, size, MSG_WAITALL);
    if (result == 0) {
      is_closed = true;
    }
    return result;
  }

  void write(const std::vector<uint8_t> &buf) {
    send(connfd, buf.data(), buf.size(), 0);
  }

  ~TcpConnection() { close(connfd); }
};

class TcpSocket {
private:
  int sockfd;

public:
  TcpSocket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
      std::cout << "[TcpSocket] socket() failed. error: " << errno << std::endl;
      exit(0);
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);

    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
      std::cout << "[TcpSocket] bind() failed. error: " << errno << std::endl;
      exit(0);
    }

    int max_pending_conns = 20;
    if ((listen(sockfd, max_pending_conns)) != 0) {
      std::cout << "[TcpSocket] listen() failed. error: " << errno << std::endl;
      exit(0);
    }
  }

  TcpConnection acceptConn() {
    struct sockaddr_in addr;
    socklen_t addrlen;
    int connfd = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
    if (connfd < 0) {
      exit(0);
    }
    return TcpConnection(connfd, addr, addrlen);
  }

  ~TcpSocket() {
    close(sockfd);
  }
};

}; // namespace net
}; // namespace oasis

#endif // OASIS_NET_TCP_H
