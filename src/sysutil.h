#include <stddef.h>
#include "debug.h"

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
#elif #defined(__APPLE__) || defined(_APPLE) || defined(APPLE)
	#define OS_MAC
#else
	#define OS_LINUX
#endif

/* Define newline sequence. Only Windows is different */

#ifdef OS_WIN
	#define EOL_STRING "\r\n"
#else
	#define EOL_STRING "\n"
#endif

#define NOFUNC() dbg_f("This platform has no support for function: %s", __func__)

size_t get_file_size(const char *filename);
size_t read_file(const char *filename, char *buf, size_t max_len);
