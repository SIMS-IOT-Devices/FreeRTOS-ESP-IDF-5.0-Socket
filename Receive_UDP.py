# Receive UDP message from ESP32 server

import socket

UDP_IP = "0.0.0.0"
UDP_PORT = 3333

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print("Listening for UDP packets...")

while True:
    data, addr = sock.recvfrom(1024)
    print(f"From {addr}: {data.decode()}")