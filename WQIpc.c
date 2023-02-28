#include "_WQ.h"

static struct _named_semaphore *make_semaphore(char const *prefix, int index, int initval, int bowner)
{
	char val[256];
	memset(val, 0, 256);
	sprintf(val, "%s%d", prefix, index);

	if(bowner)
	{
		_declare_sema_named_par_owner(semtemp, val, initval, 666);
		return Platform_semaphore_named_open(&semtemp);
	}
	_declare_sema_named_par(semtemp, val);
	return Platform_semaphore_named_open(&semtemp);
}
void threshold(struct wq_ipc *wqipc);
struct wq_ipc *_WQIpc_open(unsigned int key, unsigned int size, unsigned int length, int bowner)
{
	struct wq_ipc *wqi = NULL;
	unsigned int sharedmap_size = 0;
	int ack_fail = 0;

	do
	{
		if(size <= 0 ||
				length <= 0)
		{
			break;
		}
		wqi = (struct wq_ipc *)libwq_malloc(sizeof(struct wq_ipc));
		if(!wqi)
		{
			break;
		}
		wqi->sq = size;
		wqi->nq = length;
		wqi->key = key;
		wqi->bowner = bowner;
		sharedmap_size = sizeof(int) +	//nq
				sizeof(int) + 	//sq
				sizeof(int) + //head
				sizeof(int) +	//tail
				sizeof(int) + //state
				(sizeof(char) * wqi->nq) +	//sem ack flag
				(wqi->sq * wqi->nq) + 	//user data size
				sizeof(char) * wqi->nq; // reading flag;

		wqi->mutex = make_semaphore("wqi_mutex", wqi->key, 1, wqi->bowner);
		if(!wqi->mutex)
		{
			break;
		}

		Platform_semaphore_lock(wqi->mutex->sem, INFINITE);

		shared_memory_par par;
		par.flag = 666;
		par.mapkey = wqi->key;
		par.owner = wqi->bowner;
		par.size = sharedmap_size;
		wqi->sm = Platform_create_shared_memory(&par);
		if(!wqi->sm)
		{
			break;
		}
		if(wqi->bowner)
		{
			memset(Platform_shared_memory_reference(wqi->sm),
					0,
					Platform_shared_memory_size(wqi->sm));
		}
		else
		{
			int *nq = (int *)Platform_shared_memory_reference(wqi->sm);
			int *sq = (int *)(((char *)nq) + sizeof(int));
			if(*nq != wqi->nq ||
					*sq != wqi->sq)
			{
				break;
			}
		}
		wqi->reader = make_semaphore("wqi_reader", wqi->key, 0, wqi->bowner);
		if(!wqi->reader)
		{
			break;
		}
		wqi->writer = make_semaphore("wqi_writer", wqi->key, wqi->nq, wqi->bowner);
		if(!wqi->writer)
		{
			break;
		}
		wqi->ack = (struct _named_semaphore **)libwq_malloc(sizeof(struct _named_semaphore*) * wqi->nq);
		if(!wqi->ack)
		{
			break;
		}

		for(int i = 0; i < wqi->nq && !ack_fail; i++)
		{
			*(wqi->ack + i) = make_semaphore("wq_ack", wqi->key, 0, wqi->bowner);
			if(*(wqi->ack + i) == NULL)
			{
				ack_fail = 1;
			}
		}
		if(ack_fail)
		{
			break;
		}
		wqi->nq_ptr = (int *)Platform_shared_memory_reference(wqi->sm);
		wqi->sq_ptr = (int *)(((char *)(wqi->nq_ptr)) + sizeof(int));
		wqi->cursor_head_ptr = (int *)(((char *)(wqi->sq_ptr)) + sizeof(int));
		wqi->cursor_tail_ptr = (int *)(((char *)(wqi->cursor_head_ptr)) + sizeof(int));
		wqi->state = (int *)(((char *)(wqi->cursor_tail_ptr)) + sizeof(int));
		wqi->_sem_flag_ptr = (unsigned char *)(((char *)(wqi->state)) + sizeof(int));
		wqi->_userevent_ptr = (void *)(((char *)wqi->_sem_flag_ptr) +  (sizeof(char) * wqi->nq));
		if(wqi->bowner)
		{
			*wqi->nq_ptr = wqi->nq;
			*wqi->sq_ptr = wqi->sq;
			*wqi->state = 1;
		}
		Platform_semaphore_unlock(wqi->mutex->sem);
		return wqi;
	}while(0);

	return NULL;
}
WQ_API struct wq_ipc *WQIpc_open_owner(unsigned int key, unsigned int size, unsigned int length)
{
	return _WQIpc_open(key, size, length, 1);
}

