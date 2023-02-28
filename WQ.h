#if defined (__MINGW64__) || defined (__MINGW32__)
#define _platform_mingw
#if defined (__MINGW64__)
#define _platform_mingw64
#else
#define _platform_mingw32
#endif
#elif defined (_WIN32 ) || defined (_WIN64)
#define _platform_win
#if defined (_WIN32)
#define _platform_win32
#else
#define _platform_win64
#endif
#else
#if defined (__GNUC__)
#define _platform_linux
#if defined (__x86_64__) || defined(__ppc64__)
#define _platform_linux64
#else
#define _platform_linux32
#endif
#endif
#endif

#if defined (__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#if !defined(BOOL)
#define BOOL int
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined (FALSE)
#define FALSE 0
#endif

#if defined (_platform_mingw) || defined(_platform_win)
#if defined (WQ_API_EXPORT)
#define WQ_API  EXTERN_C __declspec(dllexport)
#else
#define WQ_API EXTERN_C __declspec(dllimport)
#endif
#else
#if defined (WQ_API_EXPORT)
#define WQ_API
#else
#define WQ_API EXTERN_C
#endif
#endif
#if !defined NULL
# ifdef __cplusplus
#  define NULL 0
# else
#  define NULL ((void *)0)
# endif
#endif
#define libwq_heap_testmode
#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#if defined (_platform_linux)
#include <stdarg.h>
#include <inttypes.h>
#include <math.h>
#include <signal.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <mqueue.h>
#include  <execinfo.h>
#include  <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include  <sys/vtimes.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/videodev2.h>
#include <sys/shm.h>
#include <sys/un.h>
#elif defined  (_platform_mingw) || defined(_platform_win32)
#endif

typedef unsigned short _word;
typedef unsigned int _dword;

#if defined  (_platform_mingw) || defined(_platform_win32)
typedef HANDLE semaphore;
typedef HANDLE criticalsection;
typedef _dword	threadid;
#elif defined (_platform_linux)
typedef sem_t semaphore;
typedef pthread_mutex_t criticalsection;
typedef pthread_t	threadid;


typedef struct
{
	char const *key;
	unsigned int initval;
	int mode;
	int owner;
}semaphore_named_par;

#define _declare_sema_named_par_owner(val, key, init, mode)\
	semaphore_named_par val={key, init, mode, 1}

#define _declare_sema_named_par(val, key)\
		semaphore_named_par val={key, 0, 0, 0};

typedef struct
{
	key_t mapkey;
	size_t size;
	int flag;
	int owner;
}shared_memory_par;
#endif


#if !defined (NULL)
#define NULL ((void *)0)
#endif
#if !defined (INFINITE)
#define INFINITE	(_dword)0xffffffffL
#endif

#if defined (_platform_linux)
#define WAIT_OBJECT_0				(_dword)0x00000000L
#define WAIT_FAILED					(_dword)0xffffffffL
#define WAIT_TIMEOUT				(_dword)0x00000102L
#define WAIT_ABANDONED		(_dword)0x00000080L
#endif

#define WQ_SIGNALED 		WAIT_OBJECT_0
#define WQ_FAIL						WAIT_FAILED
#define WQ_TIMED_OUT 		WAIT_TIMEOUT
#define WQ_INVALID 			WAIT_ABANDONED

#define SQUARE(x) 											((x) * (x))
#define BIT_SET(byte_dst, bit)					( byte_dst |= bit )
#define BIT_CLEAR(byte_dst, bit)			( byte_dst &= (~bit))
#define IS_BIT_SET(byte_dst, bit)			( ( byte_dst & bit ))
#define DIM(x) 													((int)(sizeof(x) / sizeof(x[0])))
#define X_ABS(x)        										( ((x)<0)?-(x):(x) )

/*Success*/
#define WQOK					 0
/*Not Enough Memory*/
#define WQENOMEM 		1
/*Invalid Parameter*/
#define WQEINVAL			2
/*No Such Device*/
#define WQENODEV		3
/*Resource Busy*/
#define WQEBUSY			4
/*Bad Descriptor*/
#define WQEBADF			5
/*Connection Timeout*/
#define WQETIMEDOUT 6
/*Already Use*/
#define WQEALREADY 7

#define WQ_SEND_FLAGS_BLOCK	(1 << 0)


