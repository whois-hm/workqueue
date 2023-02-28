#include "_WQ.h"
struct sp
{
	struct sp *_next_sp;
	unsigned _ref;
};
struct g_sp
{
	struct sp *_link;
	criticalsection _syn;
	int _use;
};
static struct g_sp _g_sp;

WQ_API void SP_using()
{
	if(!_g_sp._use)
	{
		if(!CS_open(&_g_sp._syn))
		{
			_g_sp._use = 1;
			_g_sp._link = NULL;
		}
	}
}
WQ_API void *SP_malloc(size_t size, int refer)
{
	void *sp_ptr = NULL;
	struct sp *sp_block = NULL;
	if(!_g_sp._use ||
			size <= 0 ||
			refer <= 0)
	{
		/*invalid parameter recv*/
		return NULL;
	}

	sp_ptr = libwq_malloc(sizeof(struct sp) + size);
	if(!sp_ptr)
	{
		return NULL;
	}
	/*increase referenced count by user offered value*/
	((struct sp *)sp_ptr)->_ref = refer;

	/*linking*/
	CS_lock(&_g_sp._syn, INFINITE);

	sp_block = _g_sp._link;
	if(!sp_block)
	{
		_g_sp._link = (struct sp *)sp_ptr;
	}
	else
	{
		while(sp_block && !sp_block->_next_sp)
		{
			sp_block = sp_block->_next_sp;
		}
		sp_block->_next_sp = (struct sp *)sp_ptr;
	}

	CS_unlock(&_g_sp._syn);

	/*return user pointer*/
	return ((char *)sp_ptr) + sizeof(struct sp);
}
WQ_API void *SP_ref(void *spmem)
{
	struct sp * sp_ptr = NULL;
	struct sp *sp_block = NULL;
	if(!_g_sp._use ||
			!spmem)
	{
		return NULL;
	}

	/*find in linkedlist*/
	sp_ptr = (struct sp *)(((char *)spmem) - sizeof(struct sp));

	/*control reference*/
	CS_lock(&_g_sp._syn, INFINITE);
	sp_block = _g_sp._link;
	while(sp_block)
	{
		if(sp_block == sp_ptr)
		{
			break;
		}
		sp_block = sp_block->_next_sp;
	}
	if(sp_block)
	{
		sp_block->_ref++;
	}
	CS_unlock(&_g_sp._syn);

	return spmem;
}
WQ_API void SP_unref(void *spmem)
{
	struct sp * sp_ptr = NULL;
	struct sp *sp_block = NULL;
	struct sp *sp_prev_block = NULL;
	int bfree = 0;
	if(!_g_sp._use ||
			!spmem)
	{
		return;
	}

	sp_ptr = (struct sp *)(((char *)spmem) - sizeof(struct sp));
	/*control reference*/
	CS_lock(&_g_sp._syn, INFINITE);
	sp_block = _g_sp._link;
	while(sp_block)
	{
		if(sp_block == sp_ptr)
		{
			if(!sp_prev_block)
			{
				_g_sp._link = sp_block->_next_sp;
			}
			else
			{
				sp_prev_block->_next_sp = sp_block->_next_sp;
			}
			break;
		}
		sp_prev_block = sp_block;
		sp_block = sp_block->_next_sp;
	}
	if(!sp_block)
	{
		CS_unlock(&_g_sp._syn);
		return;
	}
	if(sp_block->_ref > 0)
	{
		if(--sp_ptr->_ref <= 0)
		{
			bfree = 1;
		}
	}
	CS_unlock(&_g_sp._syn);
	if(bfree && sp_block)
	{
		libwq_free(sp_block);
	}
}
WQ_API void SP_free(void *spmem)
{
	SP_unref(spmem);
}
WQ_API void SP_unused()
{
	if(_g_sp._use)
	{
		_g_sp._link = NULL;
		CS_close(&_g_sp._syn);
		_g_sp._use = 0;
	}
}
