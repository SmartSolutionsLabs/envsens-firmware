#include "Multisensor.hpp"
#include "Hensor.hpp"

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

	Hensor * hensor = Hensor::getInstance();

	while (1) {
		vTaskDelay(this->iterationDelay);

		if (! this->sensor->performReading()) {
			Serial.print("Failed to perform reading\n");
			continue;
		}

		ESP_LOGI(HENSOR_TAG, "Temperature(C): %.2f", this->sensor->temperature);
		ESP_LOGI(HENSOR_TAG, "Humidity(C): %.2f", this->sensor->humidity);
		ESP_LOGI(HENSOR_TAG, "Pressure(C): %d", this->sensor->pressure);

		hensor->holdTemperatureValue(hensor->FunctionTemperatureCalibrated(this->sensor->temperature));
		//hensor->holdTemperatureValue(this->sensor->temperature);

		//hensor->holdHumidityValue(hensor->FunctionHumidityCalibrated(this->sensor->humidity));
		hensor->holdHumidityValue(this->sensor->humidity);

		hensor->holdPressureValue(this->sensor->pressure * hensor->getPressureMultiplier());
	}

	this->stop();
}
