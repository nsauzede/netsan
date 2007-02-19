#ifndef _LIBGEN_H
#define _LIBGEN_H

#ifdef WIN32
#define basename(foo)   compat_basename(foo)/* win32 su^H^Hlacks basename support */
inline char *compat_basename( const char *str)
{
	char *result = (char *)str + strlen( str);
	while (result > str)
	{
		if (*result == '\\')
			break;
		else if (*result == '/')
			break;
		result--;
	}
	return result + 1;
}
#endif

#endif

