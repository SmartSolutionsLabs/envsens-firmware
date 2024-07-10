#include "Multisensor.hpp"

Multisensor::Multisensor(const char * name, int taskCore) : Sensor(name, taskCore) {
}

void Multisensor::connect(void * data) {
	this->sensor = new Adafruit_BME680(static_cast<TwoWire*>(data));

	this->connectedStatus = this->sensor->begin();
}

void Multisensor::run(void* data) {
	TickType_t xDelayIteration = 1000 / portTICK_PERIOD_MS; // Iteration speed

	while (1) {
		vTaskDelay(xDelayIteration);

		if (! this->sensor->performReading()) {
			Serial.print("Failed to perform reading\n");
			continue;
		}

		Serial.print("Temperature: ");
		Serial.println(this->sensor->temperature);
	}
}
