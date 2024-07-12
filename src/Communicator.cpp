#include "Communicator.hpp"

Communicator * Communicator::communicator = nullptr;

Communicator * Communicator::getInstance() {
	if (communicator == nullptr) {
		communicator = new Communicator("comm", 1);
	}

	return communicator;
}

Communicator::Communicator(const char * name, int taskCore) : Thread(name, taskCore) {
}

void Communicator::parseIncome(void * data) {
}

inline void Communicator::sendOut() {
}

void Communicator::run(void * data) {
	TickType_t xDelayIteration = 1000 / portTICK_PERIOD_MS; // Iteration speed

	while (1) {
		vTaskDelay(xDelayIteration);

		this->sendOut();
	}
}
