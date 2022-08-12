#include "printk_event_handler.h"

#include <cstdarg>

#include <zephyr/sys/printk.h>

void PrintkEventHandler::Write(const char* content) { 
    printk("%s", content);
}

void PrintkEventHandler::WriteLine(const char* format, ...) {
    va_list args;

    va_start(args, format);
    vprintk(format, args);
    printk("\n");
    va_end(args);
}
