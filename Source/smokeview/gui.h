#ifndef GUI_H_DEFINED
#define GUI_H_DEFINED

#include <GLFW/glfw3.h>

typedef enum CursorMotionMode {
   Rotate,
   ZoomLeftRight,
   ZoomLeftRightSpecial,
   Vertical,
   NoMotion,
} CursorMotionMode;

typedef struct Gui {
   // A handle to the GUI window.
   GLFWwindow* window;
   // A stored cursor that has the right style for movement.
   GLFWcursor* motion_cursor;
   // The current cursor movement mode. This mode determines if we should
   // rotate, pan, zoom and the like.
   CursorMotionMode cursor_motion_mode;
} Gui;

#endif
