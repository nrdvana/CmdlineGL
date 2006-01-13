#include "Global.h"

// used to detect errors involving multiple/missing glBegin/glEnd
bool IsGlBegun= false;

#ifdef _WIN32
#include <stdio.h>
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

#ifdef DEBUG
#include <stdio.h>
#include <stdarg.h>

void DebugMsg(char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	fflush(stderr);
}

#endif

