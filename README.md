*This project has been created as part of the 42 curriculum by yel-mota,[ obajali,[ cfarai]].*

## Description
**Webserv** is a custom-built, fully functional HTTP/1.1 web server written entirely in C++98. The goal of this project is to deeply understand the underlying mechanics of the HTTP protocol, non-blocking I/O operations, multiplexing (using `poll()`), and socket programming. 

It is designed to be fully non-blocking and robust, supporting core HTTP methods (GET, POST, DELETE), directory auto-indexing, custom error pages, session management via cookies, and dynamic multi-language CGI execution (Python & PHP)—all managed through an NGINX-inspired configuration file.

### Key Features
- Multiplexed, non-blocking I/O using `poll()`.
- NGINX-style configuration file parsing (routing, ports, error pages, client limits).
- Supports GET, POST, and DELETE methods.
- Dynamic CGI execution (concurrent PHP & Python).
- File uploading and chunked transfer encoding.
- Directory auto-index generation.
- Built-in session tracking via cookies.
- A beautiful **TokyoNight-themed interactive frontend dashboard** to test all features.

## Instructions

**Compilation:**
The project includes a `Makefile`. To compile the web server, simply run:
```bash
make
```
This will compile the C++98 source files and generate the `webserv` executable.

**Execution:**
Start the server by providing a configuration file. 
```bash
./webserv [path/to/config.conf]
```
For example, to run the server with our provided testing dashboard:
```bash
./webserv config/site.conf
```
Once the server is running, open your web browser and navigate to `http://localhost:8080` to interact with the Webserv Dashboard.

## Resources
- **[RFC 2616 (HTTP/1.1)](https://www.rfc-editor.org/rfc/rfc2616):** The core specification for the HTTP/1.1 protocol.
- **[RFC 3875 (CGI 1.1)](https://www.rfc-editor.org/rfc/rfc3875):** The Common Gateway Interface specification used to implement dynamic scripts.
- **[Beej's Guide to Network Programming](https://beej.us/guide/bgnet/):** The definitive guide to understanding sockets, `poll()`, and C-style networking.
- **AI Usage:** Artificial Intelligence (Google Gemini) was utilized exclusively to assist in designing and styling the interactive TokyoNight frontend dashboard (`site/` directory), formatting CSS animations and scroll-snapping, and generating this README.md file. AI was **not** used to write the core C++98 backend logic, socket management, HTTP response construction, or configuration parsing.
