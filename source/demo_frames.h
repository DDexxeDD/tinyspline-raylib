#ifndef _demo_frames_h_
#define _demo_frames_h_

#include "raylib-nuklear.h"

void demo_frames_initialize ();
void demo_frames_run (struct nk_context* context);
void demo_frames_draw ();
void demo_frames_cleanup ();

#endif
