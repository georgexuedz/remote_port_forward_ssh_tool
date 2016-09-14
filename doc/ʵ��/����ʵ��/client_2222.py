import socket
HOST = 'localhost'
PORT = 2222
s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
s.connect((HOST, PORT))
while True:
	data = s.recv(1024)
	print 'receive data -> ' + data + '\n'
	#cmd = raw_input('input:') s.sendall(cmd)
s.close()

