#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "sysutil.h"

void debug_print(int log, const char *file, int line, enum OUTPUT_LEVEL level,
			const char *format, ...)
{
	const char *level_string;
	char entry[DEBUG_ENTRY_LEN + 1];
	int chars_left, n;
	char *entry_ptr;
	va_list ap;
	FILE *fp;

	switch(level) {
		case INFO:
			level_string = "INFO";
			break;
		case WARN:
			level_string = "WARNING";
			break;
		case ERR:
			level_string = "ERROR";
			break;
		case FATAL:
			level_string = "FATAL";
			break;
	}

	entry_ptr = entry;
	chars_left = DEBUG_ENTRY_LEN;
	memset(entry, 0, DEBUG_ENTRY_LEN + 1);

	n = sprintf(entry_ptr, "%s: ", level_string);
	entry_ptr += n;
	chars_left -= n;

	va_start(ap, format);

	n = vsnprintf(entry_ptr, chars_left, format, ap);

	entry_ptr += n;
	chars_left -= n;

	va_end(ap);

	if(file != NULL) {
		n = snprintf(entry_ptr, chars_left, " [%s:%d]", file, line);
		entry_ptr += n;
		chars_left -= n;
	}

	snprintf(entry_ptr, chars_left, "%s", EOL_STRING);

	fprintf(stderr, "%s", entry);

	if(log) {
		fp = fopen(DEBUG_LOG_FILE_NAME, "a");
		if(fp == NULL) {
			debug_print(0, __FILE__, __LINE__, ERR,
					"Unable to open log file for writing");
		} else {
			fwrite(entry, 1, strlen(entry), fp);
		}
		fclose(fp);
	}

	if(level == FATAL) {
		abort();
	}
}

