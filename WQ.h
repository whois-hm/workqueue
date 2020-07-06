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
typedef unsigned long _dword;

#if defined  (_platform_mingw) || defined(_platform_win32)
typedef HANDLE semaphore;
typedef HANDLE criticalsection;
typedef _dword	threadid;
#elif defined (_platform_linux)
typedef sem_t semaphore;
typedef pthread_mutex_t criticalsection;
typedef pthread_t	threadid;
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



WQ_API struct WQ *WQ_open(_dword size, _dword length);
WQ_API void WQ_close(struct WQ **pwq);
WQ_API _dword WQ_recv(struct WQ *wq, void **ppar, _dword *pnpar, _dword time);
WQ_API _dword WQ_send(struct WQ *wq, void *par, _dword npar, _dword time, int flags);
WQ_API int WQ_parameter_size(struct WQ *wq);
WQ_API int WQ_parameter_length(struct WQ *wq);

WQ_API struct BackGround *BackGround_turn(BackGround_function fn, void *p);
WQ_API void BackGround_turnoff(struct BackGround **pb);
WQ_API threadid BackGround_id();
WQ_API void Printf_memory(void *mem, _dword size);

WQ_API _dword Time_tickcount();
WQ_API void Time_sleep(_dword time);

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

WQ_API int CS_open(criticalsection *cs);
WQ_API _dword CS_lock(criticalsection *cs, _dword time);
WQ_API void CS_unlock(criticalsection *cs);
WQ_API void CS_close(criticalsection *cs);

WQ_API int SEMA_open(semaphore *sem, _dword i, _dword m);
WQ_API _dword SEMA_lock(semaphore *sem, _dword time);
WQ_API void SEMA_unlock(semaphore *sem);
WQ_API void SEMA_close(semaphore *sem);


WQ_API void libwq_heap_testinit();
WQ_API void libwq_heap_testdeinit();
WQ_API void *libwq_malloc(size_t size);
WQ_API void libwq_free(void *mem);
WQ_API void libwq_print_heap();

WQ_API void SP_using();
WQ_API void *SP_malloc(size_t size, int refer);
WQ_API void *SP_ref(void *spmem);
WQ_API void SP_unref(void *spmem);
WQ_API void SP_free(void *spmem);
WQ_API void SP_unused();


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