WQ_API struct wq_ipc *WQIpc_open(unsigned int key, unsigned int size, unsigned int length)
{
	return _WQIpc_open(key, size, length, 0);
}
WQ_API void WQIpc_close(struct wq_ipc **pwqipc)
{
	if(pwqipc &&
			*pwqipc)
	{
		if((*pwqipc)->bowner)
		{
			threshold(*pwqipc);
		}
		Platform_semaphore_named_close((*pwqipc)->writer);
		Platform_semaphore_named_close((*pwqipc)->reader);
		Platform_semaphore_named_close((*pwqipc)->mutex);

		for(int i = 0; (*pwqipc)->nq > 0 &&
			(*pwqipc)->ack &&
			i < (*pwqipc)->nq ;
			i++)
		{
			Platform_semaphore_named_close(*((*pwqipc)->ack + i));
		}
		Platform_delete_shared_memory((*pwqipc)->sm);

		free((*pwqipc));
		*pwqipc = NULL;
	}

}
WQ_API _dword WQIpc_owner_recv(struct wq_ipc *wqipc, void **par, _dword time)
{
	_dword res = WQ_SIGNALED;
	if(!wqipc ||
			!par ||
			!wqipc->bowner)
	{
		return WQEINVAL;
	}
	Platform_semaphore_lock(wqipc->mutex->sem, INFINITE);

	char *userevent = ((char *)wqipc->_userevent_ptr) +
			((wqipc->sq) * (*wqipc->cursor_head_ptr)) +
			(sizeof(char) * (*wqipc->cursor_head_ptr));
	if(0 == (*userevent))
	{
		Platform_semaphore_unlock(wqipc->mutex->sem);
		goto recv;
	}
	if((*(wqipc->_sem_flag_ptr + (*wqipc->cursor_head_ptr))) == 1)
	{
		Platform_semaphore_unlock((*(wqipc->ack + (*wqipc->cursor_head_ptr)))->sem);
		(*(wqipc->_sem_flag_ptr + (*wqipc->cursor_head_ptr))) = 0;
	}
	(*userevent) = 0;
	(*wqipc->cursor_head_ptr) = q_iNext((*wqipc->cursor_head_ptr), (wqipc->nq));
	Platform_semaphore_unlock(wqipc->mutex->sem);
	Platform_semaphore_unlock(wqipc->writer->sem);
recv:
	res = Platform_semaphore_lock(wqipc->mutex->sem, time);
	if(res != WQ_SIGNALED)
	{
		return res;
	}
	Platform_semaphore_lock(wqipc->mutex->sem, INFINITE);
	char *_rd = ((char *)wqipc->_userevent_ptr) +
			((wqipc->sq) * (*wqipc->cursor_head_ptr)) +
			(sizeof(char) * (*wqipc->cursor_head_ptr));

	*par = (void *)((_rd) + sizeof(char));
	*_rd = 1;
	Platform_semaphore_unlock(wqipc->mutex->sem);
	return 0;
}

void threshold(struct wq_ipc *wqipc)
{
	unsigned int time = 300;
	void *d;
	Platform_semaphore_lock(wqipc->mutex->sem, INFINITE);
	*wqipc->state = 0;
	Platform_semaphore_unlock(wqipc->mutex->sem);

	while(WQIpc_owner_recv(wqipc, &d, time) == WQ_SIGNALED){}
}
WQ_API _dword WQIpc_send(struct wq_ipc *wqipc, void *par, _dword npar, _dword time, int flags)
{
	struct _named_semaphore *ack = NULL;
	_dword res = WQ_SIGNALED;
	if(!wqipc ||
			!par ||
			npar <= 0 ||
			npar > wqipc->sq ||
			wqipc->bowner)
	{
		return WQEINVAL;
	}
	res = Platform_semaphore_lock(wqipc->writer->sem, time);
	if(res != WQ_SIGNALED)
	{
		return res;
	}

	Platform_semaphore_lock(wqipc->mutex->sem,  INFINITE);
	if((*wqipc->state) <= 0)
	{
		Platform_semaphore_unlock(wqipc->mutex->sem);
		Platform_semaphore_unlock(wqipc->writer->sem);
		return WQEBADF;
	}

	void *usrevent = ((char *)wqipc->_userevent_ptr) + ((wqipc->sq) * (*wqipc->cursor_tail_ptr)) + (sizeof(char) * (*wqipc->cursor_tail_ptr));
	memset((((char *)usrevent) + sizeof(char)), 0, (wqipc->sq));
	memcpy((((char *)usrevent) + sizeof(char)), par, npar);
	if(IS_BIT_SET(flags, WQ_SEND_FLAGS_BLOCK))
	{
		ack = (*(wqipc->ack + (*wqipc->cursor_tail_ptr)));
		*(wqipc->_sem_flag_ptr + (*wqipc->cursor_tail_ptr)) = 1;
	}

	(*wqipc->cursor_tail_ptr) = q_iNext((*wqipc->cursor_tail_ptr), (wqipc->nq));
	Platform_semaphore_unlock(wqipc->mutex->sem);
	Platform_semaphore_unlock(wqipc->reader->sem);

	if(ack)
	{
		Platform_semaphore_lock(ack->sem, INFINITE);
		memcpy(par, (((char *)usrevent) + sizeof(char)), npar);
	}

	return WQ_SIGNALED;
}
