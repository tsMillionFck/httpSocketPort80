#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  const char *hostname = "google.com";
  const char *port = "80";
  const char *request = "GET / HTTP/1.1\r\n"
                        "Host: google.com\r\n"
                        "User-Agent: MyCClient/1.0\r\n"
                        "Connection: close\r\n"
                        "\r\n";

  char ip_str[INET6_ADDRSTRLEN];
  int sockfd = -1;

  struct addrinfo hints;
  struct addrinfo *res = NULL;
  struct addrinfo *p = NULL;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(hostname, port, &hints, &res);
  if (status != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }

  printf("Resolved IP Addresses for %s:\n", hostname);

  for (p = res; p != NULL; p = p->ai_next) {
    void *raw_ip_bytes;
    const char *ip_version;

    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      raw_ip_bytes = &(ipv4->sin_addr);
      ip_version = "IPv4";
    } else {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      raw_ip_bytes = &(ipv6->sin6_addr);
      ip_version = "IPv6";
    }

    inet_ntop(p->ai_family, raw_ip_bytes, ip_str, sizeof(ip_str));
    printf("%s: %s\n", ip_version, ip_str);

    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      perror("Client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("Client: connect");
      continue;
    }

    int bytes_sent = send(sockfd, request, strlen(request), 0);
    if (bytes_sent == -1) {
      perror("Client: send");
      close(sockfd);
      continue;
    }
    printf("Sent %d bytes.\n", bytes_sent);

    char buffer[4096];
    int bytes_received;

    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
      buffer[bytes_received] = '\0';
      printf("%s", buffer);
    }

    if (bytes_received == -1) {
      perror("Client: recv");
      close(sockfd);
    }

    close(sockfd);
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "Client: Failed to connect to %s\n", hostname);
    freeaddrinfo(res);
    return 2;
  }

  printf("Successfully connected to %s on port %s!\n", hostname, port);

  freeaddrinfo(res);

  return 0;
}