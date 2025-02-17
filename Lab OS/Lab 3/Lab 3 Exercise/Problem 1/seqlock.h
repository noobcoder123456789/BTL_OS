#ifndef PTHREAD_H
#include <pthread.h>
#endif

typedef struct pthread_seqlock { /* TODO: implement the structure */
   pthread_mutex_t rw_lock; // read write lock
   unsigned int sequence; // keep track of the write action
} pthread_seqlock_t;

static inline void pthread_seqlock_init(pthread_seqlock_t *seqlock) {
   seqlock->sequence = 0;
   __sync_val_compare_and_swap(&seqlock->sequence, seqlock->sequence, 0);
   pthread_mutex_init(&seqlock->rw_lock, NULL);
}

static inline void pthread_seqlock_wrlock(pthread_seqlock_t *seqlock)
{ 
   __sync_add_and_fetch(&seqlock->sequence, 1);
   pthread_mutex_lock(&seqlock->rw_lock);
}

static inline void pthread_seqlock_wrunlock(pthread_seqlock_t *seqlock)
{
   pthread_mutex_unlock(&seqlock->rw_lock);
   __sync_add_and_fetch(&seqlock->sequence, 1);
}

static inline unsigned int pthread_seqlock_rdlock(pthread_seqlock_t *seqlock)
{
   unsigned int sequence;
   while((sequence = seqlock->sequence) & 1);
   pthread_mutex_lock(&seqlock->rw_lock);
   if(sequence & 1) return 0;
   return 1;
}

static inline unsigned pthread_seqlock_rdunlock(pthread_seqlock_t *seqlock)
{
   pthread_mutex_unlock(&seqlock->rw_lock);
   return 0;
}
