#include "Global.h"

#ifdef DEBUG
#include <stdarg.h>
#include <stdio.h>

void DebugMsg(char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	fflush(stderr);
}
#endif

