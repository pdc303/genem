#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>
#include <stdlib.h>

#define DEBUG_ENTRY_LEN 300
#define DEBUG_LOG_FILE_NAME "genem.log"

enum OUTPUT_LEVEL {INFO, WARN, ERR, FATAL};

#ifndef SILENT
#define dbg_i(...) debug_print(1, __FILE__, __LINE__, INFO, __VA_ARGS__)
#define dbg_w(...) debug_print(1, __FILE__, __LINE__, WARN, __VA_ARGS__)
#define dbg_e(...) debug_print(1, __FILE__, __LINE__, ERR, __VA_ARGS__)
#define dbg_f(...) debug_print(1, __FILE__, __LINE__, FATAL, __VA_ARGS__)
#else
#define dbg_i(...)
#define dbg_w(...)
#define dbg_e(...)
#define dbg_f(...) abort()
#endif

void debug_print(int log, const char *file, int line, enum OUTPUT_LEVEL level,
			const char *format, ...);

#endif /* __DEBUG_H */




