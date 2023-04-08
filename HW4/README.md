# HW2 Mbed BLE programming - Peripheral in mbed-os (stm32) and Central in Python (RPi)

1.  ### open the project in mbed studio

    file -> open workspace -> choose the root folder of this project, which is HW4

1.  ### modify project files

    fix the library if needed, esle you can skip this step

1.  ### run the embed program on stm32
    build, upload and run the program on the board
    watch the serial monitor to see the DEVICE MAC ADDRESS of the board, we will need it in the next step

1. ### modify code of python program

    open the file `magneto.py` and modify the following lines

    ```python
    # change the address to the address of your board
    MAC_ADDRESS = "xx:xx:xx:xx:xx:xx" # change this to the DEVICE MAC ADDRESS of your board that you got from the serial monitor in the previous step
    ```

1. ### start the python program in RPi
    > you can transfer the file to RPi with any method you like, such as scp, sftp, etc.

    ```bash
    sudo python3 magneto.py
    ```
    **Note:** you need to use `sudo` to run the program

    **Note:** you need to set up the bluetooth functionality in RPi first

1. ### select services and characteristics
    enter the uuid of services that shown  on the terminal
    > you should select the uuid of service *uuid=Heart Rate handleStart=11 handleEnd=65535*
    
    then enter the uuid of characteristics that shown on the terminal
    > you should select the uuid of characteristics *Heart Rate Measurement*
    
    ```bash
    Connecting to #fe:20:bc:2d:a2:84...
    Available Services:
    Service 00001800-0000-1000-8000-00805f9b34fb Service <uuid=Generic Access handleStart=1 handleEnd=7>
    Service 00001801-0000-1000-8000-00805f9b34fb Service <uuid=Generic Attribute handleStart=8 handleEnd=10>
    Service 0000180d-0000-1000-8000-00805f9b34fb Service <uuid=Heart Rate handleStart=11 handleEnd=65535>
    service uuid: 0000180d-0000-1000-8000-00805f9b34fb #enter uuid here
    0000180d-0000-1000-8000-00805f9b34fb

    Available Characteristics:
    00002a37-0000-1000-8000-00805f9b34fb Characteristic <Heart Rate Measurement>
    00002a38-0000-1000-8000-00805f9b34fb Characteristic <Body Sensor Location>
    char uuid: 00002a37-0000-1000-8000-00805f9b34fb #enter uuid here
    ```

1. ### connected
    you should see the following message on the terminal
    ```bash
    notified data: xxx
    notified data: xxx
    notified data: xxx
    ...
    ```
    the data is the measurement of magnetometer in x, y, z axis, the data is updated every 1 second, and is in the order of x, y, z.

### demo video
https://youtu.be/hdbEdIsr6-M
