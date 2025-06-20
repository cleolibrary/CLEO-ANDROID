#include "mutex.h"

namespace mutex
{
#ifdef ANDROID
	pthread_mutex_t mutex[msize];
#else
	SceUID mutex[msize];
#endif

	void init()
	{
		memset(mutex, 0, sizeof(mutex));
		for (int i = 0; i < msize; i++)
		{
#ifdef ANDROID
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&mutex[i], &attr);
			pthread_mutexattr_destroy(&attr);			
#else
			char name[32];
			sprintf(name, "CleoMutex_%d", i);
			mutex[i] = sceKernelCreateSema(name, 0, 1, 1, NULL);
#endif
		}
	}

	void enter(eMutex m)
	{
#ifdef ANDROID
		pthread_mutex_lock(&mutex[m]);
#else
		sceKernelWaitSema(mutex[m], 1, 0);
#endif
	}

	void leave(eMutex m)
	{
#ifdef ANDROID
		pthread_mutex_unlock(&mutex[m]);
#else
		sceKernelSignalSema(mutex[m], 1);
#endif
	}
}