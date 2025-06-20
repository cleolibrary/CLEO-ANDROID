#include "mutex.h"

namespace mutex
{
    pthread_mutex_t mutex[msize];
    
    void init()
	{
		memset(mutex, 0, sizeof(mutex));
		for (int i = 0; i < msize; i++)
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&mutex[i], &attr);
			pthread_mutexattr_destroy(&attr);
		}
	}

	void enter(eMutex m)
	{
	    pthread_mutex_lock(&mutex[m]);
	}

	void leave(eMutex m)
	{
	    pthread_mutex_unlock(&mutex[m]);
	}
}