struct WQ;
struct BackGround;
struct Dictionary;
struct net_router;
struct net_host;
struct net_host_alarm;
struct _named_semaphore;
struct _shared_memory;
struct wq_ipc;
typedef void (*BackGround_function)(void *p);
struct net_host_register_data
{
	/*
	 	 in host meta data create
	 	 return : host's meta data
	 */

	void (*_register_init)(struct net_router *router,
			struct net_host *host,
			void **meta,
			int connect);
	/* receive the other unicast */
	void (*_recv_from_unicast)(struct net_router *router,
			struct net_host *host,
			void *meta,
			void *expar,
			unsigned exnpar,
			int fromhost);
	/* receive the other broadcast */
	void (*_recv_from_broadcast)(struct net_router *router,
			struct net_host *host,
			void *meta,
			void *expar,
			unsigned exnpar,
			int fromhost);
	/* receive the other multicast event */
	void (*_recv_from_multicast)(struct net_router *router,
			struct net_host *host,
			void *meta,
			void *expar,
			unsigned exnpar,
			unsigned fromhost,
			int route_id);

	/*
	 	 host meta data delete
	 	 void *meta : in "_register_init" function return pointer
	 */
	void (*_register_deinit)(struct net_router *router,
			struct net_host *host,
			void *meta,
			int disconnect);
	unsigned _max_queuesize;
	unsigned _queue_length;
	unsigned _host_name;
};
struct _router_message_unicast_mapper
{
	unsigned _host_name;
	int _bcast;
};
typedef unsigned (*_router_message_decoder)(const void *read_buffer,
		unsigned read_len,
		char const *fromip,
		int fromport,
		void *pcopy,
		unsigned ncopy,
		unsigned *outlen,
		struct _router_message_unicast_mapper *mappers,
		unsigned mapper_max);

struct _router_message_buffer
{
	char *_p;
	unsigned _n;
};
typedef int (*_router_message_encoder)(void *par,
		unsigned npar,
		struct _router_message_buffer *b);



/*----------------------------------------------------------------------------------------------------------
 * local workqueue
 ----------------------------------------------------------------------------------------------------------*/
WQ_API struct WQ *WQ_open(_dword size, _dword length);
WQ_API void WQ_close(struct WQ **pwq);
WQ_API _dword WQ_recv(struct WQ *wq, void **ppar, _dword *pnpar, _dword time);
WQ_API _dword WQ_send(struct WQ *wq, void *par, _dword npar, _dword time, int flags);
WQ_API int WQ_parameter_size(struct WQ *wq);
WQ_API int WQ_parameter_length(struct WQ *wq);

/*----------------------------------------------------------------------------------------------------------
 * ipc workqueue
 ----------------------------------------------------------------------------------------------------------*/
WQ_API struct wq_ipc *WQIpc_open_owner(unsigned int key, unsigned int size, unsigned int length);
WQ_API struct wq_ipc *WQIpc_open(unsigned int key, unsigned int size, unsigned int length);
WQ_API void WQIpc_close(struct wq_ipc **pwqipc);
WQ_API _dword WQIpc_owner_recv(struct wq_ipc *wqipc, void **par, _dword time);
WQ_API _dword WQIpc_send(struct wq_ipc *wqipc, void *par, _dword npar, _dword time, int flags);

/*----------------------------------------------------------------------------------------------------------
 * shared memory
 ----------------------------------------------------------------------------------------------------------*/
WQ_API struct _shared_memory *Shared_memory_create(shared_memory_par *par);
WQ_API void *Shared_memory_reference(struct _shared_memory *sm);
WQ_API int Shared_memory_size(struct _shared_memory *sm);
WQ_API void Shared_memory_delete(struct _shared_memory *sm);

/*----------------------------------------------------------------------------------------------------------
 * thread
 ----------------------------------------------------------------------------------------------------------*/
WQ_API struct BackGround *BackGround_turn(BackGround_function fn, void *p);
WQ_API void BackGround_turnoff(struct BackGround **pb);
WQ_API threadid BackGround_id();

/*----------------------------------------------------------------------------------------------------------
 * time
 ----------------------------------------------------------------------------------------------------------*/
WQ_API _dword Time_tickcount();
WQ_API void Time_sleep(_dword time);

/*----------------------------------------------------------------------------------------------------------
 * dictionary
 ----------------------------------------------------------------------------------------------------------*/
