

#ifndef UNTITLED7_ASYNCI_H
#define UNTITLED7_ASYNCI_H

#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>


typedef void* task_t;
typedef void* (*taskAction_f)(task_t task, void* arg);

static int activeTasks = 0;
static pthread_mutex_t tasksMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tasksCond = PTHREAD_COND_INITIALIZER;

typedef struct {
    taskAction_f action;
    void* arg;
    pthread_t thread;
} taskData_t;

typedef enum asyncMultiplexOperation {
    AMO_READ = 0x01 << 0,
    AMO_WRITE = 0x01 << 1,
    AMO_EXCEPT = 0x03 << 2,
    AMO_ANY = AMO_READ|AMO_WRITE|AMO_EXCEPT
} asyncMultiplexOperation_t;

task_t asyncTask(taskAction_f action, void* arg);
task_t asyncTaskAfter(task_t t, taskAction_f action, void* arg);
task_t asyncTaskAfterAll(task_t* tt, taskAction_f action, void* arg);
task_t asyncAfterMultiplex(int fd, asyncMultiplexOperation_t op);



#endif //UNTITLED7_ASYNCI_H
