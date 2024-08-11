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
	this->preferences.begin("data", false); // Namespace for local storage
}

void Datalogger::run(void * data) {
	// Opening for the first time
	this->tryCard();

	while (1) {
		vTaskDelay(this->iterationDelay);

		if (this->queue.isEmpty()) {
			continue;
		}

		this->save();
	}
}

void Datalogger::saveLocalStorageRow(const Datagas &datagas) {
	uint8_t availableIndex = this->preferences.getUChar("index", 0) + 1;

	if (availableIndex > 16) {
		// reset counter
		availableIndex = 1;
	}

	this->preferences.putULong64(String("r_tim_" + String(availableIndex)).c_str(), datagas.unixtime);
	this->preferences.putUInt(String("r_va1_" + String(availableIndex)).c_str(), datagas.co2);
	this->preferences.putUInt(String("r_va2_" + String(availableIndex)).c_str(), datagas.nh3);
	this->preferences.putFloat(String("r_va3_" + String(availableIndex)).c_str(), datagas.temperature);
	this->preferences.putFloat(String("r_va4_" + String(availableIndex)).c_str(), datagas.humidity);
	this->preferences.putFloat(String("r_va5_" + String(availableIndex)).c_str(), datagas.pressure);
	this->preferences.putFloat(String("r_va6_" + String(availableIndex)).c_str(), datagas.battery);

	// Replace accumulator with new index
	this->preferences.putUChar("index", availableIndex);
}

uint8_t Datalogger::getLastLocalStorageIndex() {
	return this->preferences.getUChar("index", 0);
}

Datagas Datalogger::readLocalStorageRow(uint8_t index) {
	Datagas datagas;
	datagas.unixtime = (time_t) this->preferences.getULong64(String("r_tim_" + String(index)).c_str(), 0);
	datagas.co2 = this->preferences.getUInt(String("r_va1_" + String(index)).c_str(), 0);
	datagas.nh3 = this->preferences.getUInt(String("r_va2_" + String(index)).c_str(), 0);
	datagas.temperature = this->preferences.getFloat(String("r_va3_" + String(index)).c_str(), 0.0f);
	datagas.humidity = this->preferences.getFloat(String("r_va4_" + String(index)).c_str(), 0.0f);
	datagas.pressure = this->preferences.getFloat(String("r_va5_" + String(index)).c_str(), 0.0f);
	datagas.battery = this->preferences.getFloat(String("r_va6_" + String(index)).c_str(), 0.0f);

	return datagas;
}

void Datalogger::cleanLocalStorageRow(uint8_t index) {
	this->preferences.remove(String("r_tim_" + String(index)).c_str());
	this->preferences.remove(String("r_va1_" + String(index)).c_str());
	this->preferences.remove(String("r_va2_" + String(index)).c_str());
	this->preferences.remove(String("r_va3_" + String(index)).c_str());
	this->preferences.remove(String("r_va4_" + String(index)).c_str());
	this->preferences.remove(String("r_va5_" + String(index)).c_str());
	this->preferences.remove(String("r_va6_" + String(index)).c_str());

	// Since this is not a queue, we need to donwgrade
	this->preferences.putUChar("index", index - 1);
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
	byte buf[DATALOGGER_BUF_SIZE];

	struct dblog_write_context ctx;
	ctx.buf = buf;
	ctx.col_count = 7;
	ctx.page_resv_bytes = 0;
	ctx.page_size_exp = 12;
	ctx.max_pages_exp = 0;
	ctx.read_fn = callbackLoggerReadWriteCtx;
	ctx.flush_fn = callbackLoggerFlush;
	ctx.write_fn = callbackLoggerWrite;
	int res = dblog_write_init(&ctx);

	if (!res) {
		while (!this->queue.isEmpty()) {
			Datagas datagas = this->queue.dequeue();

			res = dblog_set_col_val(&ctx, 0, DBLOG_TYPE_INT, &datagas.unixtime, sizeof(uint32_t));
			res = dblog_set_col_val(&ctx, 1, DBLOG_TYPE_INT, &datagas.status, sizeof(uint32_t));
			res = dblog_set_col_val(&ctx, 2, DBLOG_TYPE_INT, &datagas.co2, sizeof(uint32_t));
			res = dblog_set_col_val(&ctx, 3, DBLOG_TYPE_INT, &datagas.nh3, sizeof(uint32_t));
			res = dblog_set_col_val(&ctx, 4, DBLOG_TYPE_INT, &datagas.temperature, sizeof(uint32_t));
			res = dblog_set_col_val(&ctx, 5, DBLOG_TYPE_INT, &datagas.humidity, sizeof(uint32_t));
			res = dblog_set_col_val(&ctx, 6, DBLOG_TYPE_INT, &datagas.pressure, sizeof(uint32_t));

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

void Datalogger::append(uint32_t co2, uint32_t nh3, float_t temperature, float_t humidity, float_t pressure) {
	Datagas datagas;
	datagas.unixtime = 0;
	datagas.status = 0;
	datagas.co2 = co2;
	datagas.nh3 = nh3;
	datagas.temperature = temperature;
	datagas.humidity = humidity;
	datagas.pressure = pressure;

	this->queue.enqueue(datagas);
}

void Datalogger::acquire(ArduinoQueue<Datagas> &datagasQueue) const {
	byte buf[DATALOGGER_BUF_SIZE];

	struct dblog_read_context rctx;
	rctx.page_size_exp = 9;
	rctx.read_fn = callbackLoggerReadReadCtx;
	rctx.buf = buf;
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
			const void * statusColumn = (uint32_t *) dblog_read_col_val(&rctx, 1, &col_type);
			const void * co2Column = (uint32_t *) dblog_read_col_val(&rctx, 2, &col_type);
			const void * nh3Column = (uint32_t *) dblog_read_col_val(&rctx, 3, &col_type);
			const void * temperatureColumn = (uint32_t *) dblog_read_col_val(&rctx, 4, &col_type);
			const void * humidityColumn = (uint32_t *) dblog_read_col_val(&rctx, 5, &col_type);
			const void * pressureColumn = (uint32_t *) dblog_read_col_val(&rctx, 6, &col_type);

			// Set this current row as read
			dblog_upd_col_val(&rctx, 1, (void *) &alreadyReadStatus);

			// Filling data
			datagas.unixtime = *(time_t *) unixtimeColumn;
			datagas.status = *(uint32_t *) statusColumn;
			datagas.co2 = *(uint32_t *) co2Column;
			datagas.nh3 = *(uint32_t *) nh3Column;
			datagas.temperature = *(uint32_t *) temperatureColumn;
			datagas.humidity = *(uint32_t *) humidityColumn;
			datagas.pressure = *(uint32_t *) pressureColumn;

			Serial.print("Unixtime: ");
			Serial.print(datagas.unixtime);

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

void Datalogger::setSaving(bool saving) {
	this->saving = saving;
}

bool Datalogger::isSaving() const {
	return this->saving;
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
