#!/usr/bin/env python3
import socket
import numpy as np
import json
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
# from matplotlib import style


HOST = '<IP>'  # IP address
PORT = 65431  # Port to listen on (use ports > 1023)
DATA_SIZE = 50 # number of data to be displayed in the graph simultaneously, you can change it to any number you want

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print('Server is listening...')
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)

        # set up the figure
        fig = plt.figure(figsize=plt.figaspect(0.5))

        # create 3D subplots for gyro and accelerometer data
        ax_gyro = fig.add_subplot(1, 2, 1, projection='3d')
        ax_acce = fig.add_subplot(1, 2, 2, projection='3d')

        # initialize empty lists for storing data
        gyro_x = [0]
        gyro_y = [0]
        gyro_z = [0]
        acce_x = [0]
        acce_y = [0]
        acce_z = [0]

        # define the animation function
        def animate(frame):
            message = "server requesting data"
            conn.sendall(message.encode('utf-8'))
            data = conn.recv(1024).decode('utf-8')
            print('Received from socket server : ', data)
            # print data
            json_data = json.loads(data)
            # json_formatted_str = json.dumps(json_data, indent=2)
            # print(json_formatted_str)
            # append data to list
            gyro_x.append(json_data["Gyro"]["x"])
            gyro_y.append(json_data["Gyro"]["y"])
            gyro_z.append(json_data["Gyro"]["z"])
            acce_x.append(json_data["Acce"]["x"])
            acce_y.append(json_data["Acce"]["y"])
            acce_z.append(json_data["Acce"]["z"])

            # Limit the data lists to a maximum length of 100
            if len(gyro_x) > DATA_SIZE:
                gyro_x.pop(0)
                gyro_y.pop(0)
                gyro_z.pop(0)
                acce_x.pop(0)
                acce_y.pop(0)
                acce_z.pop(0)
            # time.sleep(3)
            ax_gyro.clear()
            ax_gyro.plot(gyro_x, gyro_y, gyro_z, label="Gyro", color="blue")
            ax_gyro.legend()
            ax_acce.clear()
            ax_acce.plot(acce_x, acce_y, acce_z, label="Acce", color="red")
            ax_acce.legend()

        # create a FuncAnimation object and display the plot
        # ani = animation.FuncAnimation(fig, animate, frames=range(0, 100), interval=100, cache_frame_data=False)
        ani = animation.FuncAnimation(fig, animate, frames=range(0, DATA_SIZE), interval=200)
        plt.show()