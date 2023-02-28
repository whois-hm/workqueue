#include "_WQ.h"
WQ_API _dword Time_tickcount()
{
	return Platform_tick_count();
}
WQ_API void Time_sleep(_dword time)
{
	Platform_sleep(time);
}

WQ_API struct Dictionary *Dictionary_create()
{
	struct Dictionary *dict = NULL;
	dict = (struct Dictionary *)libwq_malloc(sizeof(struct Dictionary));
	if(!dict)
	{
		return NULL;
	}
	dict->kvs = NULL;

	return dict;
}
WQ_API void Dictionary_delete(struct Dictionary **pd)
{
	struct Dictionary *dict = NULL;
	if(!pd ||
			!(!pd))
	{
		return;
	}
	dict = *pd;
	*pd = NULL;

	struct Keyvalue *cur = dict->kvs;
	struct Keyvalue *next = NULL;
	while(cur)
	{
		next = cur->next;
		if(cur->k) libwq_free(cur->k);
		if(cur->vc) libwq_free(cur->vc);
		libwq_free(cur);
		cur = next;
	}
	libwq_free(dict);
}
WQ_API _dword Dictionary_add_int(struct Dictionary *d, char *k, int v)
{
	struct Keyvalue *kv = NULL;
	struct Keyvalue *cur = NULL;
	int dup = 0;
	_dword ret = WQOK;
	if(!d ||
			!k)
	{
		return WQEINVAL;
	}

	do
	{
		kv = (struct Keyvalue *)libwq_malloc(sizeof(struct Keyvalue));
		if(!kv)
		{
			ret = WQENOMEM;
			break;
		}

		memset(kv, 0, sizeof(struct Keyvalue));
		if(k)
		{
			kv->k = strdup(k);
			if(!kv->k)
			{
				ret = WQENOMEM;
				break;
			}
		}
		kv->vi = v;

		cur = d->kvs;
		while(cur)
		{
			if(!strcmp(cur->k, k))
			{
				dup = 1;
				break;
			}
			cur = cur->next;
		}
		if(dup)
		{
			ret = WQEALREADY;
			break;
		}

		cur = d->kvs;
		if(!cur)
		{
			d->kvs = kv;
		}
		else
		{
			while(cur->next)
			{
				cur = cur->next;
			}
			cur->next = kv;
		}
		return ret;
	}while(0);

	if(kv)
	{
		if(kv->k)libwq_free(kv->k);
		if(kv->vc)libwq_free(kv->vc);
		libwq_free(kv);
		kv = NULL;
	}
	return ret;
}
WQ_API _dword Dictionary_add_char(struct Dictionary *d, char *k, char *v)
{
	struct Keyvalue *kv = NULL;
	struct Keyvalue *cur = NULL;
	int dup = 0;
	_dword ret = WQOK;

	if(!d ||
			!k)
	{
		return WQEINVAL;
	}

	do
	{
		kv = (struct Keyvalue *)libwq_malloc(sizeof(struct Keyvalue));
		if(!kv)
		{
			ret = WQENOMEM;
			break;
		}
		memset(kv, 0, sizeof(struct Keyvalue));

		if(k)
		{
			kv->k = strdup(k);
			if(!kv->k)
			{
				ret = WQENOMEM;
				break;
			}
		}
		if(v)
		{
			kv->vc = strdup(v);
			if(!kv->vc)
			{
				ret = WQENOMEM;
				break;
			}
		}

		cur = d->kvs;
		while(cur)
		{
			if(!strcmp(cur->k, k))
			{
				dup = 1;
				break;
			}
			cur = cur->next;
		}
		if(dup)
		{
			ret = WQEALREADY;
			break;
		}
		cur = d->kvs;
		if(!cur)
		{
			d->kvs = kv;
		}
		else
		{
			while(cur->next)
			{
				cur = cur->next;
			}
			cur->next = kv;
		}
		return ret;
	}while(0);

	if(kv)
	{
		if(kv->k)libwq_free(kv->k);
		if(kv->vc)libwq_free(kv->vc);
		libwq_free(kv);
		kv = NULL;
	}
	return ret;
}
WQ_API void Dictionary_remove(struct Dictionary *d, char *k)
{
	struct Keyvalue *cur = NULL;
	struct Keyvalue *prev = NULL;
	if(!d ||
			!k)
	{
		return;
	}

	cur = d->kvs;

	while(cur)
	{
		if(!strcmp(cur->k, k))
		{
			if(!prev) d->kvs = cur->next;
			else prev->next = cur->next;

			if(cur->k)libwq_free (cur->k);
			if(cur-> vc)libwq_free (cur->vc);
			libwq_free(cur);
		}
		prev = cur;
		cur = cur->next;
	}
}
WQ_API char *Dictionary_refchar(struct Dictionary *d, char *k)
{
	struct Keyvalue *cur = NULL;
	char *ref = NULL;
	if(!d ||
			!k)
	{
		return NULL;
	}

	cur = d->kvs;

	while(cur)
	{
		if(!strcmp(cur->k, k))
		{
			ref = cur->vc;
			break;
		}
		cur = cur->next;
	}
	return ref;

}
WQ_API char *Dictionary_allocchar(struct Dictionary *d, char *k)
{
	struct Keyvalue *cur = NULL;
	char *alloc = NULL;
	if(!d ||
			!k)
	{
		return NULL;
	}
	cur = d->kvs;

	while(cur)
	{
		if(!strcmp(cur->k, k))
		{
			if(cur->vc)
			{
				alloc = strdup(cur->vc);
			}
			break;
		}
		cur = cur->next;
	}
	return alloc;
}
WQ_API void Dictionary_freechar(char **allocchar)
{
	if(allocchar ||
			*allocchar)
	{
		char *a = *allocchar;
		libwq_free(a);
		*allocchar = NULL;
	}
}
WQ_API int Dictionary_copyint(struct Dictionary *d, char *k)
{
	struct Keyvalue *cur = NULL;
	int copy = 0;
	if(!d ||
			!k)
	{
		return 0;
	}

	cur = d->kvs;

	while(cur)
	{
		if(!strcmp(cur->k, k))
		{
			copy = cur->vi;
			break;
		}
		cur = cur->next;
	}
	return copy;
}
WQ_API int Dictionary_haskey(struct Dictionary *d, char *k)
{
	struct Keyvalue *cur = NULL;
	int ret = -1;
	if(!d ||
			!k)
	{
		return ret;
	}

	cur = d->kvs;

	while(cur)
	{
		if(!strcmp(cur->k, k))
		{
			ret = 1;
			break;
		}
		cur = cur->next;
	}
	return ret;
}
