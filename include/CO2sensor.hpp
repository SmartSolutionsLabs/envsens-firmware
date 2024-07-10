#ifndef INC_CO2SENSOR
#define INC_CO2SENSOR

#include "Sensor.hpp"

#include <SensirionI2CScd4x.h>

class CO2sensor : public Sensor {
	protected:
		SensirionI2CScd4x * sensor = nullptr;

	public:
		CO2sensor(const char * name, int taskCore = 1);

		void connect(void * data);

		void run(void* data);
};

#endif
