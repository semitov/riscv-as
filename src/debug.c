#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

static void log_info(const char *fmt, va_list args) {
	(void)fmt;
	(void)args;
#ifdef INFO
	fprintf(stdout, "[INFO] ");
	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");
#endif
}

static void log_debug(const char *file, int line, const char *fmt, va_list args) {
	(void)file;
	(void)line;
	(void)fmt;
	(void)args;
#ifdef EVELOPER
	fprintf(stdout, "[DEBUG] [%s:%d] ", file, line);
	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");
#endif
}

static void log_error(const char *file, int line, const char *fmt, va_list args) {
	fprintf(stderr, "[ERROR] [%s:%d] ", file, line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}

void _log_msg(log_type_e log_type, const char *file, int line, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	switch (log_type) {
		case LOG_INFO:
			log_info(fmt, args);
			break;
		case LOG_DEBUG:
			log_debug(file, line, fmt, args);
			break;
		case LOG_ERROR:
			log_error(file, line, fmt, args);
	}

	va_end(args);
}
