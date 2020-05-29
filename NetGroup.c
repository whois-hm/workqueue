#include "_WQ.h"
/*
 	 in this page.
 	 making about NetworkGroup Framework, like observer pattern.
 	 we offer the thread group communication, using workqueue event or notification
 */
struct net_router;
struct net_host;
enum cast
{
	unicast,
	broadcast,
	multicast,
	disconnect,
	closing,
};
struct default_par
{
	unsigned _host;
	unsigned _route_id;
	enum cast _cast;
};

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
	/*
	 	receive the other unicast
	 */
	void (*_recv_from_unicast)(struct net_router *router,
			struct net_host *host,
			void *meta,
			void *expar,
			unsigned exnpar,
			unsigned fromhost);
	/*
	 	 receive the other broadcast
	 */
	void (*_recv_from_broadcast)(struct net_router *router,
			struct net_host *host,
			void *meta,
			void *expar,
			unsigned exnpar,
			unsigned fromhost);
	/*
	 	 receive the other multicast event
	 */
	void (*_recv_from_multicast)(struct net_router *router,
			struct net_host *host,
			void *meta,
			void *expar,
			unsigned exnpar,
			unsigned fromhost,
			unsigned route_id);

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
struct net_host_alarm
{
	struct net_host_alarm *_next;
	unsigned _start_tick;
	unsigned _end_tick;
	unsigned _time;
	int _repeat;
	void (*fn_alarm)(struct net_router *router,
			struct net_host *host,
			void *meta);
};
struct net_host
{
	struct net_host_alarm *_alarms;
	struct net_router *_router;
	struct net_host *_nexthost;
	struct WQ *_queue;
	struct BackGround *_thread;
	struct net_host_register_data _register_data;
	void *_meta;
};
struct net_router
{
	struct net_host *_net_hosts;
	struct net_host ***_routetable;
	criticalsection _routetable_cs;
	unsigned _max_routeindex;
	unsigned _max_routeparsize;
	unsigned _host_len;
};


