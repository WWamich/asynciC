#include "asynci.h"
#include "stdio.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>

void* exampleAction(task_t task, void* arg) {
    printf("[DEBUG] Task started with arg: %s\n", (char*)arg);
    usleep(500000);  // Симуляция работы
    printf("[DEBUG] Task completed with arg: %s\n", (char*)arg);
    return NULL;
}

void* exampleTask(task_t task, void* arg) {
    printf("[DEBUG] Task started with arg: %s\n", (char*)arg);
    usleep(500000);
    printf("[DEBUG] Task completed with arg: %s\n", (char*)arg);
    return NULL;
}

void testAsyncAfterMultiplex(int fd) {
    asyncAfterMultiplex(fd, AMO_READ | AMO_WRITE);

    task_t t1 = asyncTask(exampleAction, "Task 1");
    task_t t2 = asyncTask(exampleTask, "Task 2");

    asyncTaskAfter(t1, exampleAction, "Task 1.1");
    asyncTaskAfter(t2, exampleTask, "Task 2.1");
}

int main() {
    printf("Starting async tests...\n");

    int fd = open("testfile.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Failed to open file");
        return 1;
    }

    task_t t1_2 = asyncTask(exampleAction, "Task 1.2");
    task_t t1_5 = asyncTask(exampleTask, "Task 1.5");
    asyncTaskAfter(t1_5, exampleAction, "Task 1");
    task_t t1_3 = asyncTask(exampleTask, "Task 1.3");
    task_t t1_4 = asyncTask(exampleTask, "Task 1.4");
    task_t tt[] = {t1_3, t1_4, NULL};
    asyncTaskAfterAll(tt, exampleAction, "The end!");

    testAsyncAfterMultiplex(fd);

    close(fd);

    return 0;
}
