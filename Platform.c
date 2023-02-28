#include "_WQ.h"
#if defined  (_platform_mingw) || defined(_platform_win)

#elif defined (_platform_linux)
 static void timeconv_abs(_dword ms, struct timespec *time)
 {
		struct timespec cur_clock;
		_dword total_clock = 0;
		int carry = 0;

		if(!time)
		{
			return;
		}
		clock_gettime(CLOCK_REALTIME, &cur_clock);

		total_clock = ms + (cur_clock.tv_nsec / 1000000);

		carry = total_clock / 1000;

		time->tv_sec = cur_clock.tv_sec += carry;
		time->tv_nsec = cur_clock.tv_nsec  = ((total_clock % 1000) * 1000000) + cur_clock.tv_nsec % 1000000;
 }

static void *__start_routine (void * p)
{
	struct BackGround *b = (struct BackGround *)p;
	b->func(b->p);
	return NULL;
}

#else

#endif


int Platform_thread_open(struct BackGround *b)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return WQ_INVALID;
#elif defined (_platform_linux)

	if(!b)
	{
		return WQEINVAL;
	}
	if(pthread_create(&b->t_id, NULL, __start_routine, b) != 0)
	{
		return WQENOMEM;
	}
	return WQOK;

#else
	return WQ_INVALID;
#endif
}
void Platform_thread_close(struct BackGround *b)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return ;
#elif defined (_platform_linux)

	if(!b)
	{
		return ;
	}
	pthread_join(b->t_id, NULL);
#else
	return ;
#endif
}
void  Platform_sleep(_dword time)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return ;
#elif defined (_platform_linux)

		usleep(time * 1000);
#else
	return ;
#endif
}
_dword Platform_tick_count()
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return 0;
#elif defined (_platform_linux)
	struct timeval v;
	gettimeofday(&v, NULL);

	return (v.tv_sec * 1000) + (v.tv_usec / 1000);

#else
	return 0;
#endif
}
void  Platform_thread_id(threadid *id)
{
#if defined  (_platform_mingw) || defined(_platform_win)

#elif defined (_platform_linux)
	if(id) *id = pthread_self();
#else

#endif
}
int Platform_semephore_open(semaphore *sema, _dword init, _dword max)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return -1;
#elif defined (_platform_linux)
	return sema ? sem_init(sema,  0 , init) : -1;
#else
	return -1;
#endif
}
struct _named_semaphore *Platform_semaphore_named_open(semaphore_named_par *par)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return NULL;
#else
	struct _named_semaphore *nsem = (struct _named_semaphore *)libwq_malloc(sizeof(struct _named_semaphore));
	if(!nsem)
	{
		return NULL;
	}
	memset(nsem, 0, sizeof(struct _named_semaphore));
	sprintf(nsem->_name, "%s", par->key);
	if(par->owner)
	{
		nsem->sem = sem_open(nsem->_name, (O_CREAT|O_EXCL), par->mode, par->initval);
	}
	else
	{
		nsem->sem = sem_open(nsem->_name, 0);
	}
	if(nsem->sem == SEM_FAILED)
	{
		libwq_free(nsem);
		return NULL;
	}
	nsem->owner = par->owner;
	return nsem;
#endif
}
void Platform_semaphore_named_close(struct _named_semaphore *nsem)
{
#if defined  (_platform_mingw) || defined(_platform_win)
#else
	if(nsem)
	{
		sem_close(nsem->sem);
		if(nsem->owner)
		{
			sem_unlink(nsem->_name);
		}
		libwq_free(nsem);
	}
#endif
}
void  Platform_semaphore_close(semaphore *sema)
{
#if defined  (_platform_mingw) || defined(_platform_win)

#elif defined (_platform_linux)
	if(sema) sem_destroy(sema);
#else
#endif
}

