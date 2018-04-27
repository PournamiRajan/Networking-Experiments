import socket

s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
host=socket.gethostname()
port=12345
print("My details. Host : {}, IP: {}".format(host,socket.gethostbyname(host)))
s.bind((host,port))
s.listen(1)

print("waiting for connection request")
c,addr=s.accept()
print("Connection established with {} -- {}".format(addr,c))

# this section of code for simple echo server

#while True:

	#data = c.recv(1024)
	#if not data: break
	#c.sendall(data)
	#print("data received: {}".format(data))
#c.close()

#this section of code for one to one chat server

while True:
	data=c.recv(1024)
	print(str(addr)+": {}".format(data))
	ip=input("your reply :")
	c.sendall(ip.encode('ascii'))
	if data==b'bye' or data==b'Bye' or data==b'BYE': 
		print("exiting the loop and terminating connection")
		break
	
c.close()	
	
	

	
	

