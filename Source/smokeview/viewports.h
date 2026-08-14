#ifndef VIEWPORTS_H_DEFINED
#define VIEWPORTS_H_DEFINED
#include "options_common.h"
#include "structures.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include GL_H
#include "colorbars.h"

void ViewportClip(int quad, GLint s_left, GLint s_down);
void ViewportInfo(int quad, GLint s_left, GLint s_down);
void ViewportTimebar(int quad, GLint s_left, GLint s_down, scalebar *sbar);
void ViewportHrrPlot(int quad, GLint s_left, GLint s_down);
void ViewportSlicePlot(int quad, GLint s_left, GLint s_down);
void ViewportVerticalColorbar(int quad, GLint s_left, GLint s_down, scalebar *sbar);
void ViewportTitle(int quad, GLint s_left, GLint s_down);
void ViewportScene(int quad, int view_mode, GLint s_left, GLint s_down, screendata *screen);

//*** viewports.c headers

EXTERNCPP void GetViewportInfo(void);
#endif
