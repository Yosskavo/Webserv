import socket, time
s = socket.socket()
s.connect(('127.0.0.1', 8080))
s.sendall(b"POST /cgi-bin/hello.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1000000\r\n\r\n")
time.sleep(0.5)
s.sendall(b"A" * 10000)
time.sleep(2)
