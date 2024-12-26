#include "asynci.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>



static void* asyncTaskRunner(void* data) {
    taskData_t* taskData = (taskData_t*)data;
    if (!taskData) {
        fprintf(stderr, "Error: asyncTaskRunner received NULL task data\n");
        return NULL;
    }

    taskData->action(NULL, taskData->arg);

    free(taskData);

    pthread_mutex_lock(&tasksMutex);
    activeTasks--;
    if (activeTasks == 0) {
        pthread_cond_signal(&tasksCond);
    }
    pthread_mutex_unlock(&tasksMutex);

    return NULL;
}

task_t asyncTask(taskAction_f action, void* arg) {
    if (!action) {
        fprintf(stderr, "Error: asyncTask received a NULL action pointer\n");
        return NULL;
    }

    taskData_t* taskData = (taskData_t*)malloc(sizeof(taskData_t));
    if (!taskData) {
        perror("Failed to allocate memory for task data");
        return NULL;
    }
    taskData->action = action;
    taskData->arg = arg;

    pthread_mutex_lock(&tasksMutex);
    activeTasks++;
    pthread_mutex_unlock(&tasksMutex);



    int result = pthread_create(&taskData->thread, NULL, asyncTaskRunner, taskData);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to create thread (%d)\n", result);
        free(taskData);

        pthread_mutex_lock(&tasksMutex);
        activeTasks--;
        pthread_mutex_unlock(&tasksMutex);

        return NULL;
    }

    return (task_t)taskData;
}

__attribute__((destructor)) static void waitForTasksToComplete() {
    pthread_mutex_lock(&tasksMutex);
    while (activeTasks > 0) {
        pthread_cond_wait(&tasksCond, &tasksMutex);
    }
    pthread_mutex_unlock(&tasksMutex);
}

static void* afterOldTaskRunner(task_t t){
    taskData_t* dataOld = (taskData_t*) t;
    int joinResult = pthread_join(dataOld->thread, NULL);
    if (joinResult != 0) {
        fprintf(stderr, "Error: pthread_join failed (%d)\n", joinResult);
        return NULL;
    }
}


task_t asyncTaskAfter(task_t t, taskAction_f action, void* arg){
    if (!action) {
        fprintf(stderr, "Error: asyncTask received a NULL action pointer\n");
        return NULL;
    }
    if (!t) {
        fprintf(stderr, "Error: asyncTaskAfter received NULL task\n");
        return NULL;
    }

    afterOldTaskRunner(t);

    return asyncTask(action,arg);
}

task_t asyncTaskAfterAll(task_t* tt, taskAction_f action , void* arg){
    if (!tt ) {
        fprintf(stderr, "Error: asyncTaskAfterAll received NULL or empty task array\n");
        return NULL;
    }
    if (!action) {
        fprintf(stderr, "Error: asyncTaskAfterAll received a NULL action pointer\n");
        return NULL;
    }

    for (size_t i = 0; tt[i] != NULL; i++) {
        afterOldTaskRunner(tt[i]);
    }
    return asyncTask(action, arg);
}

task_t asyncAfterMultiplex(int fd, asyncMultiplexOperation_t op) {
    if (fd < 0) {
        fprintf(stderr, "Error: Invalid file descriptor\n");
        return NULL;
    }

    fd_set readfds, writefds, exceptfds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    if (op & AMO_READ) {
        FD_SET(fd, &readfds);
    }
    if (op & AMO_WRITE) {
        FD_SET(fd, &writefds);
    }
    if (op & AMO_EXCEPT) {
        FD_SET(fd, &exceptfds);
    }

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    int selectResult = select(fd + 1, &readfds, &writefds, &exceptfds, &timeout);
    if (selectResult == -1) {
        perror("Error: select failed");
        return NULL;
    }

    if (selectResult == 0) {
        fprintf(stderr, "Error: select timed out\n");
        return NULL;
    }

    if (FD_ISSET(fd, &readfds) || FD_ISSET(fd, &writefds) || FD_ISSET(fd, &exceptfds)) {
        pthread_mutex_lock(&tasksMutex);
        activeTasks--;
        if (activeTasks == 0) {
            pthread_cond_signal(&tasksCond);
        }
        pthread_mutex_unlock(&tasksMutex);
        return NULL;
    }

    return NULL;
}

