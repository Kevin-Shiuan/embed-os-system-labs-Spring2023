#include <stdio.h>

int main() {
	int result;
	char ch, mac[21], cmd[200];
	FILE *p;
	
	p = popen("bluetoothctl --timeout 5 scan on | awk '/Pixel 7/{print $3}'", "r");
	if(p == NULL) {
		printf("unable to open process 'bluetoothctl'\n");
		return 1;
	}
	result = fscanf(p, "%20s", mac);
	pclose(p);
	if(result <= 0) {
		printf("scan mac address failed\n");
		return 1;
	}

	// ble scan mac address read success
	printf("mac address: %s\n", mac);
	sprintf(cmd, "gatttool --device='%s' --addr-type=random --char-write-req --handle=0x008e --value=0002", mac);
	printf("running: %s\n", cmd);

	p = popen(cmd, "r");
	if(p == NULL) {
		printf("unable to open process 'gatttool\n");
		return 1;
	}
	pclose(p);

	return 0;
}
