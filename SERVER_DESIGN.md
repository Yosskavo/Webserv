# `server.cpp` — Design Document

This document explains how `server.cpp` is architected: the core idea, the data
structures, the event loop, and the full lifecycle of a request from the first
byte read to the last byte written. It also lists the CGI subsystem and a few
known fragile spots worth being able to explain.

---

## 1. Core idea in one sentence

> **One thread, one `poll()` loop, non-blocking sockets. Every connection is a
> small state machine, and every file descriptor — listeners, clients, and CGI
> pipes — lives in the same `poll` set and is dispatched by which map it belongs
> to.**

There are no threads, no blocking reads/writes on client sockets, and the server
never waits on a single client. Progress happens in tiny steps driven by
readiness events from `poll`.

---

## 2. The three kinds of file descriptors

Every fd in the `listeners` vector is exactly one of three things. The dispatcher
in `run()` figures out which by asking *which map contains this fd*:

| Kind | Stored in | Recognized in `run()` by | Handler |
|------|-----------|--------------------------|---------|
| **Listening socket** | `servers` (`map<int, vector<t_config>>`) | `servers.count(fd)` | `accept_new_clients()` |
| **Client socket** | `clients` (`map<int, t_client>`) | `clients.count(fd)` | `handle_client_read()` / `handle_client_write()` |
| **CGI output pipe** | `fd_to_pid` + `cgis` | `fd_to_pid.count(fd)` | `handle_cgi_read()` / `handle_cgi_write()` |

This is the central trick of the whole file: **a fd's type is defined by its
membership in a map**, so the poll loop stays a flat `if / else if / else if`.

```
                 ┌─────────────────────────────────────────────┐
                 │              poll(listeners)                 │
                 └─────────────────────────────────────────────┘
                                     │ revents
              ┌──────────────────────┼──────────────────────┐
              ▼                      ▼                      ▼
       servers.count(fd)     clients.count(fd)     fd_to_pid.count(fd)
              │                      │                      │
       accept_new_clients    read / write client     read / write CGI pipe
```

---

## 3. Key data structures

All state lives inside the `server` class (see `server.hpp`), so there are no
globals except the ignored `SIGPIPE`.

### `server` members
```cpp
std::map<int, std::vector<t_config> > servers;   // listener fd -> virtual hosts on it
std::vector<struct pollfd>            listeners;  // THE poll set (all 3 fd kinds)
std::map<std::pair<std::string,size_t>, int> know_exist; // (ip,port) -> listener fd  (dedup)
std::map<int, t_client>               clients;    // client fd -> connection state
std::map<pid_t, t_CgiProcess>         cgis;       // pid -> running CGI
std::map<int, pid_t>                  fd_to_pid;  // CGI pipe fd -> pid
```

- `servers` maps **one listening socket to a list of `t_config`** because several
  `server {}` blocks can share the same `ip:port` (name-based virtual hosts).
  `know_exist` ensures we only ever `socket()/bind()/listen()` **once** per
  `ip:port`, then just append the extra configs.
- `fd_to_pid` + `cgis` are two hops on purpose: `poll` gives us a fd, we look up
  the pid, then the `t_CgiProcess`.

### `t_client` — one connection (server.hpp)
```cpp
int          fd;
ClientState  state;      // where this connection is in its lifecycle
std::string  inbuf;      // raw bytes read from the socket, not yet consumed
t_request    request;    // parsed request + body handling
t_response   response;   // status/headers/body + serialized outbuf
bool         keep_alive;
int          server_fd;  // the listener this client arrived on
t_config*    server;     // default vhost for that listener
t_location*  location;   // matched location block (set during routing)
```

### `ClientState` — the state machine
```cpp
READING_HEADERS  →  READING_BODY  →  ROUTING  →  WRITING_RESPONSE  →  DEAD
                         (POST)                        │
                                                       └─(keep-alive)→ back to READING_HEADERS
WAITING_CGI  (parked while a CGI child runs)
```

### `t_CgiProcess`
```cpp
pid_t pid;
int   in_fd, out_fd;         // pipe ends the server keeps
int   client_fd;             // which client is waiting on this CGI
bool  stdin_closed, stdout_closed;
std::string outbuf;          // everything the CGI has printed so far
```

---

## 4. Startup — `init()`

`init(std::vector<t_config>& list)` turns parsed config blocks into listening
sockets:

