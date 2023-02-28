#include "_WQ.h"
typedef struct
{
	__declare_list_element
	char const *file;
}_debug_par_file;
typedef struct
{
	__declare_list_element
	char const *ip;
	unsigned short port;
}_debug_par_net;
typedef struct
{
	__declare_list_element
	void (*_f_redirection)(char *msg);
}_debug_par_redirection;
typedef struct
{
	BOOL buse;
}_debug_par_console;
typedef struct
{
	unsigned char lv;
	char const *prefix;
	_debug_par_file *_file;
	_debug_par_net *_net;
	_debug_par_redirection *_redirection;
	_debug_par_console _console;
	struct
	{
		int pipe[2];
		int sock;
		struct BackGround *th;
		void (*function)(char const *ip, char *msg);
	}server;

}_debug_par;
_debug_par *_dpar;

static void _to_file(char *msg)
{
	_debug_par_file *cursor;
	_list_for(_debug_par_file, _dpar->_file, cursor)
	{
		FILE *file = fopen(cursor->file, "ab");
		if(file)
		{
			fwrite(msg, strlen(msg), sizeof(char), file);
			fflush(file);
			fclose(file);
		}
	}
}
static void _to_net(char *msg)
{
	_debug_par_net *cursor;
	_list_for(_debug_par_net, _dpar->_net, cursor)
	{
		int sock = -1;
		struct sockaddr_in _serveraddr;
		sock = socket(PF_INET, SOCK_DGRAM, 0);
		if(sock < 0)
		{
			continue;
		}
		memset(&_serveraddr, 0, sizeof(_serveraddr));
		_serveraddr.sin_family = AF_INET;
		if(cursor->ip)	_serveraddr.sin_addr.s_addr = inet_addr(cursor->ip);
		else 			_serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		_serveraddr.sin_port = htons(cursor->port);

		sendto(sock, msg, strlen(msg)+1, MSG_DONTWAIT | MSG_NOSIGNAL,
				(struct sockaddr *)&_serveraddr,
				sizeof(_serveraddr));
		close(sock);
	}
}
static void _to_redirection(char *msg)
{
	_debug_par_redirection *cursor;
	_list_for(_debug_par_redirection, _dpar->_redirection, cursor)
	{
		cursor->_f_redirection(msg);
	}
}
static void _to_console(char *msg)
{
	if(_dpar &&
			_dpar->_console.buse)
	{
		printf("%s", msg);
	}
}

WQ_API void _debug_init(unsigned char  lv)
{
	if(!_dpar)
	{
		_dpar = (_debug_par *)malloc(sizeof(_debug_par));
		memset(_dpar, 0, sizeof(_debug_par));
		_dpar->lv = lv;
		_dpar->server.pipe[0] = -1;
		_dpar->server.pipe[1] = -1;
		_dpar->server.sock = -1;
	}
}
static void __clear_function(void *ptr)
{
	__ds_element_free(ptr);
}
WQ_API void _debug_deinit()
{
	if(_dpar)
	{
		if(_dpar->server.th)
		{
			char a = 1;
			write(_dpar->server.pipe[1], &a, sizeof(char));
			BackGround_turnoff(&_dpar->server.th);
			close(_dpar->server.pipe[0]);
			close(_dpar->server.pipe[1]);
			close(_dpar->server.sock);
		}

		_list_clear((void **)&_dpar->_file, __clear_function);
		_list_clear((void **)&_dpar->_net, __clear_function);
		_list_clear((void **)&_dpar->_redirection, __clear_function);
		free(_dpar);
		_dpar = NULL;
	}
}

