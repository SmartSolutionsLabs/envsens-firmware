#include "Datalogger.hpp"

#include "SD.h"
#include "ulog_sqlite.h"

static const char * HENSOR_TAG = "Hensor";

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
	while (1) {
		vTaskDelay(this->iterationDelay);

		if (!this->tryCard()) {
			continue;
		}

		if (this->queue.isEmpty()) {
			continue;
		}

		this->save();
	}
}

FILE * Datalogger::getDatabaseFile() const {
	return this->databaseFile;
}

inline bool Datalogger::tryCard() {
	if (SD.exists("/datagas.db")) {
		return true;
	}
	else {
		if (!SD.begin()){
			ESP_LOGW(HENSOR_TAG, "Card Mount Failed");
			return false;
		}

		uint8_t cardType = SD.cardType();

		if (cardType == CARD_NONE) {
			ESP_LOGW(HENSOR_TAG, "No SD card attached");
			return false;
		}

		if(cardType == CARD_MMC){
			ESP_LOGI(HENSOR_TAG, "SD Card Type: MMC");
		}
		else if(cardType == CARD_SD) {
			ESP_LOGI(HENSOR_TAG, "SD Card Type: SDSC");
		}
		else if(cardType == CARD_SDHC){
			ESP_LOGI(HENSOR_TAG, "SD Card Type: SDHC");
		}
		else {
			ESP_LOGI(HENSOR_TAG, "Unknown SD Card Type");
		}

		uint64_t cardSize = SD.cardSize() / (1024 * 1024);
		ESP_LOGI(HENSOR_TAG, "SD Card Size: %lluMB", cardSize);

		this->databaseFile = fopen(this->filename, "w+b");

		if (this->databaseFile == nullptr) {
			ESP_LOGE(HENSOR_TAG, "Failed to open database for writing");
			return false;
		}
		else {
			ESP_LOGI(HENSOR_TAG, "Opened file for writing");
			return true;
		}
	}
}

inline void Datalogger::save() {
	struct dblog_write_context ctx;
	ctx.flush_fn = callbackLoggerFlush;
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

void Datalogger::acquire(ArduinoQueue<Datagas> &datagasQueue) const {
	struct dblog_read_context rctx;
	int res = dblog_read_init(&rctx);

	static const int neverReadStatus = 0; // When never was read before
	static const int alreadyReadStatus = 1;

	res = dblog_bin_srch_row_by_val(&rctx, 1, DBLOG_TYPE_INT, (void *) &neverReadStatus, datagasQueue.itemCount(), 0);

	if (res == DBLOG_RES_NOT_FOUND) {
		Serial.print("No records found.\n");
		return;
	}
	else if (res == 0) {
		do {
			// Create a Datagas object and fill it with data
			Datagas datagas;
			uint32_t col_type;
			const void * unixtimeColumn = (time_t *) dblog_read_col_val(&rctx, 0, &col_type);
			const void * dataColumn = (uint32_t *) dblog_read_col_val(&rctx, 2, &col_type);

			// Set this current row as read
			dblog_upd_col_val(&rctx, 1, (void *) &alreadyReadStatus);

			// Filling data
			datagas.unixtime = *(time_t *) unixtimeColumn;
			datagas.data = *(uint32_t *) dataColumn;

			// Append it to the queue
			datagasQueue.enqueue(datagas);
		} while (!dblog_read_next_row(&rctx));

		return;
	}
	else {
		Serial.print("Error ");
		Serial.print(res);
		Serial.print(" reading datalog.\n");
	}
}

int callbackLoggerFlush(struct dblog_write_context *ctx) {
	return DBLOG_RES_OK;
}

int32_t callbackLoggerWrite(struct dblog_write_context *ctx, void *buf, uint32_t pos, size_t len) {
	FILE * databaseFile = Datalogger::getInstance()->getDatabaseFile();

	if (fseek(databaseFile, pos, SEEK_SET)) {
		return DBLOG_RES_SEEK_ERR;
	}

	size_t ret = fwrite(buf, 1, len, databaseFile);
	if (ret != len) {
		return DBLOG_RES_ERR;
	}

	if (fflush(databaseFile)) {
		return DBLOG_RES_FLUSH_ERR;
	}

	fsync(fileno(databaseFile));

	return ret;
}

int32_t callbackLoggerReadReadCtx(struct dblog_read_context *ctx, void *buf, uint32_t pos, size_t len) {
	FILE * databaseFile = Datalogger::getInstance()->getDatabaseFile();

	if (fseek(databaseFile, pos, SEEK_SET)) {
		return DBLOG_RES_SEEK_ERR;
	}

	size_t ret = fread(buf, 1, len, databaseFile);
	if (ret != len) {
		return DBLOG_RES_READ_ERR;
	}

	return ret;
}

int32_t callbackLoggerReadWriteCtx(struct dblog_write_context *ctx, void *buf, uint32_t pos, size_t len) {
	FILE * databaseFile = Datalogger::getInstance()->getDatabaseFile();

	if (fseek(databaseFile, pos, SEEK_SET)) {
		return DBLOG_RES_SEEK_ERR;
	}

	size_t ret = fread(buf, 1, len, databaseFile);
	if (ret != len) {
		return DBLOG_RES_READ_ERR;
	}

	return ret;
}
