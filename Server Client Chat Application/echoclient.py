import socket

s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
host=socket.gethostname()
port=12345
print("My details. Host : {}, IP: {}".format(host,socket.gethostbyname(host)))
s.connect((host,port))


# this section of code for simple echo server

#s.sendall('Hi, this is me '.encode('ascii'))
#data=s.recv(1024)
#print("data received: {}".format(data))
#s.sendall(b' how are you? ')
#data1=s.recv(1024)
#print("data received: " + str(data1))
#s.close()

#this section of code for one to one chat server

while True:
	ip=input("Your message :")
	s.sendall(ip.encode('ascii'))
	data=s.recv(1024)
	print("data received: {}".format(data))
	if data==b'bye' or data==b'Bye' or data==b'BYE':  
		print("exiting the loop and terminating connection")
		break
	
s.close()