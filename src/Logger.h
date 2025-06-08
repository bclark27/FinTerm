#ifndef LOGGER_H_
#define LOGGER_H_


void Logger_Open();
void Logger_Log(const char *format, ...);
void Logger_Close();

#endif