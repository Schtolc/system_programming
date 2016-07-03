#include "syscall_error.h"
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

pthread_mutex_t mt_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond;

pthread_barrier_t barrier;

void *wait_conditional(void *attr) {
    auto status = pthread_mutex_lock(&mt_cond);
    if (status != 0) { errno = status; throw syscall_error("pthread_mutex_lock"); }

    status = pthread_cond_wait(&cond, &mt_cond);
    if (status != 0) { errno = status; throw syscall_error("pthread_cond_lock"); }

    std::cout <<"Conditional unlocked!\n";

    status = pthread_mutex_unlock(&mt_cond);
    if (status != 0) { errno = status; throw syscall_error("pthread_mutex_unlock"); }

    return NULL;
}

void *wait_barrier(void *attr) {
    auto status = pthread_barrier_wait(&barrier);
    if (status != PTHREAD_BARRIER_SERIAL_THREAD && status != 0)
        { errno = status; throw syscall_error("pthread_mutex_unlock"); }

    std::cout <<"Barrier unlocked!\n";

    return NULL;
}


int main() {
    std::ofstream fout("/home/box/main.pid");
    fout << getpid();
    fout.close();

    auto thread_cond = pthread_t{};
    auto thread_barrier = pthread_t{};

    //Constructors
    auto status = pthread_cond_init(&cond, nullptr);
    if (status != 0) { errno = status; throw syscall_error("pthread_cond_init"); }

    status = pthread_barrier_init(&barrier, nullptr, 2);
    if (status != 0) { errno = status; throw syscall_error("pthread_barrier_init"); }

    status = pthread_create(&thread_cond, nullptr, wait_conditional, nullptr);
    if (status != 0) { errno = status; throw syscall_error("pthread_create"); }

    status = pthread_create(&thread_barrier, nullptr, wait_barrier, nullptr);
    if (status != 0) { errno = status; throw syscall_error("pthread_create"); }

    //Pause
    sleep(2);
//    pause();

    //Conditional mutex lock
    status = pthread_mutex_lock(&mt_cond);
    if (status != 0) { errno = status; throw syscall_error("pthread_mutex_lock"); }

    //Conditional signal
    status = pthread_cond_signal(&cond);
    if (status != 0) { errno = status; throw syscall_error("pthread_cond_signal"); }

    //Conditional mutex unlock + join
    status = pthread_mutex_unlock(&mt_cond);
    if (status != 0) { errno = status; throw syscall_error("pthread_mutex_unlock"); }

    auto join_status = int{};
    status = pthread_join(thread_cond, reinterpret_cast<void **>(&join_status));
    if (status != 0) { errno = status; throw syscall_error("pthread_join"); }

    //Barrier unlock + join
    status = pthread_barrier_wait(&barrier);
    if (status != PTHREAD_BARRIER_SERIAL_THREAD && status != 0)
    { errno = status; throw syscall_error("pthread_mutex_unlock"); }

    join_status = int{};
    status = pthread_join(thread_barrier, reinterpret_cast<void **>(&join_status));
    if (status != 0) { errno = status; throw syscall_error("pthread_join"); }

    //Destructors
    status = pthread_cond_destroy(&cond);
    if (status != 0) { errno = status; throw syscall_error("pthread_cond_destroy"); }

    status = pthread_barrier_destroy(&barrier);
    if (status != 0) { errno = status; throw syscall_error("pthread_cbarrier_destroy"); }


    return 0;
}