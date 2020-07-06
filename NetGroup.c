#include "_WQ.h"
/*
 	 in this page.
 	 making about NetworkGroup Framework, like observer pattern.
 	 we offer the thread group communication, using workqueue event or notification
 */

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
struct _router_message_unicast_to
{
	struct _router_message_unicast_to *next_to;
	int _sock;
	criticalsection _cs;
	struct _router_message_buffer _messagebuffer;
	struct sockaddr_in _in;
};
struct net_router
{
	struct net_host *_net_hosts;
	struct net_host ***_routetable;
	criticalsection _routetable_cs;
	unsigned _max_routeindex;
	unsigned _max_routeparsize;
	unsigned _host_len;
	struct
	{
		struct BackGround *_thread;
		struct _router_message_unicast_mapper *_cast_indexer;
		int _port;
		int _sock;
		int _pipe[2];
		_router_message_decoder _decoder;
		unsigned buffersize;
		char *pbuffer;
		void *_parameter_copy;
		unsigned _parameter_copy_len;
	}message_server;
	struct _router_message_unicast_to *_message_tos;
	criticalsection _to_cs;

};
void NetGroup_host_unicast(struct net_host *target_host,
		struct net_host *host,
		void *par,
		unsigned npar,
		unsigned flag);
static void message_server_start_routine(void *par)
{
	struct net_router *router = (struct net_router *)par;
	int cursor = 0;
	struct sockaddr_in clientaddr;
	socklen_t clientaddr_size = sizeof(clientaddr);

	fd_set readfds;
	fd_set exceptfds;
	int maxfd;
	int nread = 0;
	unsigned outlen = 0;
	unsigned decoder_res = 0;
	struct net_host *hosts = NULL;
	int index = 0;
	while(1)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		maxfd = -1;

		FD_SET(router->message_server._pipe[0], &readfds);
		FD_SET(router->message_server._sock, &readfds);
		FD_SET(router->message_server._pipe[0], &exceptfds);
		FD_SET(router->message_server._sock, &exceptfds);
		maxfd = router->message_server._sock > router->message_server._pipe[0] ?
				router->message_server._sock :
				router->message_server._pipe[0];

		if(select(maxfd + 1, &readfds, NULL, &exceptfds, NULL) <= 0)
		{
			/*error. down server*/
			break;
		}

		if(FD_ISSET(router->message_server._pipe[0], &exceptfds)/*pipe exception */ ||
				FD_ISSET(router->message_server._sock, &exceptfds) /*socket exception*/||
				FD_ISSET(router->message_server._pipe[0], &readfds) /*pipe reading, this mean we are done*/)
		{
			break;
		}
		if(FD_ISSET(router->message_server._sock, &readfds))
		{

			nread = recvfrom(router->message_server._sock,
					router->message_server.pbuffer + cursor,
					router->message_server.buffersize - cursor,
					0,
					(struct sockaddr *)&clientaddr,
					&clientaddr_size);
			if(nread <= 0 ||
					(nread > (router->message_server.buffersize - cursor)))
			{
				/*reading error */
				break;
			}

			cursor += nread;

			/*notify to user*/
			decoder_res = router->message_server._decoder(router->message_server.pbuffer,
					nread,
					inet_ntoa(clientaddr.sin_addr),
					ntohs(clientaddr.sin_port),
					router->message_server._parameter_copy,
					router->message_server._parameter_copy_len,
					&outlen,
					router->message_server._cast_indexer,
					router->_host_len);

			if(outlen)
			{
				/*loop list. we save the array index by list, just find index*/
				hosts = router->_net_hosts;
				index = 0;
				while(hosts)
				{
					if(router->message_server._cast_indexer[index]._bcast)
					{
						if(outlen <= (WQ_parameter_size(hosts->_queue) -
								sizeof(struct default_par)))
						{
							NetGroup_host_unicast(hosts,
									NULL/*-404*/,
									router->message_server._parameter_copy,
									outlen,
									0/*send no sync*/);
						}
						/*index return*/
						router->message_server._cast_indexer[index]._bcast = 0;
					}
					hosts = hosts->_nexthost;
					index++;
				}
				outlen = 0;
				memset(router->message_server._parameter_copy,
						0,
						router->message_server._parameter_copy_len);
			}

			/*delete the data, using from user*/
			if(decoder_res)
			{
				/*max condition*/
				if(decoder_res >= cursor)
				{
					decoder_res = cursor;
				}
				if(cursor - decoder_res > 0)
				{
					memcpy(router->message_server.pbuffer,
							router->message_server.pbuffer + decoder_res,
							cursor - decoder_res);
				}
				cursor -= decoder_res;
			}
			/*user callback no return the processed length.  for next buffer reading, data shift necessary*/
			if(cursor >= router->message_server.buffersize)
			{
				memcpy(router->message_server.pbuffer,
						router->message_server.pbuffer + router->message_server.buffersize / 2,
						router->message_server.buffersize / 2);

				cursor = router->message_server.buffersize / 2;
			}

		}
	}
}