static unsigned host_parameter_size(unsigned hostpar, unsigned routepar)
{
	unsigned max_par = hostpar > routepar ? hostpar : routepar;
	return sizeof(struct default_par) + max_par;
}
static int host_routing(struct net_host *host, void *_par, unsigned _npar)
{
	struct net_router *router = host->_router;;
	struct default_par *_defpar = NULL;
	unsigned hostparsize = 0;
	_defpar = (struct default_par *)_par;
	hostparsize = _npar - sizeof(struct default_par);
	if(_defpar->_cast == unicast)
	{
		host->_register_data._recv_from_unicast(router,
				host,
				host->_meta,
				hostparsize > 0 ? ((char *)_par) + sizeof(struct default_par) : NULL,
				hostparsize,
				_defpar->_host);
		return 1;

	}
	else if(_defpar->_cast == broadcast)
	{
		host->_register_data._recv_from_broadcast(router,
				host,
				host->_meta,
				hostparsize > 0 ? ((char *)_par) + sizeof(struct default_par) : NULL,
				hostparsize,
				_defpar->_host);
		return 1;
	}
	else if(_defpar->_cast == unicast)
	{
		host->_register_data._recv_from_multicast(router,
				host, host->_meta,
				hostparsize > 0 ? ((char *)_par) + sizeof(struct default_par) : NULL,
				hostparsize,
				_defpar->_host,
				_defpar->_route_id);
		return 1;
	}
	else if(_defpar->_cast == disconnect)
	{
		host->_register_data._register_deinit(router, host, host->_meta, 1);
		return 1;
	}

	/*_defpar->_cast == close*/
	return 0;
}
static unsigned alarm_schdule(struct net_host *host)
{
	unsigned next_alarm = INFINITE;

	unsigned tick = Time_tickcount();
	unsigned flow_tick = 0;
	unsigned remain_tick = 0;
	struct net_host_alarm *alarms = NULL;
	struct net_host_alarm *prev_alarm = NULL;
	struct net_host_alarm *temp_alarm = NULL;

	alarms = host->_alarms;
	while(alarms)
	{
		flow_tick = tick - alarms->_start_tick;
		if(flow_tick >= alarms->_time)
		{
			alarms->fn_alarm(host->_router, host, host->_meta);
			if(alarms->_repeat)
			{
				alarms->_start_tick = tick;
				remain_tick = alarms->_time;
				next_alarm = remain_tick < next_alarm ? remain_tick : next_alarm;
			}
			else
			{
				temp_alarm = alarms->_next;
				if(!prev_alarm)
				{
					host->_alarms = temp_alarm;
				}
				else
				{
					prev_alarm->_next = temp_alarm;
				}
				libwq_free(alarms);

				alarms = temp_alarm;
				continue;
			}
		}
		else
		{
			remain_tick = alarms->_time - flow_tick;
			next_alarm = remain_tick < next_alarm ? remain_tick : next_alarm;
		}
		prev_alarm = alarms;
		alarms = alarms->_next;
	}

	return next_alarm;
}
static void host_start_routine(void *par)
{
	struct net_router *router = NULL;
	void *_par = NULL;
	_dword _npar = 0;
	unsigned res = WQ_FAIL;
	int loop = 1;


	struct net_host *host = (struct net_host *)par;

	router = host->_router;

	host->_register_data._register_init(router, host, &host->_meta, 0);
	host->_register_data._register_init(router, host, &host->_meta, 1);

	while(loop)
	{
		res = WQ_recv(host->_queue,
				(void **)&_par,
				&_npar,
				alarm_schdule(host));
		if(res == WQ_FAIL ||
				res == WQ_INVALID)
		{
			/*host routine break*/
			loop = 0;
		}
		else if(res == WQ_SIGNALED)
		{
			loop = host_routing(host, _par, _npar);
		}
		else
		{
			/*timeout*/
			loop = 1;
		}
	}

	host->_register_data._register_deinit(router, host, host->_meta, 0);
}
static void host_cleanup(struct net_host **host)
{
	struct net_host *h = NULL;
	if(!host ||
			!*host)
	{
		return;
	}
	h = *host;
 	if(h->_thread)BackGround_turnoff(&h->_thread);
	if(h->_queue)WQ_close(&h->_queue);

	libwq_free(h);
	*host = NULL;
}
static int host_in(struct net_router *router,
		struct net_host_register_data register_data)
{
	struct net_host *host = NULL;
	struct net_host *head = NULL;
	int res = -1;
	do
	{
		/*check invalid parameter*/
		if(!router ||
			register_data._max_queuesize <= 0 ||
			register_data._queue_length <=0 ||
			!register_data._register_init ||
			!register_data._register_deinit ||
			(!register_data._recv_from_broadcast &&
			!register_data._recv_from_multicast &&
			!register_data._recv_from_unicast))
		{
			res = WQEINVAL;
			break;
		}
		/*create host*/
		host = (struct net_host *)libwq_malloc(sizeof(struct net_host));
		if(!host)
		{
			res = WQENOMEM;
			break;
		}
		host->_router = router;
		host->_register_data = register_data;
		/*create host's queue*/
		host->_queue = WQ_open(host_parameter_size(host->_register_data._max_queuesize, router->_max_routeparsize),
				host->_register_data._queue_length);
		if(!host->_queue)
		{
			res = WQENODEV;
			break;
		}
		/*link in router*/
		head = router->_net_hosts;
		if(!head)
		{
			router->_net_hosts = host;
		}
		else
		{
			while(head &&
					head->_nexthost)
			{
				head = head->_nexthost;
			}
			head->_nexthost = host;
		}
		return WQOK;
	}while(0);


	host_cleanup(&host);
	return res;
}



void NetGroup_host_unicast(struct net_host *target_host,
		struct net_host *host,
		void *par,
		unsigned npar,
		unsigned flag)
{
	if(!target_host ||
			!host)
	{
		return;
	}
	char dump[sizeof(struct default_par) + npar];
	if(npar)
	{
		memcpy(dump + sizeof(struct default_par), par, npar);
	}
	((struct default_par *)dump)->_host = host->_register_data._host_name;
	((struct default_par *)dump)->_cast = unicast;

	WQ_send(target_host->_queue,
			(void *)dump,
			sizeof(struct default_par) + npar,
			INFINITE, flag);

}
void NetGroup_host_broadcast(struct net_host *host,
		void *par,
		unsigned npar)
{
	struct net_host *hosts = NULL;
	if(!host)
	{
		return;
	}
	char dump[sizeof(struct default_par) + npar];
	if(npar)
	{
		memcpy(dump + sizeof(struct default_par), par, npar);
	}
	((struct default_par *)dump)->_host = host->_register_data._host_name;
	((struct default_par *)dump)->_cast = broadcast;

	hosts = host->_router->_net_hosts;
	while(hosts)
	{
		if(hosts != host)
		{
			WQ_send(hosts->_queue,
						(void *)dump,
						sizeof(struct default_par) + npar,
						INFINITE, 0);
		}
		hosts = hosts->_nexthost;
	}
}

