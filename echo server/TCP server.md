# The lifecycle of a TCP server and client
 ### Server (typical flow)

1. socket(): make a TCP socket.
2. bind(): attach it to a local IP and port (e.g., 0.0.0.0:8080 for “all interfaces, port 8080”).
3. listen(): mark it as a listening socket.
4. accept(): wait for an incoming connection; returns a new connected socket (one per client).
5. read()/write(): exchange data with that client.
6. close(): close the connected socket. The listening socket stays open to accept more clients.

### Client (typical flow)

1. socket(): make a TCP socket.
2. connect(server_ip, server_port): the kernel performs the TCP handshake.
3. read()/write(): exchange data.
4. close(): done.

 **What actually happens under the hood (one message)**

* Your program writes bytes to a socket.
* Kernel puts bytes into a TCP send buffer, splits into network packets, wraps them in TCP/IP headers.
* Network card sends packets on the wire/Wi-Fi.
* Remote machine’s NIC receives, kernel reassembles in order, acknowledges receipt.
* The remote program’s read() copies bytes from the TCP receive buffer into user memory.
* Reliability: if a packet is lost, the sender retransmits. The receiver acks what it got.