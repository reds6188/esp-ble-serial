#include "ble-serial.h"

BleSerial::BleSerial(const char* deviceName = "ESP32-BLE") :
	_device_name(deviceName),
	_enabled(false),
	_server(nullptr),
	_rxCharacteristic(nullptr),
	_txCharacteristic(nullptr),
	_onReceive(nullptr),
	_onConnect(nullptr),
	_onDisconnect(nullptr) {

	_mac_str[0] = '\0';
}

void BleSerial::begin(void) {
	NimBLEDevice::init(_device_name);

	// Getting MAC address
	NimBLEAddress addr = NimBLEDevice::getAddress();
	const uint8_t * addr_p = addr.getNative();
	for(uint8_t i=0 ; i<8 ; i++)
		_mac[i] = *(addr_p + i);
	snprintf(_mac_str, sizeof(_mac_str), "%s", addr.toString().c_str());

	// Create Server and set connection/disconnection callbacks
	_server = NimBLEDevice::createServer();
	_server->setCallbacks(new ServerCallbacks(this));

	// Crete service for TX and RX characteristics
	NimBLEService *service = _server->createService(SERVICE_UUID);

	// Create RX Characteristic
	_rxCharacteristic = service->createCharacteristic(
		RX_DATA_UUID,
		NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
	);
	_rxCharacteristic->setCallbacks(new RxCallback(this));

	// Create TX Characteristic
	_txCharacteristic = service->createCharacteristic(
		TX_DATA_UUID,
		NIMBLE_PROPERTY::NOTIFY
	);

	// Start service
	service->start();

	// Start advertising
	NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
	advertising->addServiceUUID(service->getUUID());
	advertising->start();

	_enabled = true;
}

void BleSerial::end(void) {
	NimBLEDevice::deinit(true);
	_enabled = false;
}

String BleSerial::getMacAddress(void) {
	return String(_mac_str);
}

String BleSerial::getMacAddress(uint8_t * address) {
	char mac_str[18];
	snprintf(mac_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x", address[5], address[4], address[3], address[2], address[1], address[0]);
	return String(mac_str);
}

void BleSerial::onReceive(BleReceiveCallback callback) {
	_onReceive = callback;
}

void BleSerial::onConnect(BleConnectionCallback callback) {
	_onConnect = callback;
}

void BleSerial::onDisconnect(BleConnectionCallback callback) {
	_onDisconnect = callback;
}

bool BleSerial::send(uint8_t *data, int size) {
	if(!_enabled) {
		console.warning(BLE_T, "BLE is not enabled");
		return false;
	}

	_txCharacteristic->setValue(data, size);
	_txCharacteristic->notify();

	return true;
}

bool BleSerial::send(String data) {
	return send((uint8_t *)data.c_str(), data.length());
}

BleSerial::ServerCallbacks::ServerCallbacks(BleSerial* parent) : _parent(parent) {}

void BleSerial::ServerCallbacks::onConnect(NimBLEServer *server, ble_gap_conn_desc *desc) {
	// Print client MAC address
	String bleRemoteAddress = String(_parent->getMacAddress(desc->peer_id_addr.val));
	console.success(BLE_T, "BLE client connected, remote address is " + bleRemoteAddress);

	// Get Connection ID
	uint16_t id = server->getPeerDevices()[0];
	console.info(BLE_T, "Connection ID = " + String(id));

	// Stop advertising 
	NimBLEDevice::stopAdvertising();

	if(_parent->_onConnect)
		_parent->_onConnect(desc);
}

void BleSerial::ServerCallbacks::onDisconnect(NimBLEServer *server, ble_gap_conn_desc *desc) {
	if(_parent->_onDisconnect)
		_parent->_onDisconnect(desc);
}

BleSerial::RxCallback::RxCallback(BleSerial* parent) : _parent(parent) {}

void BleSerial::RxCallback::onWrite(NimBLECharacteristic* characteristic) {
    std::string value = characteristic->getValue();
    if (_parent->_onReceive) {
        _parent->_onReceive(value.data(), value.size());
    }
}