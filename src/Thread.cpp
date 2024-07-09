#include "Thread.hpp"

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
		Serial.println("Task::start - There might be a task already running!");
		return;
	}

	Serial.println("Starting some task...");
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
}
