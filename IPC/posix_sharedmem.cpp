#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "syscall_error.h"


int main() {

    auto shmid = shm_open("/test.shm", O_CREAT | O_RDWR, 0777);
    if (shmid == -1) throw syscall_error("shmid");

    constexpr auto SIZE = 1024 * 1024;
    auto val = int{13};

    if (ftruncate(shmid, SIZE) == -1)
        throw syscall_error("ftruncate");

    auto addr = mmap(nullptr, SIZE, PROT_WRITE, MAP_SHARED, shmid, 0);
    if (addr == (void *) -1) throw syscall_error("mmap");

    if (memset(addr, val, SIZE) == nullptr)
        throw syscall_error("memset");

    if (munmap(addr, SIZE) == -1)
        throw syscall_error("munmap");

    if (shm_unlink("/test.shm") == -1)
        throw syscall_error("shm_unlink");
}