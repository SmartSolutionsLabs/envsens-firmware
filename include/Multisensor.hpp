#ifndef INC_MULTISENSOR
#define INC_MULTISENSOR

#include "Sensor.hpp"

class Multisensor : public Sensor {
	public:
		Multisensor(const char * name, int taskCore = 0);

		void connect(void * data);

		void run(void* data);
};

#endif
