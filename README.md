# Simple HTTP Client in C

A minimalist, standards-compliant HTTP client written from scratch in C using POSIX stream sockets (`SOCK_STREAM` / TCP). It resolves a domain name via DNS, establishes a connection over port 80, sends a raw HTTP GET request, and streams the server's response (headers + HTML) directly to standard output.

---

## Architecture & Lifecycle

```
                     getaddrinfo()
  "google.com" : 80 ──────────────► DNS Resolution (Linked List of IPs)
                                           │
                                           ▼
                                        socket()
                               (Create network file descriptor)
                                           │
                                           ▼
                                       connect()
                                (TCP 3-Way Handshake)
                                           │
                                           ▼
                                        send()
                                (Send raw HTTP request)
                                           │
                                           ▼
                                        recv()
                            (Read stream chunks in a loop)
                                           │
                                           ▼
                                        close()
                               (Tear down connection)
```

---

## Key Concepts Explained

### 1. `getaddrinfo` & Structs
* **`struct addrinfo hints`**: Provides criteria for the DNS query (`AF_INET` for IPv4, `SOCK_STREAM` for TCP).
* **`memset(&hints, 0, sizeof(hints))`**: Sockets structures allocate stack memory that contains random garbage bytes unless zeroed out.
* **`res` Linked List**: A domain name may resolve to multiple IP addresses for redundancy. `getaddrinfo()` returns a singly-linked list accessed via `p->ai_next`.

### 2. Type Casting & Address Extraction
* `p->ai_addr` is a pointer to a generic `struct sockaddr`.
* Casting it to `(struct sockaddr_in *)` allows accessing IPv4-specific fields, such as `ipv4->sin_addr`.
* `inet_ntop()` converts raw binary network address bytes into human-readable text (e.g., `192.178.177.113`).

### 3. Sockets & TCP Connection
* **`socket()`**: Requests a network file descriptor (`sockfd`) from the operating system kernel.
* **`connect()`**: Initiates the TCP three-way handshake (`SYN` $\rightarrow$ `SYN-ACK` $\rightarrow$ `ACK`) against the target host and port.
* The loop iterates through resolved addresses, advancing to the next candidate if `socket()` or `connect()` fails.

### 4. HTTP Wire Format & Transmission
* HTTP over TCP is plaintext. Lines terminate with `\r\n` (CRLF), and an empty line (`\r\n\r\n`) denotes the end of headers:
  ```http
  GET / HTTP/1.1\r\n
  Host: google.com\r\n
  User-Agent: MyCClient/1.0\r\n
  Connection: close\r\n
  \r\n
  ```
* `Connection: close` instructs the remote web server to shut down the TCP connection immediately after transmitting the payload, signaling to our reader loop that transmission is complete.

### 5. Buffered Streaming (`recv`)
* The client receives the payload in chunks using a fixed-size buffer (`4096` bytes).
* `recv()` returns:
  * `> 0`: Bytes received in this chunk.
  * `0`: Orderly disconnect by the server (end of transmission).
  * `-1`: Network or transmission error.
* `buffer[bytes_received] = '\0'` null-terminates each incoming chunk so it can be safely printed as a string.

---

## Building & Running

### Requirements
* GCC or Clang
* POSIX-compliant OS (macOS, Linux)

### Compilation
```bash
gcc -Wall -Wextra httpClient.c -o httpClient
```

### Execution
```bash
./httpClient
```

### Expected Output
```text
Resolved IP Addresses for google.com:
IPv4: 192.178.177.113
Sent 82 bytes.
HTTP/1.1 301 Moved Permanently
Location: http://www.google.com/
Content-Type: text/html; charset=UTF-8
...
<HTML><HEAD><meta http-equiv="content-type" content="text/html;charset=utf-8">
<TITLE>301 Moved</TITLE></HEAD><BODY>
<H1>301 Moved</H1>
The document has moved
<A HREF="http://www.google.com/">here</A>.
</BODY></HTML>
Successfully connected to google.com on port 80!
```
### Additional Information
It was a learning project, Learned through BeejGuideToNetworking and use of AI to understand complex concepts. 
Use of AI was to create the Readme in simple understandble language, and to made a step by step learning guide for this project. 
