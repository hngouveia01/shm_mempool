#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#include "shm_mempool.h"


int main(void)
{
    struct shm_mempool pool;
    int shm_fd_struct;
    void *shared_struct = NULL;
    int exist = 0;
    sem_t *mutex;

    int shm_fd = shm_open("/test-shm", O_CREAT | O_RDWR, 0666);

    int ret = ftruncate(shm_fd, sizeof(unsigned char)*1024);

    if (ret == -1)
        return 1;

    void *memory = mmap((void *)0x7f16c83e5000,
                        sizeof(unsigned char)*1024,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_FIXED, shm_fd, 0);

    if (!access("/dev/shm/test-shm-struct", R_OK | W_OK))
        exist++;

    shm_fd_struct = shm_open("/test-shm-struct", O_CREAT | O_RDWR, 0666);
    ret = ftruncate(shm_fd_struct, sizeof(struct shm_mempool));

    if (ret == -1)
        return 1;

    shared_struct = mmap(NULL,
                         sizeof(struct shm_mempool),
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED, shm_fd_struct, 0);

    mutex = sem_open("/pool-sem", O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU, 0);

    if (!exist) {
        sem_init(mutex, 1, 0);
        shm_mempool_init(shared_struct,
                         sizeof(int),
                         8,
                         sizeof(unsigned char)*1024,
                         memory);
        sem_post(mutex);
    }

    int i;
    for (i = 0; i < 11; i++) {
        sem_wait(mutex);
        unsigned char *ptr = shm_mempool_alloc(shared_struct);
        ptr[0] = 'h';
        ptr[1] = 'i';
        ptr[2] = '\0';
        sem_post(mutex);

        puts(ptr);

        sem_wait(mutex);
        shm_mempool_free(shared_struct, ptr);
        sem_post(mutex);
    }

    shm_unlink("/test-shm-struct");
    shm_unlink("/test-shm");

    return 0;
}