void NetGroup_host_multicast(struct net_host *host,
		unsigned routeindex,
		void *par,
		unsigned npar)
{
	struct net_router*router = NULL;
	int i = 0;
	if(!host)
	{
		return;
	}
	char dump[sizeof(struct default_par) + npar];
	if(npar)
	{
		memcpy(dump + sizeof(struct default_par), par, npar);
	}
	((struct default_par *)dump)->_host = host->_register_data._host_name;
	((struct default_par *)dump)->_cast = multicast;
	((struct default_par *)dump)->_route_id = routeindex;

	router = host->_router;


	if(((struct default_par *)dump)->_route_id >=
			router->_max_routeindex)
	{
		/*reason route index overflow*/
		return;
	}
	for(; i < router->_host_len; i++)
	{
		if(router->_routetable[routeindex][i])
		{
			if(router->_routetable[routeindex][i] != host)
			{
				WQ_send(router->_routetable[routeindex][i]->_queue,
						(void *)dump,
						sizeof(struct default_par) + npar,
						INFINITE, 0);
			}
		}
	}
}


struct net_host *NetGroup_host_connect(struct net_router * router,
		unsigned hostname)
{
	struct net_host *target = NULL;
	if(!router)
	{
		return NULL;
	}
	target = router->_net_hosts;
	while(target)
	{
		if(target->_register_data._host_name == hostname)
		{
			break;
		}
		target = target->_nexthost;
	}
	return target;
}
void NetGroup_host_disconnect(struct net_host **targethost)
{
	if(targethost &&
			*targethost)
	{
		*targethost = NULL;
	}
}
void NetGroup_router_delete(struct net_router ** router)
{
	struct net_host *hosts = NULL;
	struct default_par par;
	int i = 0;
	if(!router ||
			!*router)
	{
		return;
	}
	/*hosts resource unlink*/

	hosts = (*router)->_net_hosts;
	if(hosts)
	{
		par._cast = disconnect;
		par._host = 0;
		par._route_id = 0;
		NetGroup_host_broadcast(hosts, (void *)&par, sizeof(struct default_par));

		par._cast = closing;
		par._host = 0;
		par._route_id = 0;
		NetGroup_host_broadcast(hosts, (void *)&par, sizeof(struct default_par));
	}
	/*hosts resource free*/
	while(hosts)
	{
		(*router)->_net_hosts = hosts->_nexthost;
		host_cleanup(&hosts);
		hosts = (*router)->_net_hosts;
	}
	/*routingtable free*/
	if((*router)->_routetable )
	{
		for(; i < (*router)->_max_routeindex; i++)
		{
			libwq_free((void *)(*router)->_routetable[i]);
		}
		libwq_free((void *)(*router)->_routetable);
	}

	CS_close(&(*router)->_routetable_cs);

	libwq_free((*router));
	*router = NULL;
}
struct net_router *NetGroup_router_create(unsigned max_routeindex,
		unsigned max_routeparsize,
		struct net_host_register_data *phosts,
		unsigned nhosts)
{
	struct net_router *router = NULL;
	int host_len = 0;
	int i = 0;
	struct net_host *_hosts = NULL;
	do
	{
		if(!phosts ||
				nhosts <= 0)
		{
			break;
		}
		router = (struct net_router *)libwq_malloc(sizeof(struct net_router));
		if(!router)
		{
			break;
		}
		router->_host_len = nhosts;
		router->_max_routeparsize = max_routeparsize;
		router->_max_routeindex = max_routeindex;

		/*add hosts*/
		while(host_len < router->_host_len)
		{
			int res = host_in(router, phosts[host_len]);
			if(res != WQOK)
			{
				break;
			}
			host_len++;
		}
		if(host_len != router->_host_len)
		{
			/*any host can't insert to router */
			break;
		}
		/*routing table create if need*/
		if(router->_max_routeindex)
		{
			if(CS_open(&router->_routetable_cs))
			{
				/*can't sync to routing table*/
				break;
			}
			router->_routetable = (struct net_host ***)libwq_malloc(sizeof(struct net_host) * router->_max_routeindex);
			if(!router->_routetable)
			{
				break;
			}
			for(; i < router->_max_routeindex; i++)
			{
				/*now create table*/
				router->_routetable[i] = (struct net_host **)	libwq_malloc(sizeof(struct net_host) * nhosts);
				if(!router->_routetable[i])
				{
					break;
				}
			}
		}

		/*now run hosts*/
		_hosts = router->_net_hosts;
		while(_hosts)
		{
			_hosts->_thread = BackGround_turn(host_start_routine, _hosts);
			_hosts = _hosts->_nexthost;
		}

		return router;
	}while(0);

	NetGroup_router_delete(&router);
	return router;
}

