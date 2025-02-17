#ifndef PTHREAD_H
#include <pthread.h>
#endif

typedef struct pthread_seqlock { /* TODO: implement the structure */
   pthread_mutex_t write_lock; // make sure that only one thread can read at the time
   unsigned int sequence; // keep track of the write action
} pthread_seqlock_t;

static inline void pthread_seqlock_init(pthread_seqlock_t *rw)
{
   rw->sequence = 0;
   pthread_mutex_init(&rw->write_lock, NULL);
}

static inline void pthread_seqlock_wrlock(pthread_seqlock_t *rw)
{ 
   rw->sequence++;
   pthread_mutex_lock(&rw->write_lock);
}

static inline void pthread_seqlock_wrunlock(pthread_seqlock_t *rw)
{
   pthread_mutex_unlock(&rw->write_lock);
}

static inline unsigned pthread_seqlock_rdlock(pthread_seqlock_t *rw)
{
   unsigned seq;
   do {
      seq = rw->sequence;
   } while(!__sync_bool_compare_and_swap(&rw->sequence, seq, seq + 1));
   return seq;
}


static inline unsigned pthread_seqlock_rdunlock(pthread_seqlock_t *rw)
{
   __sync_add_and_fetch(&rw->sequence, 1);
   return 0;
}

