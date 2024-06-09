#pragma once

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

bool isBluetoothAdapterOn() {
	int hciSocket = hci_open_dev(hci_get_route(NULL));
	if (hciSocket < 0) {
		return 0;
	}
	int devCount, numOn;
	struct hci_dev_list_req *devList;
	struct hci_dev_info devInfo;
	devList = (struct hci_dev_list_req *)malloc(
		HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(uint16_t)
	);
	if (!devList) {
		close(hciSocket);
		return 0;
	}
	devList->dev_num = HCI_MAX_DEV;
	if (ioctl(hciSocket, HCIGETDEVLIST, (void *)devList) < 0) {
		perror("Failed to get HCI device list");
		free(devList);
		close(hciSocket);
		return 0;
	}
	devCount = devList->dev_num;
	// printf("got %d devices\n", devCount);
	for (int i = 0; i < devCount; i++) {
		// Set the device ID for device info retrieval
		devInfo.dev_id = devList->dev_req[i].dev_id;
		if (ioctl(hciSocket, HCIGETDEVINFO, (void *)&devInfo) < 0) {
			// perror("Failed to get HCI device info");
			continue;
		}
		numOn++;
		// char addr[18];
		// ba2str(&devInfo.bdaddr, addr);
		//  Print device information
		//  std::cout << "HCI Device " << i + 1 << ":" << std::endl;
		//  std::cout << "  Address: " << addr << std::endl;
		//  std::cout << "  Dev ID: " << devInfo.dev_id << std::endl;
		//  std::cout << "  Flags: 0x" << std::hex << devInfo.flags << std::dec
		//		  << std::endl;
		//  std::cout << "  Type: 0x" << std::hex << devInfo.type << std::dec
		//		  << std::endl;
		//  std::cout << "  Name: " << devInfo.name << std::endl;
	}
	free(devList);
	close(hciSocket);
	return numOn;
}