WQ_API struct Dictionary *Dictionary_create();
WQ_API void Dictionary_delete(struct Dictionary **pd);
WQ_API _dword Dictionary_add_char(struct Dictionary *d, char *k, char *v);
WQ_API _dword Dictionary_add_int(struct Dictionary *d, char *k, int v);
WQ_API void Dictionary_remove(struct Dictionary *d, char *k);
WQ_API char *Dictionary_refchar(struct Dictionary *d, char *k);
WQ_API char *Dictionary_allocchar(struct Dictionary *d, char *k);
WQ_API void Dictionary_freechar(char **allocchar);
WQ_API int Dictionary_copyint(struct Dictionary *d, char *k);
WQ_API int Dictionary_haskey(struct Dictionary *d, char *k);

/*----------------------------------------------------------------------------------------------------------
 * criticalsection
 ----------------------------------------------------------------------------------------------------------*/
WQ_API int CS_open(criticalsection *cs);
WQ_API _dword CS_lock(criticalsection *cs, _dword time);
WQ_API void CS_unlock(criticalsection *cs);
WQ_API void CS_close(criticalsection *cs);

/*----------------------------------------------------------------------------------------------------------
 * local semphore
 ----------------------------------------------------------------------------------------------------------*/
WQ_API int SEMA_open(semaphore *sem, _dword i, _dword m);
WQ_API _dword SEMA_lock(semaphore *sem, _dword time);
WQ_API void SEMA_unlock(semaphore *sem);
WQ_API void SEMA_close(semaphore *sem);

/*----------------------------------------------------------------------------------------------------------
 * proccess semaphore
 ----------------------------------------------------------------------------------------------------------*/
WQ_API struct _named_semaphore *SEMA_named_open(semaphore_named_par *par);
WQ_API void SEMA_named_close(struct _named_semaphore *nsem);
WQ_API _dword SEMA_named_lock(struct _named_semaphore *nsem, _dword time);
WQ_API void SEMA_named_unlock(struct _named_semaphore *nsem);

/*----------------------------------------------------------------------------------------------------------
 * heap memory
 ----------------------------------------------------------------------------------------------------------*/
WQ_API void libwq_heap_testinit();
WQ_API void libwq_heap_testdeinit();
WQ_API void *libwq_malloc(size_t size);
WQ_API void libwq_free(void *mem);
WQ_API void libwq_print_heap();

/*----------------------------------------------------------------------------------------------------------
 * smart pointer
 ----------------------------------------------------------------------------------------------------------*/
WQ_API void SP_using();
WQ_API void *SP_malloc(size_t size, int refer);
WQ_API void *SP_ref(void *spmem);
WQ_API void SP_unref(void *spmem);
WQ_API void SP_free(void *spmem);
WQ_API void SP_unused();


/*----------------------------------------------------------------------------------------------------------
 * workqueue framwork
 ----------------------------------------------------------------------------------------------------------*/
WQ_API struct net_router *NetGroup_router_create(unsigned max_routeindex,
		unsigned max_routeparsize,
		struct net_host_register_data *phosts,
		unsigned nhosts,
		_router_message_decoder message_decoder,
		int message_port,
		unsigned message_buffer_size);
WQ_API void NetGroup_router_delete(struct net_router ** router);
WQ_API struct net_host *NetGroup_host_connect(struct net_router * router,
		unsigned hostname);
WQ_API void NetGroup_host_disconnect(struct net_host **targethost);
WQ_API void NetGroup_host_unicast(struct net_host *target_host,
		struct net_host *host,
		void *par,
		unsigned npar,
		unsigned flag);
WQ_API void NetGroup_host_broadcast(struct net_host *host,
		void *par,
		unsigned npar);
WQ_API void NetGroup_host_multicast(struct net_host *host,
		unsigned routeindex,
		void *par,
		unsigned npar);
WQ_API int NetGroup_router_send_message(struct net_router *router,
		char const *ip,
		int port,
		void *par,
		unsigned npar,
		_router_message_encoder encoder);
WQ_API void NetGroup_router_send_message_buffer_realloc(struct _router_message_buffer *b,
		unsigned want,
		int clear);
WQ_API void NetGroup_membership_join(unsigned routeindex,
		struct net_router *router,
		struct net_host *host);
WQ_API void NetGroup_membership_drop(unsigned routeindex,
		struct net_router *router,
		struct net_host *host);
