#include "Multisensor.hpp"
#include "Hensor.hpp"

static const char * HENSOR_TAG = "Hensor";

Multisensor::Multisensor(const char * name, int taskCore) : Sensor(name, taskCore) {
}

void Multisensor::connect(void * data) {
	this->sensor = new Adafruit_BME680(static_cast<TwoWire*>(data));

	while (--this->remainingAttempts && !this->connectedStatus) {
		this->connectedStatus = this->sensor->begin();
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}

void Multisensor::run(void* data) {
	Hensor * hensor = Hensor::getInstance();

	if (hensor->isProductionMode()) {
		this->testReset();
	}

	while (1) {
		vTaskDelay(this->iterationDelay);

		if (! this->sensor->performReading()) {
			Serial.print("Failed to perform reading\n");
			continue;
		}

		ESP_LOGI(HENSOR_TAG, "Temperature(C): %.2f", hensor->FunctionTemperatureCalibrated(this->sensor->temperature));
		ESP_LOGI(HENSOR_TAG, "Humidity(C): %.2f", hensor->FunctionHumidityCalibrated(this->sensor->humidity));
		ESP_LOGI(HENSOR_TAG, "Pressure(C): %.2f", hensor->FunctionPressureCalibrated(this->sensor->pressure));

		hensor->holdTemperatureValue(hensor->FunctionTemperatureCalibrated(this->sensor->temperature));
		//hensor->holdTemperatureValue(this->sensor->temperature);

		//hensor->holdHumidityValue(hensor->FunctionHumidityCalibrated(this->sensor->humidity));
		hensor->holdHumidityValue(hensor->FunctionHumidityCalibrated(this->sensor->humidity));

		hensor->holdPressureValue(hensor->FunctionPressureCalibrated(this->sensor->pressure));
	}

	this->stop();
}
