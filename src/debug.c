/*  SemiTOV-Assembler, Small RISC-V Assembler.
	Copyright (C) 2025 SemiTO-V Student Group <semitofive@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>. */

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
