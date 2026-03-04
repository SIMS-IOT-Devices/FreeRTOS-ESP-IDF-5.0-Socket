# Send UDP message from PC to ESP32 server

import socket

# Target IP and port
UDP_IP = "192.168.1.153"   # change to receiver IP
UDP_PORT = 3333

message = b"Hello from Python via UDP"

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Send data
sock.sendto(message, (UDP_IP, UDP_PORT))

print("Message sent")