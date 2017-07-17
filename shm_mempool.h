#ifndef _SHM_MEMPOOL_H_
#define _SHM_MEMPOOL_H_

#include <stdint.h>

#define SHM_MEMPOOL_BLOCK_SIZE 8
#define SHM_MEMPOOL_BLOCKS 128

struct shm_mempool_freed {
    struct shm_mempool_freed *next_free;
};

struct shm_mempool {
    uint32_t element_size;
    uint32_t block_size;
    uint32_t used;
    int32_t block;
    struct shm_mempool_freed *freed;
    uint32_t blocks_used;
    uint8_t **blocks[SHM_MEMPOOL_BLOCKS];
    unsigned char *memory;
};


void shm_mempool_init(struct shm_mempool *pool,
                      const uint32_t element_size,
                      const uint32_t block_size,
                      const uint32_t memory_size,
                      void *memory_location);
void *shm_mempool_alloc(struct shm_mempool *pool);
void shm_mempool_free(struct shm_mempool *pool, void *ptr);
void shm_mempool_free_all(struct shm_mempool *pool);

#endif
