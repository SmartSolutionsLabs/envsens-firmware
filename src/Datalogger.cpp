#include "Datalogger.hpp"

static const char * HENSOR_TAG = "Hensor";

Datalogger * Datalogger::datalogger = nullptr;

Datalogger * Datalogger::getInstance() {
	if(datalogger == nullptr) {
		datalogger = new Datalogger("logger");
	}

	return datalogger;
}

Datalogger::Datalogger(const char * name) : Thread(name) {
	//~ this->preferences.begin("data", false); // Namespace for local storage
}

bool Datalogger::trySpiffs() {
	if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
		Serial.println("Failed to mount file system");
		return false;
	}

	sqlite3_initialize();

	if (sqlite3_open(this->filename, &databaseFile)) {
		return false;
	}

	return true;
}

bool Datalogger::createDatabase() {
	char *zErrMsg = 0;

	int rc = sqlite3_exec(this->databaseFile,
		"CREATE TABLE datagas(unixtime INTEGER NOT NULL, status INTEGER NOT NULL, co2 INTEGER NOT NULL, nh3 INTEGER NOT NULL, temperature REAL NOT NULL, humidity REAL NOT NULL, pressure REAL NOT NULL, battery REAL NOT NULL, PRIMARY KEY (unixtime));",
		nullptr, nullptr, &zErrMsg);

	if (rc != SQLITE_OK) {
		sqlite3_free(zErrMsg);
		return false;
	}

	return true;
}

void Datalogger::run(void * data) {
	while (1) {
		vTaskDelay(this->iterationDelay);

		if (this->queue.isEmpty()) {
			continue;
		}
	}
}

void Datalogger::saveLocalStorageRow(const Datagas &datagas) {
	char sqlBuffer[256];
	static const char * sqlInsertPattern = "INSERT INTO datagas VALUES(%d, %d, %d, %d)";
	sprintf(sqlBuffer, sqlInsertPattern, datagas.unixtime, datagas.co2, datagas.nh3, datagas.temperature, datagas.humidity, datagas.pressure, datagas.battery);
	char *zErrMsg = 0;

	int rc = sqlite3_exec(this->databaseFile,
		sqlBuffer,
		nullptr, nullptr, &zErrMsg);

	if (rc != SQLITE_OK) {
		sqlite3_free(zErrMsg);
	}
}

Datagas Datalogger::readLocalStorageRow(uint8_t index) {
	Datagas datagas;

	return datagas;
}

void Datalogger::cleanLocalStorageRow(uint8_t index) {
}

sqlite3 * Datalogger::getDatabaseFile() const {
	return this->databaseFile;
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
}

void Datalogger::setSaving(bool saving) {
	this->saving = saving;
}

bool Datalogger::isSaving() const {
	return this->saving;
}
