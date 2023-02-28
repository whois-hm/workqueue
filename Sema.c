#include "_WQ.h"
WQ_API int SEMA_open(semaphore *sem, _dword i, _dword m)
{
	return Platform_semephore_open(sem, i, m);
}
WQ_API _dword SEMA_lock(semaphore *sem, _dword time)
{
	return Platform_semaphore_lock(sem, time);
}
WQ_API void SEMA_unlock(semaphore *sem)
{
	return Platform_semaphore_unlock(sem);
}
WQ_API void SEMA_close(semaphore *sem)
{
	return Platform_semaphore_close(sem);
}
WQ_API struct _named_semaphore *SEMA_named_open(semaphore_named_par *par)
{
	return Platform_semaphore_named_open(par);
}
WQ_API void SEMA_named_close(struct _named_semaphore *nsem)
{
	Platform_semaphore_named_close(nsem);
}
WQ_API _dword SEMA_named_lock(struct _named_semaphore *nsem, _dword time)
{
	return Platform_semaphore_lock(nsem->sem, time);
}
WQ_API void SEMA_named_unlock(struct _named_semaphore *nsem)
{
	Platform_semaphore_unlock(nsem->sem);
}
