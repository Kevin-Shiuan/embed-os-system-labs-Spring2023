# HW2 Socket Programming and Data Visualization

## for STM32CubeIDE

1.  ### open the project in STM32CubeIDE

    file -> open project from file systems -> choose the project folder

    ./HW2/STM32CubeL4/Projects/B-L475E-IOT01A/Applications/WiFi/WiFi_Client_Server/SW4STM32/B-L475E-IOT01

2.  ### modify project files

    #### i. at ./HW2/STM32CubeL4/Projects/B-L475E-IOT01A/Applications/WiFi/WiFi_Client_Server/Inc/main.h, add the following lines

```cpp
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_qspi.h"
```

**Note:** you should include the libraries of the sensors. We included the path of library in our project, you may need to add the path yourself, you can refer to this [document](https://wiki.st.com/stm32mcu/wiki/STM32StepByStep:Step4_Sensors_usage)

#### ii. at ./HW2/STM32CubeL4/Projects/B-L475E-IOT01A/Applications/WiFi/WiFi_Client_Server/INC/stm32l4xx_hal_conf.h, uncomment line 62

```cpp
#define HAL_QSPI_MODULE_ENABLED
```

3.  ### edit main.c

    Open ./Application/User/main.c, configure the wifi network name & password, IP address & port of the TCP server

```cpp
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_qspi.h"

// WiFi network name and password
#define SSID_NAME "<your wifi name>"
#define SSID_PSWD "<your wifi password>"

// IP address and port of the TCP server
uint8_t RemoteIP[] = {192,168, xxx, yyy};
#define RemotePORT <port>
```

4. ### build and run

   Build the project and run it on the board

5. ### set up the TCP server

    Our TCP server is coded in python, you can find the code in TCPserver.py. Before running the server, open the file and modify the following lines

    ```python
    HOST = '<IP>'  # IP address
    PORT = 65431  # Port to listen on (use ports > 1023)
    DATA_SIZE = 50 # number of data to be displayed in the graph simultaneously, you can change it to any number you want
    ```
    
    To run the server in desktop, you can use the following command in the terminal

    ```bash
    python3 TCPserver.py 
    ```

6. ### connection between the board and the TCP server

   The board will try to connect to the TCP server. Status of the board will be shown on the serial monitor, which is **Command Shell Console** of STM32CubeIDE. the baud rate is _115200_.

7. ### communication between the board and the TCP server

   The board will send the data to the TCP server every time server is requesting (which is 200ms in our code ). The raw data is send in json format, which is easy to parse in python. The data is in the following format.

   ```json
   {
       "acce": {
           "x": 0,
           "y": 0,
           "z": 0,
       },
       "gyro": {
           "x": 0.0,
           "y": 0.0,
           "z": 0.0,
       }
   }
   ```

### demo video
https://youtu.be/q4nFjyXjW88

---

## for mbed studio

1. ### create new project

start with example program mbed-os-example-sockets and import bsp library into project from http://developer.mbed.org/teams/ST/code/BSP_B-L475E-IOT01/

2. ### modify project files

replace source/main.cpp and mbed_app.json and edit parameters accordingly if needed (ssid, password, ip address, port number etc.)

3. ### build and run project in mbed studio

4. ### compile and run server.c on laptop to log data

data will be tab delimited and written to the file 'log.dat' in the current directory

5. ### run myplot.m with octave (an open source matlab alternative)

a pop-up window should appear showing a figure containing plots of 'log.dat' updated in real-time