static unsigned host_parameter_size(unsigned hostpar, unsigned routepar)
{
	unsigned max_par = hostpar > routepar ? hostpar : routepar;
	return sizeof(struct default_par) + max_par;
}
static int host_routing(struct net_host *host, void *_par, unsigned _npar)
{
	struct net_router *router = host->_router;
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
	if(!target_host)
	{
		return;
	}
	char dump[sizeof(struct default_par) + npar];
	if(npar)
	{
		memcpy(dump + sizeof(struct default_par), par, npar);
	}
	((struct default_par *)dump)->_host = host  ? host->_register_data._host_name : -404;
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
WQ_API void NetGroup_router_send_message_buffer_realloc(struct _router_message_buffer *b,
		unsigned want,
		int clear)
{
	if(!b)
	{
		return;
	}

	if(want)
	{
		if(want > b->_n)
		{
			if(b->_p)
			{
				libwq_free(b->_p);
			}
			b->_p = (char *)libwq_malloc(want);
			b->_n = want;
		}
	}

	if(clear > 0 && b->_p)
	{
		memset(b->_p, 0, b->_n);
	}
}
WQ_API int NetGroup_router_send_message(struct net_router *router,
		char const *ip,
		int port,
		void *par,
		unsigned npar,
		_router_message_encoder encoder)
{
	unsigned sendlen = 0;
	unsigned sendlen_s = 0;
	unsigned sendlen_t = 0;
	unsigned sendedlen = 0;
	struct _router_message_unicast_to *to;
	struct _router_message_unicast_to *new_to;
	uint32_t uip = 0;
	uint16_t uport = 0;
	if(!router ||
			!ip ||
			port <= 0 ||
			!par ||
			npar <= 0 ||
			!encoder)
	{
		return -1;
	}
	uip = inet_addr(ip);
	uport  = htons(port);

	CS_lock(&router->_to_cs, INFINITE);

	to = router->_message_tos;
	while(to)
	{
		if(uip == to->_in.sin_addr.s_addr &&
				uport == to->_in.sin_port)
		{
			break;
		}
		to = to->next_to;
	}
	if(!to)
	{
		new_to = (struct _router_message_unicast_to *)libwq_malloc(sizeof(struct _router_message_unicast_to));
		new_to->_in.sin_family = AF_INET;
		new_to->_in.sin_port = uport;
		new_to->_in.sin_addr.s_addr = uip;
		new_to->_sock = socket(PF_INET, SOCK_DGRAM, 0);
		if(new_to->_sock < 0)
		{
			libwq_free(new_to);
			CS_unlock(&router->_to_cs);
			return -1;
		}
		if(CS_open(&new_to->_cs))
		{
			libwq_free(new_to);
			CS_unlock(&router->_to_cs);
			return -1;
		}
		to = router->_message_tos;
		if(!to)
		{
			router->_message_tos = new_to;
		}
		else
		{
			while(to && to->next_to)
			{
				to = to->next_to;
			}
			to->next_to = new_to;
		}
		to = new_to;
	}
	CS_unlock(&router->_to_cs);

	CS_lock(&to->_cs, INFINITE);
	sendlen_s = sendlen = encoder(par, npar, &to->_messagebuffer);
	sendedlen = 0;
	while(sendlen > 0)
	{
		sendlen_t = sendto(to->_sock,
				to->_messagebuffer._p + sendedlen,
				sendlen - sendedlen,
				MSG_DONTWAIT | MSG_NOSIGNAL,
				(struct sockaddr *)&to->_in,
				sizeof(to->_in));
		if(sendlen_t <= 0)
		{
			break;
		}
		sendedlen += sendlen_t;
		sendlen -= sendlen_t;
	}

	CS_unlock(&to->_cs);
	if(sendlen_s == sendedlen)
	{
		return 1;
	}
	return 0;

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
	struct _router_message_unicast_to *tos = NULL;
	struct default_par par;
	int i = 0;
	char dump = 1;
	if(!router ||
			!*router)
	{
		return;
	}
	/*uninstall message server if loaded*/
	if((*router)->message_server._thread)
	{
		write((*router)->message_server._pipe[1], &dump, sizeof(char));
		BackGround_turnoff(&(*router)->message_server._thread);
		close((*router)->message_server._sock);
		close((*router)->message_server._pipe[0]);
		close((*router)->message_server._pipe[1]);
		libwq_free((*router)->message_server.pbuffer);
	}
	if((*router)->message_server._parameter_copy)
	{
		libwq_free((*router)->message_server._parameter_copy);
	}

	/*hosts resource unlink*/
	hosts = (*router)->_net_hosts;
	while(hosts)
	{
		par._cast = disconnect;
		WQ_send(hosts->_queue,
					(void *)&par,
					sizeof(struct default_par),
					INFINITE, 0);

		hosts = hosts->_nexthost;
	}
	/*hosts close*/
	hosts = (*router)->_net_hosts;
	while(hosts)
	{
		par._cast = closing;
		WQ_send(hosts->_queue,
					(void *)&par,
					sizeof(struct default_par),
					INFINITE, 0);

		hosts = hosts->_nexthost;
	}
	/*hosts resource free*/
	while(hosts)
	{
		(*router)->_net_hosts = hosts->_nexthost;
		host_cleanup(&hosts);
		hosts = (*router)->_net_hosts;
	}

	/*router message client delete*/
	tos = (*router)->_message_tos;
	while(tos)
	{
		(*router)->_message_tos = tos->next_to;
		close(tos->_sock);
		if(tos->_messagebuffer._n)
		{
			libwq_free(tos->_messagebuffer._p);
		}
		CS_close(&tos->_cs);
		libwq_free(tos);
		tos = (*router)->_message_tos;
	}
	CS_close(&(*router)->_to_cs);


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
		unsigned nhosts,
		_router_message_decoder message_decoder,
		int message_port,
		unsigned message_buffer_size)
{
	/*
	 	 local group -
	 	 message send -
	 	 message receive -
	 */
	struct net_router *router = NULL;
	int host_len = 0;
	int i = 0;
	struct net_host *_hosts = NULL;
	struct sockaddr_in   server_addr;
	int opt = 0;
	unsigned max_copylen = 0;
	do
	{
		router = (struct net_router *)libwq_malloc(sizeof(struct net_router));
		if(!router)
		{
			break;
		}
		router->_host_len = nhosts;
		router->_max_routeparsize = max_routeparsize;
		router->_max_routeindex = max_routeindex;

		/*add hosts if reqeust*/
		if(phosts && nhosts > 0)
		{
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
		}

		/*create message server if request*/
		if(message_decoder &&
				message_port &&
				message_buffer_size)
		{
			router->message_server._port = message_port;
			router->message_server._decoder = message_decoder;
			router->message_server.buffersize = message_buffer_size;

			/*buffer setup*/
			router->message_server.pbuffer = (char *)libwq_malloc(router->message_server.buffersize);
			if(!router->message_server.pbuffer)
			{
				break;
			}

			/*indexer setup*/
			router->message_server._cast_indexer = (struct _router_message_unicast_mapper *)libwq_malloc(sizeof(struct _router_message_unicast_mapper) * router->_host_len);
			if(!router->message_server._cast_indexer)
			{
				break;
			}

			_hosts = router->_net_hosts;
			i = 0;
			while(_hosts)
			{
				max_copylen = max_copylen < WQ_parameter_size(_hosts->_queue) ? WQ_parameter_size(_hosts->_queue) :  max_copylen;
				router->message_server._cast_indexer[i++]._host_name = _hosts->_register_data._host_name;
				_hosts = _hosts->_nexthost;
			}
			if(max_copylen)
			{
				router->message_server._parameter_copy_len = max_copylen-sizeof(struct default_par);
				router->message_server._parameter_copy = libwq_malloc(router->message_server._parameter_copy_len);
				if(!router->message_server._parameter_copy)
				{
					break;
				}
			}


			/*pipe setup*/
			if(pipe(router->message_server._pipe) < 0)
			{
				break;
			}

			/*socket setup*/

			router->message_server._sock = socket(PF_INET, SOCK_DGRAM, 0);
			if(router->message_server._sock < 0)
			{
				break;
			}

			memset(&server_addr, 0, sizeof(struct sockaddr_in));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(message_port);
			server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

			if(bind(router->message_server._sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
			{
				break;
			}

			opt = fcntl(router->message_server._sock, F_GETFL);
			if(opt < 0)
			{
				break;
			}
			opt = (O_NONBLOCK | O_RDWR);
			if(fcntl(router->message_server._sock, F_SETFL, opt) < 0)
			{
				break;
			}
		}



		/*now run hosts*/
		_hosts = router->_net_hosts;
		while(_hosts)
		{
			_hosts->_thread = BackGround_turn(host_start_routine, _hosts);
			_hosts = _hosts->_nexthost;
		}

		/*now run message server if request*/
		if(message_decoder &&
				message_port &&
				message_buffer_size)
		{
			router->message_server._thread = BackGround_turn(message_server_start_routine, router);
		}
		/*create the route sendmessage syncronize*/
		CS_open(&router->_to_cs);

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
