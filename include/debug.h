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

/*
 * Logging and debugging utilities.
 */

#ifndef ASSEMBLER_DEBUG_H
#define ASSEMBLER_DEBUG_H

/**
 * @brief Log message severity levels.
 */
typedef enum log_type {
	LOG_DEBUG, /**< Debug-level message */
	LOG_INFO,  /**< Informational message */
	LOG_ERROR, /**< Error message */
} log_type_e;

void _log_msg(log_type_e log_type, const char *file, int line, const char *fmt, ...);

/**
 * @brief Log a formatted message.
 *
 * @param type Severity level of the log message.
 * @param fmt printf-style format string.
 * @param ... Additional format arguments.
 */
#define log_msg(type, fmt, ...) _log_msg(type, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
