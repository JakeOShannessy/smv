#ifndef GUI_H_DEFINED
#define GUI_DEFINED
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

typedef enum CursorMotionMode {
    Rotate,
    ZoomLeftRight,
    ZoomLeftRightSpecial,
    Vertical,
    NoMotion,
} CursorMotionMode;

typedef struct Gui {
    GLFWwindow* window;
    GLFWcursor* motion_cursor;
    CursorMotionMode cursor_motion_mode;
} Gui;

extern Gui smv_gui;

int guiInit();
int guiRun();
int guiDestroy();
void InitOpenGL_gui();

#endif
