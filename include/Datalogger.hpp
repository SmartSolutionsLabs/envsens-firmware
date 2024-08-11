#ifndef INC_DATALOGGER
#define INC_DATALOGGER

#include "Thread.hpp"

#include "FS.h"
#include <ArduinoQueue.h>
#include <Preferences.h>

#ifndef DATALOGGER_QUEUE_SIZE_ITEMS
#define DATALOGGER_QUEUE_SIZE_ITEMS 128
#endif

#ifndef DATALOGGER_BUF_SIZE
#define DATALOGGER_BUF_SIZE 4096
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

		/**
		 * Store for local data.
		 */
		Preferences preferences;

	public:
		static Datalogger * getInstance();
		Datalogger(Datalogger &other) = delete;
		void operator=(const Datalogger &) = delete;

		esp_timer_handle_t saverTimer = nullptr;

		void run(void* data) override;

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
		 * View the last used index for local storage.
		 */
		uint8_t getLastLocalStorageIndex();

		/**
		 * Remove keys for that index in local storage.
		 */
		void cleanLocalStorageRow(uint8_t index);

		/**
		 * Fill the queue that must be empty with records.
		 */
		void acquire(ArduinoQueue<Datagas> &datagasQueue) const;

		FILE * getDatabaseFile() const;

		inline bool tryCard();

		void setSaving(bool saving = true);

		bool isSaving() const;

	private:
		inline void save();

		const char * filename = "/sd/datagas.db";

		FILE * databaseFile;
};

// Callback headers for Micro Logger API
int callbackLoggerFlush(struct dblog_write_context *ctx);
int32_t callbackLoggerWrite(struct dblog_write_context *ctx, void *buf, uint32_t pos, size_t len);
int32_t callbackLoggerReadReadCtx(struct dblog_read_context *ctx, void *buf, uint32_t pos, size_t len);
int32_t callbackLoggerReadWriteCtx(struct dblog_write_context *ctx, void *buf, uint32_t pos, size_t len);

#endif
