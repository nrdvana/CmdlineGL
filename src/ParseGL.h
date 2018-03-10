#ifndef PARSEGL_H
#define PARSEGL_H

#include "Global.h"

extern bool PointsInProgress; // Whenever glBegin is active, until glEnd
extern bool FrameInProgress;  // True after any gl command, until cglSwapBuffers

#endif
