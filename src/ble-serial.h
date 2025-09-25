#ifndef BLE_SERIAL_H_
#define BLE_SERIAL_H_

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <console.h>

#define BLE_T				"BLE"

#define BLE_TX_BUFFER_SIZE	320

// List of Service and Characteristic UUIDs
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define RX_DATA_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TX_DATA_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

typedef void (*BleReceiveCallback)(const char* data, int size);
typedef void (*BleConnectionCallback)(ble_gap_conn_desc *desc);

class BleSerial {
	private:
		const char * _device_name;
		uint8_t _mac[8];
		char _mac_str[18];
		bool _enabled;
		NimBLEServer* _server;
		NimBLECharacteristic* _rxCharacteristic;
		NimBLECharacteristic* _txCharacteristic;
		//void (*_onReceive)(const char *data, int data_size);
		BleReceiveCallback _onReceive;
		BleConnectionCallback _onConnect;
		BleConnectionCallback _onDisconnect;
		class ServerCallbacks : public NimBLEServerCallbacks {
			public:
				ServerCallbacks(BleSerial* parent);
				void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override;
				void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override;
			private:
				BleSerial* _parent;
		};
		class RxCallback : public NimBLECharacteristicCallbacks {
			public:
				RxCallback(BleSerial* parent);
				void onWrite(NimBLECharacteristic* characteristic) override;
			private:
				BleSerial* _parent;
		};
	public:
		BleSerial(const char* deviceName);
		void begin(void);
		void end(void);
		String getMacAddress(void);
		String getMacAddress(uint8_t * address);
		void onReceive(BleReceiveCallback callback);
		void onConnect(BleConnectionCallback callback);
		void onDisconnect(BleConnectionCallback callback);
		bool send(uint8_t *data, int size);
		bool send(String data);
};

#endif  /* BLE_SERIAL_H_ */