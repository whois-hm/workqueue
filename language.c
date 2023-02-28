#include "_WQ.h"


static unsigned char *_lan_alloc(unsigned int size)
{
	void *ptr = NULL;
	if(size > 0)
	{
		ptr = malloc(size);
		if(ptr)
		{
			memset(ptr, 0, size);
			return (unsigned char *)ptr;
		}
	}
	return NULL;
}

/*------------------------------------------------------------------
 * transcoding in : invalid out : invalid
 ------------------------------------------------------------------*/
static unsigned int _lan_invalid_len(void *invalid) { return 0; }
static BOOL _lan_invalid_copy(void *invalid1, void *invalid2, unsigned int invalid3) { return FALSE; }
static void *_lan_invalid_transcode(void *invalid) { return NULL; }
/*------------------------------------------------------------------
 * transcoding in : utf8 out : utf16le
 ------------------------------------------------------------------*/
static unsigned int _lan_utf8_to_utf16le_len(void *utf8)
{
	unsigned int len = 0;
	unsigned char *_utf8 = (unsigned char *)utf8;

	while(_utf8 && *_utf8)
	{
		if(*_utf8 <= 0x7f)
			_utf8 += 1;
		else if(*_utf8 <= 0xdf)
			_utf8 += 2;
		else
			_utf8 += 3;
		len++;
	}

	if(len > 0)
	{
		len = len * sizeof(unsigned short);
		len += 2;
	}
	return len;
}

static BOOL _lan_utf8_to_utf16le_copy(void *utf8, void *utf16, unsigned int utf16_len)
{
	unsigned char *_utf8 = (unsigned char *)utf8;
	unsigned char *_utf16 = (unsigned char *)utf16;
	unsigned char fbyte = 0x00;
	unsigned char sbyte = 0x00;
	unsigned char tbyte = 0x00;
	if(!utf8 || !utf16)
	{
		return FALSE;
	}
	if(utf16_len > 0)
	{
		if((_lan_utf8_to_utf16le_len(utf8) != utf16_len))
		{
			return FALSE;
		}
	}

	while((*_utf8) != 0)
	{
		if((*_utf8) <= 0x7f)
		{
			fbyte = *_utf8++;
			sbyte = 0x00;
			(*_utf16++) = fbyte;
			(*_utf16++) = sbyte;
		}
		else if((*_utf8) <= 0xdf)
		{
			fbyte = *_utf8++;
			sbyte = *_utf8++;
			(*_utf16++) = ((fbyte & 0x03) << 6) | (sbyte & 0x3f);
			(*_utf16++) = (fbyte & 0x1c) >> 2;
		}
		else
		{
			fbyte = *_utf8++;
			sbyte = *_utf8++;
			tbyte = *_utf8++;

			(*_utf16++) = ((sbyte & 0x03) << 6) | (tbyte & 0x3f);
			(*_utf16++) = ((fbyte & 0x0f) << 4)| (sbyte & 0x3c >> 2);
		}
	}
	(*_utf16++) = 0x00;
	(*_utf16) = 0x00;
	return TRUE;
}

static void *_lan_utf8_to_utf16le(void *utf8)
{
	unsigned char *lanutf16 = NULL;
	unsigned int len = _lan_utf8_to_utf16le_len(utf8);
	if(len <= 0)
	{
		return NULL;
	}
	lanutf16 = _lan_alloc(len);
	if(!lanutf16)
	{
		return NULL;
	}
	if(!_lan_utf8_to_utf16le_copy(utf8, lanutf16, 0))
	{
		_lan_free((void *)lanutf16);
	}
	return (void *)lanutf16;
}
/*------------------------------------------------------------------
 * transcoding in : utf8 out : utf16be
 ------------------------------------------------------------------*/
