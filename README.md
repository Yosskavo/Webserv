# Webserv

### **Phase 1: Raw TCP Socket Foundations**

* [x] **Create the Socket:** Initialized a raw socket descriptor using `socket()`.
* [x] **Address Binding:** Configured `sockaddr_in` structures and tied the socket to a port using `::bind()`.
* [x] **Namespace Fix:** Fixed the method collision between your class's `Csocket::bind` and the global POSIX `::bind()` using the scope resolution operator (`::`).
* [x] **Passive Listening:** Set up the connection backlog queue using `listen()`.
* [x] **Port Reusability:** Configured `setsockopt()` with `SO_REUSEADDR` to prevent the annoying "Address already in use" kernel lockouts.

---

### **Phase 2: Non-Blocking Multiplexing (`poll`)**

* [x] **Asynchronous Mode:** Turned on `O_NONBLOCK` via `fcntl()` to prevent `accept()` and `recv()` from freezing the server thread.
* [x] **Connection Architecture:** Structured a contiguous array system to watch descriptors (Index 0 for the Server, Indexes 1+ for clients).
* [x] **The Heap Transition:** Transitioned your memory allocation layout from standard stack arrays over to manual heap memory arrays using C++98 `new[]` and `delete[]`.
* [x] **The Loop Spin Bugfix:** Corrected the fatal `for (i < ready)` event tracking bug by expanding the loop scale to evaluate `fds.size()`.
* [x] **The 100% CPU Death Spin Fix:** Configured your `recv()` logic to explicitly trap `bytes_read <= 0` markers, successfully closing files and purging disconnected clients from memory instead of infinitely looping.
* [x] **Bitwise Masking:** Mastered using `&` and `|` bitmasks to safely check for combinations of `POLLIN`, `POLLOUT`, `POLLHUP`, and `POLLERR`.

---

### **Phase 3: The HTTP Protocol & Buffer Stream Parsing**

* [x] **TCP Stream Tracking:** Discovered that TCP is a byte stream—not a text line mechanism—meaning it will never pause at a `\n` on its own.
* [x] **Memory Sanitization:** Cleared out leftover RAM residue and eliminated string corruption bugs by zeroing your receipt blocks via `std::memset()`.
* [x] **Delimiter Isolation:** Maintained custom persistent client storage tracking strings to pinpoint the raw HTTP header divider (`\r\n\r\n`).
* [x] **Request Tokenization:** Applied C++98 `std::istringstream` streams to split the initial HTTP Request line automatically into isolated `method`, `path`, and `protocol` strings.
* [x] **HTTP Conversation Mechanics:** Memorized the raw textual architecture of headers, status responses (`200 OK`, `404 Not Found`), and metadata tags (`Content-Length`, `Content-Type`).

---

### **Phase 4: Routing & Static File Serving (Current Step)**

* [x] **The Action Filter:** Confirmed that your primary target focus for processing incoming browser actions is strictly filtered down to **`GET`**.
* [x] **Document Root Virtualization:** Designed the layout concept of mapping a local subdirectory (`./www`) over path targets to secure your operating system's root files from malicious input.
* [ ] **File System Input Stream:** Write the binary C++98 `std::ifstream` code layout using `.c_str()` parsing strings to fetch real storage assets off your hard drive.
* [ ] **MIME-Type Translation Mapping:** Implement basic file extension filters (checking if a path ends in `.html` or `.css`) to automatically supply accurate `Content-Type` properties to web clients.
* [ ] **Error Status Trapping:** Write the fallback response generation code blocks to cleanly return specialized `404 Not Found` pages back to web browsers if requested file resources are missing.


