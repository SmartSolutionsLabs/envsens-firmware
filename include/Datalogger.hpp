#ifndef INC_DATALOGGER
#define INC_DATALOGGER

#include "Thread.hpp"

#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoQueue.h>
#include <sqlite3.h>

#ifndef FORMAT_SPIFFS_IF_FAILED
#define FORMAT_SPIFFS_IF_FAILED false
#endif

void IRAM_ATTR interruptDataSaver(void* arg);

/**
 * Data of environment detected by any sensor.
 * Not only for gas.
 */
struct Datagas {
	time_t unixtime = 0;
	uint32_t status = 0; // As read or not
	uint32_t co2 = 0;
	uint32_t nh3 = 0;
	float_t temperature = 0.0f;
	float_t humidity = 0.0f;
	float_t pressure = 0.0f;
	float_t battery = 0.0f;
};

/**
 * Datalogger works as database.
 */
class Datalogger : public Thread {
	protected:
		static Datalogger * datalogger;
		Datalogger();
		Datalogger(const char * name);

		ArduinoQueue<Datagas> queue;

		volatile bool saving = false;

	public:
		static Datalogger * getInstance();
		Datalogger(Datalogger &other) = delete;
		void operator=(const Datalogger &) = delete;

		esp_timer_handle_t saverTimer = nullptr;

		void run(void* data) override;

		bool createDatabase();

		/**
		 * Appends a datagas object in queue.
		 */
		void append(const Datagas &datagas);

		/**
		 * Appends a datagas created with passed parameters.
		 * Unixtime is gotten inside.
		 */
		void append(uint32_t co2, uint32_t nh3, float_t temperature, float_t humidity, float_t pressure);

		/**
		 * Insert a row in local storage.
		 */
		void saveLocalStorageRow(const Datagas &datagas);

		/**
		 * Select a row in local storage.
		 */
		Datagas readLocalStorageRow(uint8_t index);

		/**
		 * Remove keys for that index in local storage.
		 */
		void cleanLocalStorageRow(uint8_t index);

		/**
		 * Fill the queue that must be empty with records.
		 */
		void acquire(ArduinoQueue<Datagas> &datagasQueue) const;

		sqlite3 * getDatabaseFile() const;

		bool trySpiffs();

		void setSaving(bool saving = true);

		bool isSaving() const;

	private:
		const char * filename = "/spiffs/datagas.db";

		sqlite3 * databaseFile;
};

#endif
