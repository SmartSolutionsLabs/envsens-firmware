#ifndef INC_MULTISENSOR
#define INC_MULTISENSOR

#include "Sensor.hpp"

#include <Adafruit_BME680.h>

class Multisensor : public Sensor {
	protected:
		Adafruit_BME680 * sensor = nullptr;

	public:
		Multisensor(const char * name, int taskCore = 0);

		void connect(void * data) override;

		void run(void* data) override;
};

#endif
