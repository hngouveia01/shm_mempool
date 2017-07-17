#include <stdint.h>
#include <stdlib.h>
#include <sys/param.h>
#include <fcntl.h>

#include "shm_mempool.h"

/*!
 * \brief Initializes memory pool to a given
 *        shared memory.
 * \param[out] pool memory pool structure to be initialized.
 * \param[in] element_size element size of data that will be stored.
 * \param[in] block_size block size can be 4 or 8.
 * \param[in] memory_size size of shared memory.
 * \param[in] memory address of the shared memory.
 */
void shm_mempool_init(struct shm_mempool *pool,
                      const uint32_t element_size,
                      const uint32_t block_size,
                      const uint32_t memory_size,
                      void *memory)
{
    int i;

    pool->element_size = MAX(element_size, sizeof(struct shm_mempool_freed));
    pool->block_size = block_size;

    shm_mempool_free_all(pool);

    pool->blocks_used = memory_size / block_size;
    pool->memory = memory;

    for (i = 0; i < pool->blocks_used; i++)
        pool->blocks[i] = NULL;
}

/*!
 * \brief Returns a free block of memory from the pool.
 * \param[out] pool pool structure that will be updated.
 *
 * \return returns a free block of memory from the pool.
 * \return NULL if there is no space left in the pool.
 */
void *shm_mempool_alloc(struct shm_mempool *pool)
{
    /* there is a block recently freed. Recycle. */
    if (pool->freed != NULL) {
        void *recycle = pool->freed;
        pool->freed = pool->freed->next_free;
        return recycle;
    }

    /* first allocation */
    if (++pool->used == pool->block_size) {
        pool->used = 0;

        if (++pool->block == (uint32_t) pool->blocks_used) {
            /* no space left */
            return NULL;
        }

        /* block unused */
        if (pool->blocks[pool->block] == NULL)
            pool->blocks[pool->block] = (uint8_t **) pool->memory + (pool->used * pool->element_size);
    }

    return pool->blocks[pool->block] + pool->used * pool->element_size;
}

/*!
 * \brief Given a pointer of a previously allocated memory region,
 *        returns it to the pool and marks it as freed.
 * \param[out] pool pool structure that will be updated.
 * \param[in] ptr previously allocated memory to be returned to the pool.
 */
void shm_mempool_free(struct shm_mempool *pool, void *ptr)
{
    struct shm_mempool_freed *pool_freed = pool->freed;
    pool->freed = ptr;
    pool->freed->next_free = pool_freed;
}

/*! Marks all blocks of memory as free. */
void shm_mempool_free_all(struct shm_mempool *pool)
{
    pool->used = pool->block_size - 1;
    pool->block = -1;
    pool->freed = NULL;
}
