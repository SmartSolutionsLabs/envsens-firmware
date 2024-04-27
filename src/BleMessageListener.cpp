#include "BleMessageListener.hpp"
#include "Ble.hpp"

BleMessageListener::BleMessageListener() {
}

void BleMessageListener::onWrite(BLECharacteristic * characteristic) {
	String input(characteristic->getValue().c_str());
	if(input.length() > 0){
		Serial.print("\tBleListened\n\t");
		Serial.println(input);
	}
}

void BleMessageListener::writeLargeText(BLECharacteristic * characteristic, std::string largeText) {
	for(int i = 0; i < largeText.length(); i += MTU_SIZE - 3) {
		int len = MTU_SIZE - 3;
		if(len > largeText.length() - i) {
			len = largeText.length() - i;
		}

		characteristic->setValue(largeText.substr(i, len));
		characteristic->notify();
	}

	characteristic->setValue("|");
	characteristic->notify();
}
