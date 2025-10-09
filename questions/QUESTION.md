1. we are doing FD_SET(listen_fd,&rdfs) upper then why we are checking it again accepting the clients ?
 **Reason:**
 ``` select() modifies the rfds set. After select() returns:
  * rfds only contains file descriptors that are ready
  * FD_ISSET() checks if listen_fd is among the ready ones
  * If yes → new client wants to connect 
  ```

2.  cli (struct sockaddr_in) , each client has its own or there is only one cli structure ?
**Answer:**
```
 There's only ONE cli structure, but it's reused:

* Each time accept() is called, it overwrites cli with the new client's info
* It's just temporary storage for the current connection being processed
* Not persistent - data is lost after that iteration


```
3. in short if the socket is created between (through the accept fuction) then we can use fd only to communicate , the info is not required then ?
**Answer**
```Client info (cli structure) is only needed for:

* Logging: "Client 127.0.0.0.1 connected"
* Security: Block certain IP addresses
* Statistics: Track connections per IP

For basic communication: File descriptor is enough
The socket connection is identified by the cli_fd, not the IP/port. The network stack handles routing automatically.
Think of it like a phone call:

* Phone number = IP:port (used to establish connection)
* Call ID = file descriptor (used during conversation)
* Once connected, you only need the Call ID to talk!

in short , once accept returns fd , that's all we need for communication
```
4. Listening Socket Role my thinking (which is correct)```  we create first socket , listening socket , it is used to bind and connect to the port , then we use this socket to gets new requests from clients to connect```
**Working of Sockets**
```Listening Socket (listen_fd):
├── Bound to port (e.g., 8080)
├── Listens for connection requests
├── When client connects → becomes "ready for reading"
└── accept() creates NEW socket for that client

Client Sockets (cli_fd):
├── Each client gets unique file descriptor
├── Used for actual data communication
└── Independent of listening socket

Flow:

1. Listening socket = "receptionist" (handles new arrivals)
2. Client sockets = "individual phone lines" (handle conversations)
```

5. What is "Binary Network" Format?
```
Computers store IP addresses as binary numbers:
Format Example 
Human  : "192.168.1.100"
Binary : 0xC0A80164 (32-bit integer)
// cli.sin_addr contains: 0xC0A80164 (binary)
// inet_ntop converts to: "192.168.1.100" (string)
```
6. Why Binary Format is Needed ?
**Reason**
```
1. Network efficiency: Routers work with 32-bit integers, not strings
2. Memory: 0xC0A80164 (4 bytes) vs "192.168.1.100" (12 bytes + null)
3. Speed: Binary comparisons are faster than string comparisons
4. Network protocols: All network packets use binary addresses
```

7. Why does copy-paste show random/partial text?
**Reason**
```
Problem: Large pasted text appears in fragments, not all at once.
Answer: This is normal TCP behavior:

* TCP Packet Fragmentation: Large data splits into multiple packets
* Terminal Buffering: Terminals send large pastes in chunks
* Network Buffering: OS delivers data as it arrives
* Buffer Size: Server reads max 4096 bytes at a time

Example:
Paste: "Very long code snippet..."
Arrives as:
Chunk 1: "Very long"
Chunk 2: " code snippet"
Chunk 3: "..."

Solutions:

1. Accept it (normal behavior)
2. Buffer until complete (e.g., until newline)
3. Show fragment numbers for debugging
```
8. Why is some copied data not printing on server side?
```Problem: Character-by-character printing causes display issues:
Answer: Multiple issues:

* Non-printable characters (tabs, nulls) mess up display
* No newlines after data chunks
* Printf buffering with individual characters
* Special characters might not display
```