static unsigned int _lan_utf8_to_utf16be_len(void *utf8)
{
	unsigned int len = 0;
	unsigned char *_utf8 = (unsigned char *)utf8;

	while(_utf8 && *_utf8)
	{
		if(*_utf8 <= 0x7f)
			_utf8 += 1;
		else if(*_utf8 <= 0xdf)
			_utf8 += 2;
		else
			_utf8 += 3;
		len++;
	}

	if(len > 0)
	{
		len = len * sizeof(unsigned short);
		len += 2;
	}
	return len;
}
static BOOL _lan_utf8_to_utf16be_copy(void *utf8, void *utf16, unsigned int utf16_len)
{
	unsigned char *_utf8 = (unsigned char *)utf8;
	unsigned char *_utf16 = (unsigned char *)utf16;
	unsigned char fbyte = 0x00;
	unsigned char sbyte = 0x00;
	unsigned char tbyte = 0x00;
	if(!utf8 || !utf16)
	{
		return FALSE;
	}
	if(utf16_len > 0)
	{
		if((_lan_utf8_to_utf16be_len(utf8) != utf16_len))
		{
			return FALSE;
		}
	}

	while((*_utf8) != 0)
	{
		if((*_utf8) <= 0x7f)
		{
			fbyte = 0x00;
			sbyte = *_utf8++;
			(*_utf16++) = fbyte;
			(*_utf16++) = sbyte;
		}
		else if((*_utf8) <= 0xdf)
		{
			fbyte = *_utf8++;
			sbyte = *_utf8++;
			(*_utf16++) = (fbyte & 0x1c) >> 2;
			(*_utf16++) = ((fbyte & 0x03) << 6) | (sbyte & 0x3f);
		}
		else
		{
			fbyte = *_utf8++;
			sbyte = *_utf8++;
			tbyte = *_utf8++;

			(*_utf16++) = ((fbyte & 0x0f) << 4)| (sbyte & 0x3c >> 2);
			(*_utf16++) = ((sbyte & 0x03) << 6) | (tbyte & 0x3f);
		}
	}
	(*_utf16++) = 0x00;
	(*_utf16) = 0x00;
	return TRUE;
}
static void *_lan_utf8_to_utf16be(void *utf8)
{
	unsigned char *lanutf16 = NULL;
	unsigned int len = _lan_utf8_to_utf16be_len(utf8);
	if(len <= 0)
	{
		return NULL;
	}
	lanutf16 = _lan_alloc(len);
	if(!lanutf16)
	{
		return NULL;
	}
	if(!_lan_utf8_to_utf16be_copy(utf8, lanutf16, 0))
	{
		_lan_free((void *)lanutf16);
	}
	return (void *)lanutf16;
}
/*------------------------------------------------------------------
 * transcoding in : utf16le out : utf16be
 ------------------------------------------------------------------*/
static unsigned int _lan_utf16le_to_utf16be_len(void *utf16le)
{
	unsigned short *_utf16 = (unsigned short *)utf16le;
	unsigned int _len = 0;
	if(!_utf16)
	{
		return 0;
	}
	while((*_utf16) != 0)
	{
		_utf16++;
		_len++;
	}
	return (_len * sizeof(unsigned short)) + sizeof(unsigned short);
}
static BOOL _lan_utf16le_to_utf16be_copy(void *utf16le, void *utf16be, unsigned int utf16be_len)
{
	unsigned short *_utf16le = (unsigned short *)utf16le;
	unsigned short *_utf16be = (unsigned short *)utf16be;

	if(!_utf16le ||
			_utf16be)
	{
		return FALSE;
	}
	if(utf16be_len > 0)
	{
		if(utf16be_len != _lan_utf16le_to_utf16be_len(utf16le))
		{
			return FALSE;
		}
	}
	while((*_utf16le) != 0)
	{
		*_utf16be++ = (((*_utf16le) & 0x00ff) << 8) | (((*_utf16le) & 0xff00) >> 8);
		_utf16le++;
	}
	*_utf16be = 0x0000;
	return TRUE;
}
static void * _lan_utf16le_to_utf16be(void *utf16le)
{
	unsigned char *lanutf16be = NULL;
	unsigned int len = _lan_utf16le_to_utf16be_len(utf16le);
	if(len <= 0)
	{
		return NULL;
	}
	lanutf16be = _lan_alloc(len);
	if(!lanutf16be)
	{
		return NULL;
	}
	if(!_lan_utf16le_to_utf16be_copy(utf16le, lanutf16be, 0))
	{
		_lan_free((void *)lanutf16be);
	}
	return (void *)lanutf16be;
}

/*------------------------------------------------------------------
 * transcoding in : utf16be out : utf16le
 ------------------------------------------------------------------*/
