#include "Multisensor.hpp"

static const char * HENSOR_TAG = "Hensor";

Multisensor::Multisensor(const char * name, int taskCore) : Sensor(name, taskCore) {
}

void Multisensor::connect(void * data) {
	this->sensor = new Adafruit_BME680(static_cast<TwoWire*>(data));

	this->connectedStatus = this->sensor->begin();
}

void Multisensor::run(void* data) {
	if( !this->connectedStatus ) {
		this->stop();
	}

	while (1) {
		vTaskDelay(this->iterationDelay);

		if (! this->sensor->performReading()) {
			Serial.print("Failed to perform reading\n");
			continue;
		}

		ESP_LOGI(HENSOR_TAG, "Temperature(C): %.2f", this->sensor->temperature);
	}

	this->stop();
}
