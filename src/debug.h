#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>

#define DEBUG_ENTRY_LEN 300
#define DEBUG_LOG_FILE_NAME "genem.log"

enum OUTPUT_LEVEL {INFO, WARN, ERR, FATAL};

#define dbg_i(...) debug_print(1, __FILE__, __LINE__, INFO, __VA_ARGS__)
#define dbg_w(...) debug_print(1, __FILE__, __LINE__, WARN, __VA_ARGS__)
#define dbg_e(...) debug_print(1, __FILE__, __LINE__, ERR, __VA_ARGS__)
#define dbg_f(...) debug_print(1, __FILE__, __LINE__, FATAL, __VA_ARGS__)

void debug_print(int log, const char *file, int line, enum OUTPUT_LEVEL level,
			const char *format, ...);

#endif /* __DEBUG_H */




