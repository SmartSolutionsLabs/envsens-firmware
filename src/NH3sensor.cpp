#include "NH3sensor.hpp"

NH3sensor::NH3sensor(const char * name, int taskCore) : Sensor(name, taskCore) {
}

void NH3sensor::connect(void * data) {
	this->sensor = new Adafruit_ADS1115();

	this->connectedStatus = this->sensor->begin(ADS1X15_ADDRESS, static_cast<TwoWire*>(data));
}

void NH3sensor::run(void* data) {
	if( !this->connectedStatus ) {
		this->stop();
	}

	int16_t channelData = 0;
	float voltage;

	while (1) {
		vTaskDelay(this->iterationDelay);

		channelData = this->sensor->readADC_SingleEnded(0);
		voltage = this->sensor->computeVolts(channelData);

		Serial.print("Voltage: ");
		Serial.println(voltage);
	}

	this->stop();
}