static unsigned int _lan_utf16be_to_utf16le_len(void *utf16be)
{
	unsigned short *_utf16 = (unsigned short *)utf16be;
	unsigned int _len = 0;
	if(!_utf16)
	{
		return 0;
	}
	while((*_utf16) != 0)
	{
		_utf16++;
		_len++;
	}
	return (_len * sizeof(unsigned short)) + sizeof(unsigned short);
}
static BOOL _lan_utf16be_to_utf16le_copy(void *utf16be, void *utf16le, unsigned int utf16le_len)
{
	unsigned short *_utf16le = (unsigned short *)utf16le;
	unsigned short *_utf16be = (unsigned short *)utf16be;

	if(!_utf16le ||
			_utf16be)
	{
		return FALSE;
	}
	if(utf16le_len > 0)
	{
		if(utf16le_len != _lan_utf16be_to_utf16le_len(utf16le))
		{
			return FALSE;
		}
	}
	while((*_utf16be) != 0)
	{
		*_utf16le++ = (((*_utf16be) & 0x00ff) << 8) | (((*_utf16be) & 0xff00) >> 8);
		_utf16be++;
	}
	*_utf16be = 0x0000;
	return TRUE;
}
static void * _lan_utf16be_to_utf16le(void *utf16be)
{
	unsigned char *lanutf16le = NULL;
	unsigned int len = _lan_utf16be_to_utf16le_len(utf16be);
	if(len <= 0)
	{
		return NULL;
	}
	lanutf16le = _lan_alloc(len);
	if(!lanutf16le)
	{
		return NULL;
	}
	if(!_lan_utf16be_to_utf16le_copy(utf16be, lanutf16le, 0))
	{
		_lan_free((void *)lanutf16le);
	}
	return (void *)lanutf16le;
}


/*------------------------------------------------------------------
 * transcoding in : utf16le out : utf8
 ------------------------------------------------------------------*/
static unsigned int _lan_utf16le_to_utf8_len(void *utf16le)
{
	unsigned short *_utf16le = (unsigned short *)utf16le;
	unsigned int len = 0;

	if(!_utf16le)
	{
		return 0;
	}

	while((*_utf16le) != 0)
	{
		if((*_utf16le) < 128)
			len += 1;
		else if((*_utf16le) < 0x800)
			len += 2;
		else
			len += 3;
		_utf16le++;
	}
	return len;
}
static BOOL _lan_utf16le_to_utf8_copy(void *utf16le, void *utf8, unsigned int utf8_len)
{
	unsigned short *_utf16le = (unsigned short *)utf16le;
	unsigned char *_utf8 = (unsigned char *)utf8;


	if(!_utf16le ||
			!_utf8)
	{
		return FALSE;
	}
	if(utf8_len > 0)
	{
		if(utf8_len != _lan_utf16le_to_utf8_len(utf16le))
		{
			return FALSE;
		}
	}

	while((*_utf16le) != 0)
	{
		if((*_utf16le) < 128)
		{
			*_utf8++ = *((unsigned char *)_utf16le);
		}
		else if((*_utf16le) < 0x800)
		{
			*_utf8++ = (0xc0 | ((*_utf16le) >> 6));
			*_utf8++ = (0x80 | ((*_utf16le) & 0x3f));
		}
		else
		{
			*_utf8++ = 0xe0 | ((*_utf16le) >> 12);
			*_utf8++ = 0x80 | (((*_utf16le) >> 6) & 0x3f);
			*_utf8++ = 0x80 | ((*_utf16le) & 0x3f);
		}
		_utf16le++;
	}
	return TRUE;
}
static void * _lan_utf16le_to_utf8(void *utf16le)
{
	unsigned char *lanutf8 = NULL;
	unsigned int len = _lan_utf16le_to_utf8_len(utf16le);
	if(len <= 0)
	{
		return NULL;
	}
	lanutf8 = _lan_alloc(len);
	if(!lanutf8)
	{
		return NULL;
	}
	if(!_lan_utf16le_to_utf8_copy(utf16le, lanutf8, 0))
	{
		_lan_free((void *)lanutf8);
	}
	return (void *)lanutf8;
}

/*------------------------------------------------------------------
 * transcoding in : utf16be out : utf8
 ------------------------------------------------------------------*/
