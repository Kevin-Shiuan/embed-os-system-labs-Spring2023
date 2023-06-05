#!/usr/bin/env python3
import socket
import numpy as np
import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation

HOST = '192.168.10.196'  # IP address
PORT = 65431  # Port to listen on (use ports > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print('Server is listening...')
    while True:
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)
            message = "{\"name\":\"John\",\"age\":30,\"city\":\"New York\",\"fanMode\":\"2\",}"
            conn.sendall(message.encode('utf-8'))
            data = conn.recv(1024).decode('utf-8')
            print('Received from socket server : ', data)
