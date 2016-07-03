#include "syscall_error.h"
#include <sys/types.h>
#include <unistd.h>
#include <fstream>


pthread_mutex_t mt_simple = PTHREAD_MUTEX_INITIALIZER;
pthread_spinlock_t mt_spin;
pthread_rwlock_t mt_rw= PTHREAD_RWLOCK_INITIALIZER;

void *wait_mutex(void *attr) {
    auto status = pthread_mutex_lock(&mt_simple);
    if (status != 0) { errno = status; throw syscall_error("pthread_mutex_lock"); }

    std::cout <<"I got the simple mutex\n";

    status = pthread_mutex_unlock(&mt_simple);
    if (status != 0) { errno = status; throw syscall_error("pthread_mutex_unlock"); }

    return NULL;
}

void *wait_spin(void *attr) {
    auto status = pthread_spin_lock(&mt_spin);
    if (status != 0) { errno = status; throw syscall_error("pthread_spin_lock"); }

    std::cout <<"I got the spin mutex\n";

    status = pthread_spin_unlock(&mt_spin);
    if (status != 0) { errno = status; throw syscall_error("pthread_spin_unlock"); }

    return NULL;
}

void *wait_read(void *attr) {
    auto status = pthread_rwlock_rdlock(&mt_rw);
    if (status != 0) { errno = status; throw syscall_error("pthread_rwlock_rdlock"); }

    std::cout <<"I got the read mutex\n";

    status = pthread_rwlock_unlock(&mt_rw);
    if (status != 0) { errno = status; throw syscall_error("pthread_rwlock_unlock"); }

    return NULL;
}

void *wait_write(void *attr) {
    auto status = pthread_rwlock_wrlock(&mt_rw);
    if (status != 0) { errno = status; throw syscall_error("pthread_rwlock_wrlock"); }

    std::cout <<"I got the write mutex\n";

    status = pthread_rwlock_unlock(&mt_rw);
    if (status != 0) { errno = status; throw syscall_error("pthread_rwlock_unlock"); }

    return NULL;
}


int main() {
    std::ofstream fout("/home/box/main.pid");
    fout << getpid();
    fout.close();

    auto thread_simple = pthread_t{};
    auto thread_spin = pthread_t{};
    auto thread_read = pthread_t{};
    auto thread_write = pthread_t{};
    auto status = pthread_spin_init(&mt_spin, PTHREAD_PROCESS_PRIVATE);
    if (status != 0) { errno = status; throw syscall_error("pthread_spin_init"); }


    //Simple mutex lock
    status = pthread_mutex_lock(&mt_simple);
    if (status != 0) { errno = status; throw syscall_error("pthread_mutex_lock"); }

    status = pthread_create(&thread_simple, nullptr, wait_mutex, nullptr);
    if (status != 0) { errno = status; throw syscall_error("pthread_create"); }

    //Spin mutex lock
    status = pthread_spin_lock(&mt_spin);
    if (status != 0) { errno = status; throw syscall_error("pthread_spinlock"); }

    status = pthread_create(&thread_spin, nullptr, wait_spin, nullptr);
    if (status != 0) { errno = status; throw syscall_error("pthread_create"); }


    //write mutex lock
    status = pthread_rwlock_wrlock(&mt_rw);
    if (status != 0) { errno = status; throw syscall_error("pthread_rwlock_rdlock"); }

    status = pthread_create(&thread_write, nullptr, wait_write, nullptr);
    if (status != 0) { errno = status; throw syscall_error("pthread_create"); }

    //Read mutex lock
//    status = pthread_rwlock_rdlock(&mt_rw);
//    if (status != 0) { errno = status; throw syscall_error("pthread_rwlock_rdlock"); }

    status = pthread_create(&thread_read, nullptr, wait_read, nullptr);
    if (status != 0) { errno = status; throw syscall_error("pthread_create"); }

    //Pause
//    sleep(2);
    pause();

    //Simple mutex unlock + join
    status = pthread_mutex_unlock(&mt_simple);
    if (status != 0) { errno = status; throw syscall_error("pthread_mutex_unlock"); }

    auto join_status = int{};
    status = pthread_join(thread_simple, reinterpret_cast<void **>(&join_status));
    if (status != 0) { errno = status; throw syscall_error("pthread_join"); }

    //Spin mutex unlock + join
    status = pthread_spin_unlock(&mt_spin);
    if (status != 0) { errno = status; throw syscall_error("pthread_spin_unlock"); }

    join_status = int{};
    status = pthread_join(thread_spin, reinterpret_cast<void **>(&join_status));
    if (status != 0) { errno = status; throw syscall_error("pthread_join"); }

    //Read + write mutex unlock + join
    status = pthread_rwlock_unlock(&mt_rw);
    if (status != 0) { errno = status; throw syscall_error("pthread_rwlock_unlock"); }

    join_status = int{};
    status = pthread_join(thread_read, reinterpret_cast<void **>(&join_status));
    if (status != 0) { errno = status; throw syscall_error("pthread_join"); }

    join_status = int{};
    status = pthread_join(thread_write, reinterpret_cast<void **>(&join_status));
    if (status != 0) { errno = status; throw syscall_error("pthread_join"); }


    //Destructors
    status = pthread_spin_destroy(&mt_spin);
    if (status != 0) { errno = status; throw syscall_error("pthread_spin_destroy"); }

    return 0;
}