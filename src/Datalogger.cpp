#include "Datalogger.hpp"

#include "ulog_sqlite.h"

Datalogger * Datalogger::datalogger = nullptr;

Datalogger * Datalogger::getInstance() {
	if(datalogger == nullptr) {
		datalogger = new Datalogger("logger");
	}

	return datalogger;
}

Datalogger::Datalogger(const char * name) : Thread(name), queue(DATALOGGER_QUEUE_SIZE_ITEMS) {
}

void Datalogger::run(void * data) {
	TickType_t xDelayIteration = 1000 / portTICK_PERIOD_MS; // Iteration speed

	while (1) {
		vTaskDelay(xDelayIteration);

		if (this->queue.isEmpty()) {
			continue;
		}

		this->save();
	}
}

inline void Datalogger::save() {
	struct dblog_write_context ctx;
	int res = dblog_write_init(&ctx);

	if (!res) {
		while (!this->queue.isEmpty()) {
			Datagas datagas = this->queue.dequeue();

			res = dblog_set_col_val(&ctx, 0, DBLOG_TYPE_INT, &datagas.data, sizeof(uint32_t));

			res = dblog_append_empty_row(&ctx);
		}
	}

	Serial.print("Logging completed. Finalizing...\n");
    if (!res) {
		res = dblog_finalize(&ctx);
	}
}

void Datalogger::append(const Datagas &datagas) {
	this->queue.enqueue(datagas);
}

void Datalogger::append(uint32_t data) {
	Datagas datagas;
	datagas.unixtime = 0;
	datagas.data = data;

	this->queue.enqueue(datagas);
}