1. `signal(SIGPIPE, SIG_IGN)` — writing to a closed socket must not kill us.
2. For each config, build the `(ip, port)` key.
   - **Already seen** → just `servers[fd].push_back(config)` (extra virtual host).
   - **New** → `socket()` → `fcntl(O_NONBLOCK)` → `setsockopt(SO_REUSEADDR)` →
     `bind()` → `listen(SOMAXCONN)`, then register a `pollfd{fd, POLLIN}` in
     `listeners` and record it in `know_exist` and `servers`.
3. Any syscall failure calls `print_error_exit()`, which closes every listener
   and exits.

`ip_convert()` is a minimal helper: `"127.0.0.1"` → loopback, anything else →
`INADDR_ANY`.

---

## 5. The event loop — `run()`

```
while (1):
    poll(listeners, -1)                 # block until something is ready
    (EINTR → retry, other error → break)

    for each listeners[i]:
        if it's a listener   → accept_new_clients()
        elif it's a client:
            POLLERR/POLLNVAL → mark DEAD
            POLLIN           → handle_client_read()   (unless WAITING_CGI)
            POLLHUP          → mark DEAD
            POLLOUT          → handle_client_write()
        elif it's a CGI pipe:
            POLLERR/POLLNVAL → clean up + 500
            POLLIN/POLLHUP   → handle_cgi_read()
            POLLOUT          → handle_cgi_write()

    # --- reaping phase, after the fd sweep ---
    erase every client whose state == DEAD          (cleaning_client)
    reap every CGI whose stdin_closed && stdout_closed (waitpid + erase)
```

Two design choices to note:

- **Deferred cleanup.** Connections are never freed mid-sweep from the client
  side; they are only *marked* `DEAD`, then reaped in a separate pass at the
  bottom of the loop. This avoids invalidating the iterator/index while we are
  still walking `listeners`.
- **`WAITING_CGI` clients ignore `POLLIN`** so a client can't be re-read while
  its CGI child is still producing the response.

---

## 6. Accepting connections — `accept_new_clients()`

`accept()` the new fd, set `O_NONBLOCK`, push a `pollfd{fd, POLLIN}` into
`listeners`, and create a `t_client` in `clients` with:
- `state = READING_HEADERS`
- `server = &servers[listener_fd][0]` — the **first** config on that listener is
  the default vhost until the `Host:` header lets us pick a better one.

---

## 7. Reading & parsing — `handle_client_read()`

This one function drives `READING_HEADERS → READING_BODY → ROUTING` in a single
call using fall-through `if` blocks (not `else if`), so a read that completes the
headers can immediately continue into body handling in the same invocation.

```
recv() into a 1KB buffer
  n < 0  → EAGAIN/EWOULDBLOCK: return;  else DEAD
  n == 0 → peer closed: DEAD
  else   → append to client.inbuf

if state == READING_HEADERS:
    find "\r\n\r\n"
      not found → return (wait for more)
      found     → parse_request(); choose_server();
                  erase header bytes from inbuf;
                  next state = READING_BODY (POST) or ROUTING

if state == READING_BODY:            # note: not "else if" — falls through
    ensure a /tmp body file is open (body_fd)
    if chunked   → decode chunks into the temp file, enforce max_body, 0-chunk → ROUTING
    else         → accumulate until inbuf has content_length bytes, enforce max_body → ROUTING

if state == ROUTING:                 # falls through again
    route(client)
```

### `parse_request()`
Splits the request line off the first `\r\n`, hands it to `ft_method()`, hands the
rest to `ft_headers()`, and flags `is_chunked` when
`Transfer-Encoding: chunked` is present. (The actual tokenizing lives in
`ft_method` / `ft_headers` under `parce/` — `server.cpp` only orchestrates.)

### `choose_server()` — virtual-host selection
Reads the `Host:` header, strips any `:port`, and scans the configs on that
listener for a matching `server_name`. Match → that config becomes
`request.server`; no match → keep the listener's default config. This is
name-based virtual hosting.

### Body handling details
- **Bodies always go to a temp file** `/tmp/webserv_body_<fd>_<rand>` opened
  lazily on first body byte. This keeps large uploads off the heap and lets CGI
  read the body straight from a file.
