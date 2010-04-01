#include <stddef.h>
#include "types.h"
#include "debug.h"

#ifndef __SYSUTIL_H__
#define __SYSUTIL_H__

/*
Sort which OS we have.
These are #defines we may get from gcc.
*/

#if defined(__linux__) || defined(__linux) || defined(linux)
	#define OS_LINUX
#elif defined(__unix__) || defined(__unix) || defined(unix)
	#define OS_UNIX
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	#define OS_WIN
#elif defined(__APPLE__) || defined(_APPLE) || defined(APPLE)
	#define OS_MAC
#else
	#define OS_LINUX
#endif

/* platform specific includes */
#ifdef OS_LINUX
#include <sys/time.h>
#include <unistd.h>
#endif

/* Define newline sequence. Only Windows is different */

#ifdef OS_WIN
	#define EOL_STRING "\r\n"
#else
	#define EOL_STRING "\n"
#endif

/* microsecond-accurate sleep */

#ifdef OS_LINUX
	#define sleep_usec(n) usleep(n)
#else
	dbg_f("No usleep set up for this platform");
#endif

#define NOFUNC() dbg_f("This platform has no support for function: %s", __func__)

size_t get_file_size(const char *filename);
size_t read_file(const char *filename, byte *buf, size_t max_len);
/* current time in usecs */
unsigned long now_usec();

#endif
