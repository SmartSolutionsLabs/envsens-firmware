#ifndef INC_DATALOGGER
#define INC_DATALOGGER

#include "Thread.hpp"

#include "FS.h"
#include <ArduinoQueue.h>

#ifndef DATALOGGER_QUEUE_SIZE_ITEMS
#define DATALOGGER_QUEUE_SIZE_ITEMS 128
#endif

/**
 * Data of environment detected by any sensor.
 * Not only for gas.
 */
struct Datagas {
	time_t unixtime;
	uint32_t data;
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

	public:
		static Datalogger * getInstance();
		Datalogger(Datalogger &other) = delete;
		void operator=(const Datalogger &) = delete;

		void run(void* data) override;

		/**
		 * Appends a datagas object in queue.
		 */
		void append(const Datagas &datagas);

		/**
		 * Appends a datagas created with passed parameters.
		 * Unixtime is gotten inside.
		 */
		void append(uint32_t data);

		/**
		 * Fill the queue that must be empty with records.
		 */
		void acquire(ArduinoQueue<Datagas> &datagasQueue) const;

		FILE * getDatabaseFile() const;

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