static unsigned int _lan_utf16be_to_utf8_len(void *utf16be)
{
	unsigned short *_utf16be = (unsigned short *)utf16be;
	unsigned int len = 0;
	unsigned short str = 0;

	if(!_utf16be)
	{
		return 0;
	}

	str = (((*_utf16be) & 0x00ff) << 8 ) | (((*_utf16be) & 0xff00) >> 8);
	while(str != 0)
	{
		if(str < 128)
			len += 1;
		else if(str < 0x800)
			len += 2;
		else
			len += 3;
		_utf16be++;
		str = (((*_utf16be) & 0x00ff) << 8 ) | (((*_utf16be) & 0xff00) >> 8);
	}
	return len;
}
static BOOL _lan_utf16be_to_utf8_copy(void *utf16be, void *utf8, unsigned int utf8_len)
{
	unsigned short *_utf16be = (unsigned short *)utf16be;
	unsigned char *_utf8 = (unsigned char *)utf8;
	unsigned short str = 0;

	if(!_utf16be ||
			!_utf8)
	{
		return FALSE;
	}
	if(utf8_len > 0)
	{
		if(utf8_len != _lan_utf16be_to_utf8_len(utf16be))
		{
			return FALSE;
		}
	}

	str = (((*_utf16be) & 0x00ff) << 8 ) | (((*_utf16be) & 0xff00) >> 8);
	while(str != 0)
	{
		if(str < 128)
		{
			*_utf8++ = ((unsigned char )str);
		}
		else if(str < 0x800)
		{
			*_utf8++ = (0xc0 | (str >> 6));
			*_utf8++ = (0x80 | (str & 0x3f));
		}
		else
		{
			*_utf8++ = 0xe0 | (str >> 12);
			*_utf8++ = 0x80 | ((str >> 6) & 0x3f);
			*_utf8++ = 0x80 | (str & 0x3f);
		}
		_utf16be++;
		str = (((*_utf16be) & 0x00ff) << 8 ) | (((*_utf16be) & 0xff00) >> 8);
	}
	return TRUE;
}
static void *_lan_utf16be_to_utf8(void *utf16be)
{
	unsigned char *lanutf8 = NULL;
	unsigned int len = _lan_utf16be_to_utf8_len(utf16be);
	if(len <= 0)
	{
		return NULL;
	}
	lanutf8 = _lan_alloc(len);
	if(!lanutf8)
	{
		return NULL;
	}
	if(!_lan_utf16be_to_utf8_copy(utf16be, lanutf8, 0))
	{
		_lan_free((void *)lanutf8);
	}
	return (void *)lanutf8;
}


struct
{
	unsigned int (*require_length)(void *base);
	BOOL (*copy)(void *base, void *to, unsigned int to_len);
	void *(*transcode)(void *base);
}_transcode_table[_lan_max][_lan_max] =
{
		{
				{_lan_invalid_len, _lan_invalid_copy, _lan_invalid_transcode},
				{_lan_utf8_to_utf16le_len, _lan_utf8_to_utf16le_copy, _lan_utf8_to_utf16le},
				{_lan_utf8_to_utf16be_len, _lan_utf8_to_utf16be_copy, _lan_utf8_to_utf16be}
		},
		{

				{_lan_utf16le_to_utf8_len, _lan_utf16le_to_utf8_copy, _lan_utf16le_to_utf8},
				{_lan_invalid_len, _lan_invalid_copy, _lan_invalid_transcode},
				{_lan_utf16le_to_utf16be_len, _lan_utf16le_to_utf16be_copy, _lan_utf16le_to_utf16be}
		},
		{
				{_lan_utf16be_to_utf8_len, _lan_utf16be_to_utf8_copy, _lan_utf16be_to_utf8},
				{_lan_utf16be_to_utf16le_len, _lan_utf16be_to_utf16le_copy, _lan_utf16be_to_utf16le},
				{_lan_invalid_len, _lan_invalid_copy, _lan_invalid_transcode}
		}

};

