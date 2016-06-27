#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "syscall_error.h"
#include <fcntl.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};


int main() {

    auto key = ftok("/tmp/sem.temp", 1);
    if (key == -1) throw syscall_error("ftok");

    auto semid = semget(key, 16, 0777 | IPC_CREAT);
    if (semid == -1) throw syscall_error("semget");

    auto arg = semun{};

    for (auto i = 0; i < 16; i++) {
        arg.val = i;
        if (semctl(semid, i, SETVAL, arg) == -1)
            throw syscall_error("semctl");
    }

    if (semctl(semid,0, IPC_RMID, nullptr) == -1)
        throw syscall_error("semctl");

    return 0;
}
