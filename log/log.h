//
// Created by HelliWrold1 on 2024/3/31.
//

#ifndef PRACTICE_LOG_H
#define PRACTICE_LOG_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>

#define OPEN_LOG // comment this to close log
//#define WRITE_LOG_FILE
#ifdef WRITE_LOG_FILE
#define LOG_FILE_PATH "./SensorData.log"
#endif

#define CURR_LOG_LEVEL LOG_INFO // level ref: enum:G_LOG_LEVEL

/**
 * @warning 使用这样的调用可能会出现问题：
 *          char sql[1024];
 *          sprintf(sql, "%s'%%Y-%%m-%%dT%%H:%%i:%%s'","insert ");
 *          LOG(LOG_DEBUG, sql);
 *          输出的信息含有乱码和Resource temporarily unavailable报错
 * @note 如果输出带有%%的字符串，应该向第二个参数fmt传递"%s"，用于说明要输出的信息不是fmt，而是输出信息
 * @example LOG(LOG_DEBUG, "%s", "%%Y-%%m-%%d %%H:%%i:%%s");
 */
#define LOG(level, fmt, ...) log(level, __DATE__, __TIME__, __FILE__, \
                                    __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LOG_LEVEL;

#define LOG_DEBUG(fmt, ...) LOG(LOG_DEBUG,fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG(LOG_INFO,fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_WARN,fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG(LOG_ERROR,fmt, ##__VA_ARGS__)

char *logLevelGet(const int level);
void log(const int level, const char *date, const char *time, const char *file,
            const char *fun, const int line, const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#endif //PRACTICE_LOG_H
