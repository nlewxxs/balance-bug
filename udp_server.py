import socket, time

UDP_IP = "192.168.203.180"
UDP_PORT = 2390
MESSAGE = b"Hello, World!"

time.sleep(10)
print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)
print("message: %s" % MESSAGE)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