void NetGroup_membership_join(unsigned routeindex,
		struct net_router *router,
		struct net_host *host)
{
	int i = 0;
	int dup = 0;
	do
	{
		if(!router ||
				!host)
		{
			/*reason invalid parameter*/
			break;
		}

		if(routeindex >= router->_max_routeindex)
		{
			/*reason route index overflow*/
			break;
		}

		/* protection route table */
		CS_lock(&router->_routetable_cs, INFINITE);

		/*check duplicated*/
		for(; i < router->_host_len; i++)
		{
			if(router->_routetable[routeindex][i] == host)
			{
				dup = 1;
				break;
			}
		}
		if(dup)
		{
			CS_unlock(&router->_routetable_cs);
			break;
		}
		/*insert*/
		for(; i < router->_host_len; i++)
		{
			if(!router->_routetable[routeindex][i])
			{
				router->_routetable[routeindex][i] = host;
				break;
			}
		}

		CS_unlock(&router->_routetable_cs);

	}while(0);
}
void NetGroup_membership_drop(unsigned routeindex,
		struct net_router *router,
		struct net_host *host)
{
	int i = 0;

	do
	{
		if(!router ||
				!host)
		{
			/*reason invalid parameter*/
			break;
		}

		if(routeindex >= router->_max_routeindex)
		{
			/*reason route index overflow*/
			break;
		}

		/* protection route table */
		CS_lock(&router->_routetable_cs, INFINITE);

		/*find */
		for(; i < router->_host_len; i++)
		{
			if(router->_routetable[routeindex][i] == host)
			{
				router->_routetable[routeindex][i] = NULL;
				break;
			}
		}
		CS_unlock(&router->_routetable_cs);
	}while(0);
}

struct net_host_alarm *NetGroup_host_alarm_start(struct net_host *host,
		void (*fn_alarm)(struct net_router *router, struct net_host *host, void *meta),
		unsigned mstime,
		int repeat)
{
	struct net_host_alarm *alarm = NULL;
	struct net_host_alarm *alarm_head = NULL;
	if(!host ||
			!fn_alarm)
	{
		return NULL;
	}
	alarm = (struct net_host_alarm *)libwq_malloc(sizeof(struct net_host_alarm));
	alarm->_start_tick = Time_tickcount();
	alarm->_end_tick = alarm->_start_tick + mstime;
	alarm->fn_alarm = fn_alarm;
	alarm->_repeat = repeat;

	if(!host->_alarms)
	{
		host->_alarms = alarm;
	}
	else
	{
		alarm_head = host->_alarms;
		while(alarm_head && !alarm_head->_next)
		{
			alarm_head = alarm_head->_next;
		}
		alarm_head->_next = alarm;
	}
	return alarm;
}
void NetGroup_host_alarm_end(struct net_host *host,
		struct net_host_alarm **alarm)
{
	struct net_host_alarm *_alarm = NULL;
	struct net_host_alarm *next = NULL;
	struct net_host_alarm *prev = NULL;
	if(!host ||
			!alarm ||
			!*alarm)
	{
		return;
	}
	_alarm = *alarm;
	*alarm = NULL;


	next = host->_alarms;
	while(next)
	{
		if(next == _alarm)
		{
			if(!prev)
			{
				host->_alarms = next->_next;
			}
			else
			{
				prev->_next = next->_next;;
			}
			break;
		}
		prev = next;
		next = next->_next;
	}
	libwq_free(alarm);
}
