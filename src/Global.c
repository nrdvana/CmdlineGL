#include <config.h>
#include "Global.h"

#ifdef _WIN32
void WinPerror(char *msg) {
	char* BuffPtr= NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		0,
		(LPTSTR) &BuffPtr,
		0,
		NULL);
	fprintf(stderr, "%s: %s", msg, BuffPtr? BuffPtr : "Error message lookup failed");
	if (BuffPtr) LocalFree(BuffPtr);
}
#endif

#ifndef NDEBUG
void DebugMsg(char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	fflush(stderr);
}
#endif

