import socket, time
s = socket.socket()
s.connect(('127.0.0.1', 8080))
s.sendall(b"GET /cgi-bin/hello.py HTTP/1.1\r\nHost: localhost\r\n\r\n")
print(s.recv(4096))
