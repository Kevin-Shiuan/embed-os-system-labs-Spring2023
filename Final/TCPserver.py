#!/usr/bin/env python3
import socket
import json

HOST = '192.168.10.196'  # IP address
PORT = 65431  # Port to listen on (use ports > 1023)
DATA_SIZE = 50  # number of data points to display

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print('Server is listening...')
    desired_temperature = 26.0
    fan_mode = 0
    while True:
        conn, addr = s.accept()
        with conn:
            # print('Connected by', addr)
            message = f"{{\"desired_temperature\":\"{desired_temperature}\",\"fanMode\":\"{fan_mode}\",}}"
            conn.sendall(message.encode('utf-8'))
            data = conn.recv(1024).decode('utf-8')
            json_data = json.loads(data)
            print('temperature : ', json_data["temperature"])
            desired_temperature = input("Enter desired_temperature(current=): ")
            fan_mode = input("Enter fanMode value(0 for auto): ")
