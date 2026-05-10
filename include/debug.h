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
