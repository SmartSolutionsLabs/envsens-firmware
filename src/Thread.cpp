#include "Thread.hpp"

static const char * HENSOR_TAG = "Hensor";

Thread::Thread(const char * name, int taskCore) : name(name), taskCore(taskCore) {
}

Thread::Thread() : name("noName"), taskCore(0) {
}

void Thread::runTask(void *_this) {
	Thread * thread = (Thread *) _this;
	thread->run(_this);
}

void Thread::start() {
	if(this->taskHandler != nullptr) {
		ESP_LOGE(HENSOR_TAG, "There might be a %s task already running!", this->name);
		return;
	}

	//Start Task with input parameter set to "this" class
	xTaskCreatePinnedToCore(
		&runTask,          //Function to implement the task
		this->name,        //Name of the task
		25000,             //Stack size in words
		this,              //Task input parameter
		this->taskPriority,//Priority of the task
		&taskHandler,      //Task handle.
		this->taskCore);   //Core where the task should run
}

void Thread::stop() {
	ESP_LOGV(HENSOR_TAG, "Stopping %s task...", this->name);
	// From within the task itself is null
	vTaskDelete(NULL);
}