_dword Platform_semaphore_lock(semaphore *sema, _dword time)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return OBJECT_INVALID;
#elif defined (_platform_linux)
	int i = -1;
	if(!sema)
	{
		return WQ_INVALID;
	}
	if(time == 0)
	{
		if(sem_trywait(sema) >= 0)
		{
			return WQ_SIGNALED;
		}
		else
		{
			i = errno;
		}
	}
	else if(time == INFINITE)
	{
		if(sem_wait(sema) >= 0)
		{
			return WQ_SIGNALED;
		}
		else
		{
			i = errno;
		}
	}
	else
	{
		struct timespec timespec;
		timeconv_abs(time, &timespec);
		if(sem_timedwait(sema, &timespec) < 0)
		{
			i = errno;
			//printf("errno : %d", errno);
		}
		else
		{
			return  WQ_SIGNALED;
		}
	}

	if(i == EAGAIN ||
			i == ETIMEDOUT)
	{
		return WQ_TIMED_OUT;
	}

	return WQ_FAIL;
#else
#endif
}
void Platform_semaphore_unlock(semaphore *sema)
{
#if defined  (_platform_mingw) || defined(_platform_win)

#elif defined (_platform_linux)
	if(sema) sem_post(sema);
#else
#endif
}

int Platform_criticalsection_open(criticalsection *cs)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return GEINVAL;
#elif defined (_platform_linux)
	return cs ? pthread_mutex_init(cs, NULL) : WQEINVAL;
#else
	return GEINVAL;
#endif
}
void  Platform_criticalsection_close(criticalsection *cs)
{
#if defined  (_platform_mingw) || defined(_platform_win)

#elif defined (_platform_linux)
	if(cs) pthread_mutex_destroy(cs);
#else
#endif
}


_dword Platform_criticalsection_lock(criticalsection *cs, _dword time)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return WQ_INVALID;
#elif defined (_platform_linux)
		int i = -1;
		if(!cs)
		{
			return WQ_INVALID;
		}
		if(time == 0)
			i = pthread_mutex_trylock(cs);
		else if(time == INFINITE)
			i = pthread_mutex_lock(cs);
		else
		{
			struct timespec timeabs;
			timeconv_abs(time, &timeabs);
			i = pthread_mutex_timedlock(cs, &timeabs);
		}
		if(i != 0)
		{
			if(i == EAGAIN || i == ETIMEDOUT) i = WQ_TIMED_OUT;
			else i =WQ_FAIL;
		}
		return i;
#else
	return WQ_INVALID;
#endif
}
void Platform_criticalsection_unlock(criticalsection *cs)
{
#if defined  (_platform_mingw) || defined(_platform_win)

#elif defined (_platform_linux)
	if(cs) pthread_mutex_unlock(cs);
#else
#endif

}

struct _shared_memory *Platform_create_shared_memory(shared_memory_par *par)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return NULL;
#elif defined (_platform_linux)
	struct _shared_memory *sm = (struct _shared_memory *)libwq_malloc(sizeof(struct _shared_memory));
	if(!sm)
	{
		return NULL;
	}
	sm->mapkey = par->mapkey;
	sm->mode = par->flag;
	sm->owner = par->owner;
	sm->size = par->size;
	sm->ptr = NULL;
	sm->key = -1;

	sm->key = shmget(sm->mapkey, sm->size, sm->owner ? (O_CREAT|O_EXCL) | sm->mode : sm->mode);
	if(sm->key < 0)
	{
		libwq_free(sm);
		return NULL;
	}
	sm->ptr = shmat(sm->key, NULL, 0);
	if((void *)-1 == sm->ptr)
	{
		if(sm->owner)
		{
			shmctl(sm->key, IPC_RMID, 0);
		}
		libwq_free(sm);
		return NULL;
	}
	return sm;
#endif
}
void *Platform_shared_memory_reference(struct _shared_memory *sm)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return NULL;
#elif defined (_platform_linux)
	return sm ? sm->ptr : NULL;
#endif
}
int Platform_shared_memory_size(struct _shared_memory *sm)
{
#if defined  (_platform_mingw) || defined(_platform_win)
	return -1;
#elif defined (_platform_linux)
	return sm->size;
#endif
}
void Platform_delete_shared_memory(struct _shared_memory *sm)
{
#if defined  (_platform_mingw) || defined(_platform_win)

#elif defined (_platform_linux)
	if(sm)
	{
		shmdt(sm->ptr);
		if(sm->owner)
		{
			shmctl(sm->key, IPC_RMID, 0);
		}
		libwq_free(sm);
	}
#endif
}

