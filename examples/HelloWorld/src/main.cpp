#include <ble-serial.h>

#define MAIN_T	"MAIN"

String str = "Hello World\n";

unsigned long last = 0;

BleSerial ble("MyESP32");

void callback(const char *data, int size) {
	console.log(MAIN_T, "Received: " + String(data));
}

void on_connect(ble_gap_conn_desc *desc) {
	console.success(MAIN_T, "Connected!");
}

void on_disconnect(ble_gap_conn_desc *desc) {
	console.warning(MAIN_T, "Disconnected!");
}

void setup(void)
{
	Serial.begin(115200);
	ble.begin();
	ble.onReceive(callback);
	ble.onConnect(on_connect);
	ble.onDisconnect(on_disconnect);
	console.log(MAIN_T, "Local MAC address: " + String(ble.getMacAddress()));
}

void loop(void)
{
	if (millis() - last > 5000) {
		ble.send(str);
		last = millis();
	}
}