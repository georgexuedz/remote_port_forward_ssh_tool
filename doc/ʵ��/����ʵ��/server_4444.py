#!/usr/bin/python
import socket   
import time
import commands 

HOST='localhost'
PORT=4444
s= socket.socket(socket.AF_INET,socket.SOCK_STREAM) 
s.bind((HOST,PORT))   
s.listen(1)         
while True:
	conn,addr=s.accept()   
	print'Connected by',addr    
	while 1:
		data = 'i am 4444, server'
		conn.sendall(data)    
		print 'send-> ' + data + '\n'
		time.sleep(1)

conn.close() 

