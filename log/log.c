//
// Created by HelliWrold1 on 2024/3/31.
//

#include "log.h"
#include <string.h>

char* logLevelGet(const int level)
{
    if (level == LOG_DEBUG)
        return "DEBUG";
    else if (level == LOG_INFO)
        return "INFO";
    else if (level == LOG_WARN)
        return "WARN";
    else if (level == LOG_ERROR)
        return "ERROR";
    else
        return "UNKNOWN";
}

void log(const int level, const char* date, const char* time, const char* file,
            const char* fun, const int line,const char* fmt,...)
{
#ifdef OPEN_LOG // 如果已经打开了日志开关
    va_list arg;
    va_start(arg, fmt);
    char buf[vsnprintf(NULL, 0, fmt, arg) + 1];
#if defined(__x86_64__)
    va_end(arg);

    va_start(arg, fmt);
#endif
    memset(buf,0,sizeof (buf));
    vsnprintf(buf, sizeof(buf), fmt, arg);
    va_end(arg);

    if (level >= CURR_LOG_LEVEL)
        printf("[%s]\t[%s %s][%s: %s:%d] %s\r\n", logLevelGet(level), date, time,
               file, fun, line, buf);

#ifdef WRITE_LOG_FILE // 需要写日志文件
    FILE* fp;
	fp = fopen(LOG_FILE_PATH, "a");
	if (fp && level>=LOG_LEVEL) // 文件打开成功并且leve大于等于需要的日志级别
	{
		fprintf(fp, "[%s]\t[%s %s][%s: %s:%d]\t%s\n", logLevelGet(level),
														date, time, file,
														fun, line, buf);
		fclose(fp);
	}
#endif

#endif
}