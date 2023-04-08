#include <stdio.h>

int main() {
	int i, result;
	char ch, mac[21], cmd[1000], output[300], handles[4][7];
	FILE *p;
	
	p = popen("bluetoothctl --timeout 5 scan on | awk '/Heartrate/{print $3}'", "r");
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
	// obtain cccd handles
	printf("mac address: %s\n", mac);
	sprintf(cmd, "gatttool --device='%s' --addr-type=random --char-read --uuid=0x2902 | awk '/^handle:/{print $2}'", mac);
	printf("running: %s\n", cmd);

	p = popen(cmd, "r");
	if(p == NULL) {
		printf("unable to open process 'gatttool\n");
		return 1;
	}

	for(i = 0; i < 4 && result > 0; i++) {
		result = fscanf(p, "%6s", handles[i]);
	}

	pclose(p);

	if(result <= 0) {
		printf("scan cccd handles failed\n");
		return 1;
	}

	// scan cccd handles succeeded
	// write cccd's to enable notification and listen
	for(i = 0; i < 4; i++) {
		printf("cccd handle %d: %s\n", i, handles[i]);
	}
	sprintf(cmd,
		"gatttool --device='%s' --addr-type=random --char-write-req --handle='%s' --value=0100 && \n"
		"gatttool --device='%s' --addr-type=random --char-write-req --handle='%s' --value=0100 && \n"
		"gatttool --device='%s' --addr-type=random --char-write-req --handle='%s' --value=0100 && \n"
		"gatttool --device='%s' --addr-type=random --char-write-req --handle='%s' --value=0100 --listen",
		mac, handles[0], mac, handles[1], mac, handles[2], mac, handles[3]);
	printf("running:\n%s\n", cmd);

	p = popen(cmd, "r");
	if(p == NULL) {
		printf("unable to open process 'gatttool\n");
		return 1;
	}

	while(1) {
		fgets(output, 200, p);
		printf("%s", output);
	}

	pclose(p);

	return 0;
}