WQ_API struct net_host_alarm *NetGroup_host_alarm_start(struct net_host *host,
		void (*fn_alarm)(struct net_router *router, struct net_host *host, void *meta),
		unsigned mstime,
		int repeat);
WQ_API void NetGroup_host_alarm_end(struct net_host *host,
		struct net_host_alarm **alarm);


/*----------------------------------------------------------------------------------------------------------
 * datastructures
 ----------------------------------------------------------------------------------------------------------*/
#define __check_address(address) 					if(address == NULL){return;}
#define __check_address_and_return(address, value) 	if(address == NULL){return value;}
#define __to_void(element) 							((void *)element)
#define __to_type(type, _void)						((type *)(_void));

typedef BOOL (*_bool_function)(void *ptr, void *par);
typedef void *(*_clone_function)(void *ptr);
typedef void (*_clear_function)(void *ptr);
typedef void (*_void_function)(void *ptr);

#define __ds_element_alloc(type)	({	\
	type *ptr = (type *)malloc(sizeof(type));	\
	if(ptr) memset((void *)ptr, 0, sizeof(type));	\
	ptr;	\
})
#define __ds_element_free(ptr) ({	\
		if(ptr) free(ptr);	\
		ptr = NULL;	\
})

/*----------------------------------------------------------------------------------------------------------
 * list
 ----------------------------------------------------------------------------------------------------------*/
#define __declare_list_element _list_element _declare_list_element;
#define __to_list_element(_void) 	((_list_element *)_void)
typedef struct __list_element
{
	struct __list_element *next;
}_list_element;

WQ_API void _list_link(void **head, void *ptr);
WQ_API void *_list_find(void **head, _bool_function func, void *par);
WQ_API void *_list_unlink(void **head, _bool_function func, void *par);
WQ_API void _list_clone(void **head, void **clone_head, _clone_function func);
WQ_API void _list_clear(void **head, _clear_function func);
WQ_API void _list_look(void **head, _void_function func);
WQ_API unsigned int _list_length(void **head);
WQ_API BOOL _list_empty(void **head);
#define _list_for(type, head, current) \
		current = head; _list_element *ptr_next = (current ? __to_list_element(current)->next : NULL); \
		for(;  current; (current = (type *)ptr_next), (ptr_next = (current ? __to_list_element(current)->next : NULL)))

/*----------------------------------------------------------------------------------------------------------
 * double list
 ----------------------------------------------------------------------------------------------------------*/
#define __declare_doublelist_element _doublelist_element _declare_doublelist_element;
#define __to_doublelist_element(_void) ((_doublelist_element *)_void)
typedef struct __doublelist_element
{
	struct __doublelist_element *next;
	struct __doublelist_element *previous;
}_doublelist_element;
typedef enum
{
	doublelist_direction_next,
	doublelist_direction_previous
}doublelist_direction;

WQ_API void _doublelist_link(void **head, void *ptr);
WQ_API void *_doublelist_unlink(void **head, _bool_function func, void *par);
WQ_API void *_doublelist_find(void *ptr, _bool_function func, void *par, doublelist_direction dir);
WQ_API void _doublelist_clear(void **head, _clear_function func);
WQ_API unsigned int _doublelist_length(void **head);
WQ_API BOOL _doublelist_empty(void **head);
WQ_API void _doublelist_look(void **head, _void_function func);
WQ_API void *doublelist_previous(void *ptr);
WQ_API void *doublelist_next(void *ptr);

/*----------------------------------------------------------------------------------------------------------
 * stack
 ----------------------------------------------------------------------------------------------------------*/
#define __declare_stack_element _stack_element _declare_stack_element;
#define __to_stack_element(_void) ((_stack_element *)(_void))
typedef struct __stack_element
{
	struct __stack_element *previous;
}_stack_element;

WQ_API void _stack_push(void **head, void *ptr);
WQ_API void *_stack_pop(void **head);
WQ_API unsigned int _stack_length(void **head);
WQ_API BOOL _stack_empty(void **head);
WQ_API void _stack_clear(void **head, _clear_function func);
WQ_API void _stack_look(void **head, _void_function func);


/*----------------------------------------------------------------------------------------------------------
 * queue
 ----------------------------------------------------------------------------------------------------------*/
#define __declare_queue_element _queue_element _declare_queue_element;
#define __to_queue_element(_void) ((_queue_element *)(_void))
typedef struct __queue_element
{
	struct __queue_element *next;
}_queue_element;

