#ifndef INC_NH3SENSOR
#define INC_NH3SENSOR

#include "Sensor.hpp"

#include <Adafruit_ADS1X15.h>

class NH3sensor : public Sensor {
	protected:
		Adafruit_ADS1115 * sensor = nullptr;

	public:
		NH3sensor(const char * name, int taskCore = 1);

		void connect(void * data) override;

		void run(void* data) override;

		uint32_t  getPPM(float voltage);
	private:
		int16_t channelData;

		float voltage;

		uint32_t R0;

		float maxVoltage;

		uint32_t RS;

		float SR;

		uint32_t nh3_in_ppm;

		float nh3_read_value;
};

#endif
