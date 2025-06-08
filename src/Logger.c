#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

FILE *logfile;

void Logger_Open() {
    if (logfile) return;
    logfile = fopen("log", "w");
}

void Logger_Log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(logfile, format, args);
    fflush(logfile); // Ensure it's written immediately
    va_end(args);
}

void Logger_Close() {
    if (!logfile) return;
    fclose(logfile);
    logfile = NULL;
}