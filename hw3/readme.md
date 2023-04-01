It was noted that the library gattlib was created by examining the source code of the tools that come with bluez.
In this case, we make use of the tools bluetoothctl and gatttool directy to acheive our goals.
First we scan for the mac address of the phone running a gatt server and then
proceed to connect using bluetoothctl. Afterwards, we simply proceed to writing the required value to the CCCD.
