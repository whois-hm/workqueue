#include "_WQ.h"

WQ_API struct _shared_memory *Shared_memory_create(shared_memory_par *par)
{
	return Platform_create_shared_memory(par);
}
WQ_API void *Shared_memory_reference(struct _shared_memory *sm)
{
	return Platform_shared_memory_reference(sm);
}
WQ_API void Shared_memory_delete(struct _shared_memory *sm)
{
	Platform_delete_shared_memory(sm);
}
WQ_API int Shared_memory_size(struct _shared_memory *sm)
{
	return -1;
}
