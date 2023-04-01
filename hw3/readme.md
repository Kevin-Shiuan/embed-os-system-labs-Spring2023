# this program was tested on a laptop running wsl2 debian in windows

1. ## change name to 'Pixel 7' for gatt server program running on phone or modify ble.c accordingly

2. ## compile and run

It was noted that the library gattlib was created by examining the source code of the tools that come with bluez.
In this case, we make use of the tools bluetoothctl and gatttool directy to acheive our goals.
First we scan for the mac address of the phone running a gatt server and then
proceed to connect using bluetoothctl. Afterwards, we simply proceed to writing the required value to the CCCD.

3. ## verify that a write request for the CCCD value is sent to the gatt server program