- **`client_max_body_size`** is enforced per-location (falling back to the
  server's value) and returns **413** the moment it is exceeded.
- **Chunked decoding** is done inline: read hex size, wait until the full chunk
  (+ CRLFs) is buffered, append to the temp file, repeat until the `0` chunk.

---

## 8. Routing — `route()`

The request has been parsed; now decide what to do with it:

1. **Directory redirect.** If the target resolves to a directory and does *not*
   end in `/`, reply **301** to `target + "/"`.
2. **Location match.** `choose_location()` picks the longest matching `location`
   prefix (with proper `/` boundary checks so `/foo` doesn't match `/foobar`).
   No match → **404**.
3. **Method check.** `is_method_allow()` tests the location's `allow_method`
   bitmask (`GET|POST|DELETE`). Not allowed → **405**.
4. **Return / redirect directive.** If the location has a `return`, reply with
   that code + location.
5. **CGI dispatch.** If the target's extension is in `location->cgi_ext`, hand
   off to `start_cgi()`.
6. **Otherwise dispatch by method** → `handle_get` / `handle_post` /
   `handle_delete`.

### `choose_location()`
Longest-prefix match. For each location whose `path` is a prefix of the target,
it accepts the match only if it's exact, or the boundary is a `/` (on either the
target or the path), then keeps the **longest** such path.

---

## 9. Static file serving

| Function | Job |
|----------|-----|
| `handle_get()` | `stat()` the path. Missing → 404. Directory → try index files, else autoindex, else 404. File → `serve_file()`. |
| `get_index()` | Try each configured `index` filename inside the directory; first hit is served with 200. |
| `generate_index()` | Autoindex: `opendir` + `readdir` into an HTML listing of `<a>` links. |
| `serve_file()` | Slurp file (binary) into `response.body`, set `Content-Type` from `getContentType()`, 200. Unreadable → 403. |
| `getContentType()` | Extension → MIME (`.html`, `.css`, `.js`, `.jpg`, `.png`, `.py`), default `application/octet-stream`. |

### `handle_post()`
If the target is an existing directory → 200 "OK" (no write). Otherwise open the
path for binary write, dump `request.body` into it, and reply **201 Created**.
Open failure → 500.

### `handle_delete()`
`stat()` → missing = 404, directory = 403. Otherwise `remove()`; `EACCES` → 403,
other failure → 500, success → **204 No Content**.

---

## 10. CGI subsystem

### `start_cgi()` — spawn
1. `pipe(std_out)` always; `pipe(std_in)` only for POST.
2. `fork()`.
3. **Child:**
   - POST → open the saved body temp file and `dup2` it onto **stdin (fd 0)**.
   - GET → `dup2` `/dev/null` onto stdin.
   - `dup2(std_out[1], 1)` so the CGI's stdout flows back to us.
   - Build `argv` (`build_argv`) and the CGI environment (`build_env`), then
     `execve`.
4. **Parent:** close the write end, register `std_out[0]` (non-blocking) in
   `listeners` with `POLLIN`, record the `t_CgiProcess`, set the client to
   `WAITING_CGI`, and stop polling the client fd (`set_events(client.fd, 0)`).

### `build_env()`
Assembles the CGI/1.1 environment: `REQUEST_METHOD`, `QUERY_STRING`,
`CONTENT_LENGTH`, `CONTENT_TYPE`, `SCRIPT_FILENAME`, `SCRIPT_NAME`, `PATH_INFO`,
`REQUEST_URI`, `SERVER_PROTOCOL/NAME/PORT`, `GATEWAY_INTERFACE`, cookies, and
every request header re-exported as `HTTP_*` (dashes → underscores, uppercased).

### `handle_cgi_read()` — collect output
`read()` the pipe into `cgi.outbuf`. On EOF (`n == 0`): clean up the pipe, run
`parse_cgi()` to split CGI headers from the body, `build_response()`, and switch
the client to `WRITING_RESPONSE`. Read error → 502.

### `parse_cgi()`
Splits the CGI output on `\r\n\r\n` (or `\n\n`), parses the header block via
`ft_cgi()`, and sets the body. Malformed → false → 502.

### Reaping
CGIs are reaped in the bottom of `run()` once both pipe ends are closed:
`waitpid(..., WNOHANG)` + erase from `cgis`.

---

## 11. Building & writing the response

### `build_response()`
Serializes the whole response into `response.outbuf`:
```
HTTP/1.1 <code> <reason>\r\n
Content-Length: <body size>\r\n
Content-Type: <type, defaulted to text/plain>\r\n
Connection: keep-alive | close\r\n
<other headers...>\r\n
\r\n
<body>
```

### `handle_client_write()`
Writes `outbuf` starting at `bytes_sent`, advancing that offset each call
(handles partial writes across multiple `POLLOUT` events). When everything is
sent:
- **keep-alive** → `reset_client()` (fresh request/response, back to
  `READING_HEADERS`, re-arm `POLLIN`).
- **otherwise** → mark `DEAD`.

### Error & redirect shortcuts
- `queue_error(code)` — serves a configured `error_page` if present, otherwise
  generates a minimal HTML error page; then goes straight to `WRITING_RESPONSE`.
- `queue_redirect(location, code)` — sets `Location`, empty body, and writes.

---

## 12. Cleanup paths

| Function | When | What it does |
|----------|------|--------------|
| `reset_client()` | keep-alive between requests | close/unlink body temp file, reset `request`/`response`, state → `READING_HEADERS` |
| `cleaning_client()` | client marked `DEAD` | drain socket, close body fd + unlink temp file, `close(fd)`, remove from `listeners` and `clients` |
| `cleaning_cgi()` | CGI pipe done/errored | `close(fd)` and remove it from `listeners` |
| `print_error_exit()` | fatal startup error | close all listeners, `exit(1)` |

---

## 13. Request lifecycle — end to end

```
accept ─► READING_HEADERS ─► parse_request ─► choose_server
                                  │
                    POST ─────────┤───────── GET/DELETE
                    │             │              │
              READING_BODY        │              │
             (file + max_body)    │              │
                    │             ▼              │
                    └────────► ROUTING ◄─────────┘
                                  │
             ┌───────────┬────────┼─────────┬─────────────┐
             ▼           ▼        ▼         ▼             ▼
           301 dir    404/405   return    CGI         GET/POST/DELETE
                                          │                │
                                    WAITING_CGI      build_response
                                          │                │
                                    handle_cgi_read         │
                                          │                │
                                          └──► WRITING_RESPONSE
                                                     │
                                        keep-alive? ─┴─ no ─► DEAD ─► cleaning_client
                                             │
                                             └─ yes ─► reset_client ─► READING_HEADERS
```

---

## 14. Design principles worth stating out loud

- **Never block.** Every socket is `O_NONBLOCK`; `EAGAIN`/`EWOULDBLOCK` is a
  "come back later", not an error.
- **`poll` owns the schedule.** The code only *reacts* to readiness; it never
  polls in a busy loop or sleeps.
- **State lives on the connection.** Because a single `recv` may deliver a
  fraction of a request (or several requests), all progress is stored in the
  `t_client` and resumed on the next event.
- **One serialization point.** Everything a client sends goes through
  `build_response()` → `outbuf`, and everything is written by the single
  `handle_client_write()` with a `bytes_sent` cursor.
- **Maps as a type system.** The fd's map membership *is* its type; the loop
  needs no per-fd tag field.

---

## 15. Known fragile spots / things to be ready to explain

These are real inconsistencies in the current code — none of them are style
nitpicks. Knowing them cold is the difference between a clean defense and a
surprise.

1. **`keep_alive` is never set to `true`.** `t_client` initializes it to `false`
   and nothing in `server.cpp` flips it, so `build_response()` always emits
   `Connection: close` and the keep-alive branch in `handle_client_write()` is
   effectively dead. If keep-alive is a requirement, it needs to be set from the
   request (`Connection` header / HTTP version) during parsing.

2. **The POST→CGI stdin pipe (`std_in`) is vestigial and leaks.** `start_cgi()`
   creates `pipe(std_in)` for POST, but the child reads the body from the temp
   **file** (`dup2(file_fd, 0)`), and the parent never registers or closes
   `std_in[0]`/`std_in[1]`. So `handle_cgi_write()` / `cgi.in_fd` are never
   actually used for that path, and two fds leak per POST-CGI request. Either
   wire the pipe up or drop it and close it.

3. **`handle_delete()` uses `root_path.substr(1)`** while `handle_get()` and
   `handle_post()` use `root_path` directly. That single stripped leading
   character makes DELETE resolve paths differently from GET/POST — likely a bug
   unless there's a deliberate reason.

4. **The directory-redirect check in `route()` uses `server->root_path`**, but
   the actual file handlers use `location->root_path`. If a location overrides
   root, the 301-trailing-slash decision is computed against the wrong directory.

5. **`cleaning_cgi()` erases from `listeners` mid-sweep.** When a CGI hits EOF
   inside the `for` loop, the erase shifts later elements left by one for the
   rest of that poll cycle (the loop indexes by position). It self-corrects on
   the next `poll()`, but it's the kind of thing an evaluator will poke at.

6. **`waitpid(..., WNOHANG)` without a loop** can leave a zombie if the child
   hasn't exited at the exact moment both pipes are closed. Consider reaping
   until it actually returns the pid.

> None of these block the happy path — the server serves files, runs CGI, and
> handles uploads. They're the edges to tighten and the questions to have
> answers for.

---

*Generated from a read of `server.cpp`, `server.hpp`, `webserv.h`, and
`main.cpp`. Parsing internals (`ft_method`, `ft_headers`, `ft_cgi`,
`ft_parce_config`) live under `parce/` and `utils/` and are only referenced
here.*
