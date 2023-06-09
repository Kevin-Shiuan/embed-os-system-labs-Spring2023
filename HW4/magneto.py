from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate
import struct

# change the address to the address of your board
MAC_ADDRESS = "fe:20:bc:2d:a2:84"

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device", dev.addr)
        elif isNewData:
            print("Received new data from", dev.addr)


class MyDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        print(f'notified data: {int(data.hex(),16)}')
        return

print(f"Connecting to #{MAC_ADDRESS}...")
dev = Peripheral(MAC_ADDRESS, 'random')
dev.setDelegate(MyDelegate())

services = dev.getServices()

print("Available Services:")
for svc in services:    
    print("Service", str(svc.uuid), svc)

try:
    target_svc = input("service uuid: ")
    print(target_svc)
    testService = dev.getServiceByUUID(target_svc)
    print()
    print("Available Characteristics:")
    for ch in testService.getCharacteristics():
        print(ch.uuid, str(ch))
    target_char = input("char uuid: ")
    ch = dev.getCharacteristics(uuid=UUID(target_char))[0]
    ch_handle = ch.getHandle()
    ch_notify_handle = ch_handle + 1

    while True:
        dev.writeCharacteristic(
            ch_notify_handle, struct.pack('<bb', 0x01, 0x00))
        dev.waitForNotifications(10)

finally:
    dev.disconnect()