WQ_API void _debug_register_prefix(char const *prefix)
{
	if(_dpar)
	{
		_dpar->prefix = prefix;
	}
}
WQ_API void _debug_register_file(char const *path)
{
	if(_dpar && path)
	{
		_debug_par_file *f = __ds_element_alloc(_debug_par_file);
		f->file = path;
		_list_link((void **)&_dpar->_file, f);
	}
}
WQ_API void _debug_register_net(char const *ip, unsigned short port)
{
	if(_dpar)
	{
		_debug_par_net *n = __ds_element_alloc(_debug_par_net);
		n->ip = ip;
		n->port = port;
		_list_link((void **)&_dpar->_net, n);
	}
}
WQ_API void _debug_register_redirection(void (*function)(char *msg))
{
	if(_dpar && function)
	{
		_debug_par_redirection *r = __ds_element_alloc(_debug_par_redirection);
		r->_f_redirection = function;
		_list_link((void **)&_dpar->_redirection, r);
	}
}
WQ_API void _debug_register_console()
{
	if(_dpar)
	{
		_dpar->_console.buse = TRUE;
	}
}
void server_routin(void *par)
{
	_debug_par *ptr = (_debug_par *)par;
	fd_set readfds;
	fd_set exceptfds;
	int fd_max = ptr->server.sock > ptr->server.pipe[0] ? ptr->server.sock : ptr->server.pipe[0];
	while(fd_max > 0)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		FD_SET(ptr->server.sock, &readfds);
		FD_SET(ptr->server.sock, &exceptfds);
		FD_SET(ptr->server.pipe[0], &readfds);
		FD_SET(ptr->server.pipe[0], &exceptfds);
		int ret = select(fd_max, &readfds, NULL, &exceptfds, NULL);
		if(ret < 0 ||
				FD_ISSET(ptr->server.pipe[0], &readfds) ||
				FD_ISSET(ptr->server.pipe[0], &exceptfds) ||
				FD_ISSET(ptr->server.sock, &exceptfds))
		{
			break;
		}
		if(FD_ISSET(ptr->server.sock, &readfds))
		{
			struct sockaddr_in clientaddr;
			int clientaddr_size = sizeof(clientaddr);
			char buffer[_debug_buffer_size] = {0, };
			int res = recvfrom(ptr->server.sock,
					buffer,
					_debug_buffer_size,
				MSG_NOSIGNAL,
				(struct sockaddr *)&clientaddr,
				(socklen_t  *)&clientaddr_size);
			if(res > 0)
			{
				buffer[_debug_buffer_size - 1]= 0;
				ptr->server.function(inet_ntoa(clientaddr.sin_addr), buffer);
			}
		}
	}
}
WQ_API BOOL _debug_register_net_server(char const *ip, unsigned short port, void (*function)(char const *ip, char *msg))
{
	int sock = -1;
	int opt = -1;
	int _pipe[2] = {-1, -1};
	do
	{
		if(!function ||
				!_dpar)
		{
			break;
		}
		if(_dpar->server.th)
		{
			break;
		}
		struct sockaddr_in server_addr;
		sock = socket(PF_INET, SOCK_DGRAM, 0);
		memset(&server_addr, 0, sizeof(struct sockaddr_in));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		if(ip)server_addr.sin_addr.s_addr = inet_addr(ip);
		else server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		opt = fcntl(sock, F_GETFL);
		if(opt < 0)
		{
			break;
		}
		opt = (O_NONBLOCK | O_RDWR);
		opt = fcntl(sock, F_SETFL);
		if(opt < 0)
		{
			break;
		}
		if(bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		{
			break;
		}
		if(pipe(_pipe) < 0)
		{
			break;
		}

		_dpar->server.th = BackGround_turn(server_routin, _dpar);
		if(!_dpar->server.th)
		{
			break;
		}
		_dpar->server.sock = sock;
		_dpar->server.pipe[0] = _pipe[0];
		_dpar->server.pipe[1] = _pipe[1];
		_dpar->server.function = function;
		return TRUE;
	}while(0);

	if(_pipe[0] >= 0) close(_pipe[0]);
	if(_pipe[1] >= 0) close(_pipe[1]);
	if(sock >= 0) close(sock);
	BackGround_turnoff(&_dpar->server.th);

	return FALSE;
}
WQ_API void _debug(_debug_level level,  char const *msg, ...)
{
	if(_dpar && BIT_SET(level, _dpar->lv))
	{
		char buffer[_debug_buffer_size] = {0, };
		char *_pbuffer = buffer;
		unsigned int prefix_len = 0;

		if(_dpar->prefix)
		{
			prefix_len = strlen(_dpar->prefix);
			memcpy(_pbuffer, _dpar->prefix, prefix_len < _debug_buffer_size-1 ? prefix_len : _debug_buffer_size - 1);
		}
		va_list s;
		va_start(s, msg);
		if(prefix_len < _debug_buffer_size - 1)
		{
			vsnprintf(_pbuffer + prefix_len, _debug_buffer_size - prefix_len, msg, s);
		}
		buffer[_debug_buffer_size - 1] = 0;
		va_end(s);
		_to_file(buffer);
		_to_net(buffer);
		_to_redirection(buffer);
		_to_console(buffer);
	}
}
WQ_API void _debug_memory(_debug_level level, void *mem, _dword size)
{

#define CONV_PRINT_ASCII(v)	(	((v >= 0x20) && (v <= 0x7e))	)
#define PVH(c) 				(	*(pBuffer + (nBf_Cs + c))		)
#define PVC(c)  			(CONV_PRINT_ASCII(*(pBuffer + (nBf_Cs + c))) ? PVH(c) : 0x2e)

	_dword nBf_Cs = 0;
	int nTot_Line = 0;
	int nTot_Rest = 0;
	unsigned char *pBuffer 			= (unsigned char *)mem;
	char RestBuffer[100] 	= {0, };
	char *prest = RestBuffer;
	int i = 0;
	int arch_bit_len = (sizeof(void *) <= 4) ? 0 : 6;

	if(!pBuffer || size <= 0)
	{
		return;
	}

	nTot_Line = (size	>>	4);
	nTot_Rest = (size 	&	0x0000000F);

	if(sizeof(void *) <= 4)
	{
		_debug(level, "===========================================================================\n");
		_debug(level, "offset h  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  Size [%uByte]\n", size);
		_debug(level, "---------------------------------------------------------------------------\n");
	}
	else
	{
		_debug(level, "=================================================================================\n");
		_debug(level, "offset h        00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  Size [%uByte]\n", size);
		_debug(level, "---------------------------------------------------------------------------------\n");
	}


	while(nTot_Line-- > 0)
	{
		_debug(level, "%p  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
				(void *)(pBuffer + nBf_Cs),
				PVH(0),		 PVH(1), 	PVH(2), 	PVH(3), 	PVH(4), 	PVH(5), 	PVH(6), 	PVH(7),
				PVH(8), 	 PVH(9), 	PVH(10), 	PVH(11), 	PVH(12), 	PVH(13), 	PVH(14), 	PVH(15),
				PVC(0),		 PVC(1),	PVC(2),		PVC(3),		PVC(4),		PVC(5),		PVC(6),		PVC(7),
				PVC(8),		 PVC(9),	PVC(10),	PVC(11),	PVC(12),	PVC(13),	PVC(14),	PVC(15));
				nBf_Cs += 16;
	}


	if(nTot_Rest)
	{
		memset((void *)(RestBuffer), 0x20, DIM(RestBuffer));
		sprintf(RestBuffer, "%p", (void *)(pBuffer + nBf_Cs));
		RestBuffer[strlen(RestBuffer)] = 0x20;

		for(; i < nTot_Rest; i++)
		{
			sprintf(prest + arch_bit_len + (10 + ((i << 1) + i)), "%02x", PVH(0));
			*(prest + arch_bit_len + (10 + ((i << 1) + i)) + 2) = 0x20;

			sprintf(prest + arch_bit_len + (59 + i), "%c", PVC(0));
			if(((i + 1) == (nTot_Rest)))
			{
				*(prest + arch_bit_len + (59 + i) + 1) = 0x0a;
				*(prest + arch_bit_len + (59 + i) + 2) = 0x00;
				break;
			}
			nBf_Cs++;
		}
		_debug(level, "%s", RestBuffer);
	}
	if(sizeof(void *) <= 4)
	{
		_debug(level, "---------------------------------------------------------------------------\n");
	}
	else
	{
		_debug(level, "---------------------------------------------------------------------------------\n");
	}

}

