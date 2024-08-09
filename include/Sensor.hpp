#ifndef INC_SENSOR
#define INC_SENSOR

#include "Thread.hpp"

/**
 * Controlling hardware for input of gas.
 */
class Sensor : public Thread {
	protected:
		bool connectedStatus;

		unsigned int remainingAttempts = 4;

	public:
		Sensor(const char * name, int taskCore = 0);

		/**
		 * Here starts.
		 */
		virtual void connect(void * data) = 0;

		virtual void run(void* data) = 0;

		bool isConnected() const;
};

#endif