WQ_API unsigned int _lan_transcode_require_length(void *base, _lan baselan, _lan tolan)
{
	return _transcode_table[baselan][tolan].require_length(base);
}
WQ_API BOOL _lan_transcode_copy(void *base, void *to, unsigned int to_len, _lan baselan, _lan tolan)
{
	return _transcode_table[baselan][tolan].copy(base, to, to_len);
}
WQ_API void *_lan_transcode(void *base, _lan baselan, _lan tolan)
{
	return _transcode_table[baselan][tolan].transcode(base);
}
WQ_API void _lan_free(void *ptr)
{
	if(ptr)
	{
		free(ptr);
	}
}
WQ_API BOOL _lan_convert_string_to_hex(void *base,
		void *to,
		unsigned int to_len,
		_lan baselan,
		BOOL base_prefix)
{
#define __isascii_hex(x) ((x >= 0x30 && x <= 0x39) || (x >= 0x41 && x <= 0x46) || (x >= 0x61 && x <= 0x66))
	unsigned char *_base = NULL;
	unsigned int _base_len = 0;
	unsigned char *_to = NULL;
	BOOL invalid_nibble = FALSE;

	do
	{
		if(!base ||
			!to ||
			to_len <= 0)
		{
			break;
		}

		_base = (unsigned char *)base;
		if(baselan != _lan_utf8)
		{
			_base = (unsigned char *)_lan_transcode(base, baselan, _lan_utf8);
			if(!_base)
			{
				break;
			}
		}
		_base_len = strlen((char *)_base);
		if(base_prefix && (_base_len != ((to_len << 1)+ 2)))
		{
			return FALSE;
		}
		if(!base_prefix && (_base_len != (to_len << 1)))
		{
			return FALSE;
		}
		if((_base_len & 1) != 0)
		{
			return FALSE;
		}
		unsigned char _to_dump[to_len];
		_to = (unsigned char *)&_to_dump[to_len -1];

		if(base_prefix)
		{
			_base += 2;
			_base_len -= 2;
		}

		while(_base_len > 0)
		{
			if(!(__isascii_hex(*_base)) ||
					!(__isascii_hex(*(_base + 1))))
			{
				invalid_nibble = TRUE;
				break;
			}

			*_to = 0;
			*_to |= (*_base) > 0x40 ? (((*_base) & 0x0f) + 9) << 4 : (((*_base) & 0x0f) << 4);
			*_to |= (*(_base+1)) > 0x40 ? (((*(_base+1)) & 0x0f) + 9) : (((*(_base+1)) & 0x0f));

			_base += 2;
			_base_len -= 2;
			_to--;
		}
		if(invalid_nibble)
		{
			break;
		}

		memcpy(to, _to_dump, to_len);
	}while(0);

	if(baselan != _lan_utf8)
	{
		if(_base)
		{
			_lan_free(_base);
		}
	}
	return !invalid_nibble;
}
WQ_API unsigned int _lan_convert_hex_to_string_require_length(void *base, unsigned int base_len, _lan outlan, BOOL prefix)
{
	return ((base && base_len > 0) ? ((((base_len * 2) + (prefix ? 2 : 0)) + 1) * (outlan == _lan_utf8 ? sizeof(char) : sizeof(short))) : 0);
}
BOOL _lan_convert_hex_to_string_copy(void *base, void *to, unsigned int base_len, unsigned int to_len, _lan outlan, BOOL prefix)
{
	if(!base ||
		!to ||
		base_len <= 0 ||
		to_len <= 0)
	{
		return FALSE;
	}
	if(to_len != (_lan_convert_hex_to_string_require_length(base, base_len, outlan, prefix)))
	{
		return FALSE;
	}
	unsigned char *_base = (((unsigned char *)base) + (base_len - 1));
	unsigned int _to_dump_len = _lan_convert_hex_to_string_require_length(base, base_len, _lan_utf8, prefix);
	unsigned char _to_dump[_to_dump_len];
	unsigned char *_to = _to_dump;
	if(prefix)
	{
		*_to++ = 0x30;
		*_to++ = 0x78;
	}
	while(base_len > 0)
	{
		unsigned char hinibble = ((*_base) & 0xf0) >> 4;
		unsigned char lownibble = ((*_base) & 0x0f);

		*_to++ = ((hinibble > 9) ? (0x57 + hinibble) : (0x30 | hinibble));
		*_to++ = ((lownibble > 9) ? (0x57 + lownibble) : (0x30 | lownibble));

		_base--;
		base_len--;
	}
	_to_dump[_to_dump_len - 1] = 0;
	if(outlan == _lan_utf8) memcpy(to, _to_dump, _to_dump_len);
	else 					_lan_transcode_copy(_to, to, to_len, _lan_utf8, outlan);

	return TRUE;
}
WQ_API void *_lan_convert_hex_to_string(void *base, unsigned int base_len, _lan outlan, BOOL prefix)
{
	unsigned int len = _lan_convert_hex_to_string_require_length(base, base_len, outlan, prefix);
	if(len <= 0)
	{
		return NULL;
	}
	unsigned char *_base = _lan_alloc(len);
	if(!_lan_convert_hex_to_string_copy(base, _base, base_len, len, outlan, prefix))
	{
		_lan_free(_base);
		_base = NULL;
	}
	return (void *)_base;
}