WQ_API void _queue_push(void **head, void *ptr);
WQ_API void *_queue_pop(void **head);
WQ_API unsigned int _queue_length(void **head);
WQ_API BOOL _queue_empty(void **head);
WQ_API void _queue_clear(void **head, _clear_function func);
WQ_API void _queue_look(void **head, _void_function func);

/*----------------------------------------------------------------------------------------------------------
 * tree
 ----------------------------------------------------------------------------------------------------------*/
#define declare_tree_element _tree_element _declare_tree_element;
#define __to_tree_element(_void) ((_tree_element *)(_void))
typedef struct __tree_element
{
	struct __tree_element *parent;
	struct __tree_element *child;
	struct __tree_element *sibling;
}_tree_element;

WQ_API void _tree_link(void **root, void *parent, void *ptr);
WQ_API void *_tree_unlink(void **root, void *parent, _bool_function func, void *par);
WQ_API void *_tree_find(void *parent, _bool_function func, void *par);
WQ_API void _tree_clear(void **parent, _clear_function func);
WQ_API BOOL _tree_have_child(void *parent);
WQ_API void *_tree_parent(void *ptr);
WQ_API void *_tree_next_child(void *parent, void *ptr);
WQ_API void _tree_look(void *parent, _void_function func);


/*----------------------------------------------------------------------------------------------------------
 * language
 ----------------------------------------------------------------------------------------------------------*/
typedef enum
{
	_lan_utf8,
	_lan_utf16le,
	_lan_utf16be,
	_lan_max,
	_lan_unknown = _lan_max
}_lan;
WQ_API unsigned int _lan_transcode_require_length(void *base, _lan baselan, _lan tolan);
WQ_API BOOL _lan_transcode_copy(void *base, void *to, unsigned int to_len, _lan baselan, _lan tolan);
WQ_API void *_lan_transcode(void *base, _lan baselan, _lan tolan);
WQ_API void _lan_free(void *ptr);
WQ_API BOOL _lan_convert_string_to_hex(void *base,
		void *to,
		unsigned int to_len,
		_lan baselan,
		BOOL base_prefix);
WQ_API unsigned int _lan_convert_hex_to_string_require_length(void *base, unsigned int base_len, _lan outlan, BOOL prefix);
WQ_API BOOL _lan_convert_hex_to_string_copy(void *base, void *to, unsigned int base_len, unsigned int to_len, _lan outlan, BOOL prefix);
WQ_API void *_lan_convert_hex_to_string(void *base, unsigned int base_len, _lan outlan, BOOL prefix);

#define _debug_buffer_size  1024
typedef enum
{
	_debug_level_notice = 1 << 0,
	_debug_level_warnning = 1 << 1,
	_debug_level_critical = 1 << 2,
	_debug_level_all = (_debug_level_notice | _debug_level_warnning | _debug_level_critical),
	_debug_level_caution = (_debug_level_warnning | _debug_level_critical)
}_debug_level;



WQ_API void _debug_init(unsigned char  lv);
WQ_API void _debug_deinit();
WQ_API void _debug_register_prefix(char const *prefix);
WQ_API void _debug_register_file(char const *path);
WQ_API void _debug_register_redirection(void (*function)(char *msg));
WQ_API void _debug_register_console();
WQ_API void _debug_register_net(char const *ip, unsigned short port);
WQ_API BOOL _debug_register_net_server(char const *ip, unsigned short port, void (*function)(char const *ip, char *msg));
WQ_API void _debug(_debug_level level, char const *msg, ...);
WQ_API void _debug_memory(_debug_level level, void *mem, _dword size);
#define _debug_position(lv) _debug(lv, "%s(%s)\n", __FUNCTION__, __LINE__);
#define _debug_declare_tick_elapse_measure(from) _dword _tick_elapse_measure = Time_tickcount();\
												 char const *_tick_elapse_from = from;

#define _debug_tick_elapse_measure_check(lv, to)	\
		do{	\
		_dword _tick_elapse_measure_check = Time_tickcount();	\
		_debug(lv, "tick_elapse_measure_check %s <-> %s = %ums\n", 	\
				_tick_elapse_from, 	\
				to, 	\
				_tick_elapse_measure_check - _tick_elapse_measure);	\
		_tick_elapse_measure = _tick_elapse_measure_check;	\
		_tick_elapse_from = to; \
		}while(0);



