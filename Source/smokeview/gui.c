#include "gui.h"
#include <GL/gl.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
// #include "ui.h"
#include "callbacks.h"
#include "update.h"

// TODO: remove these includes
int InitShaders(void);
void DoNonStereo(void);
void DoScript(void);

Gui smv_gui = {0};

void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
}

int guiRun() {
  while (!glfwWindowShouldClose(smv_gui.window)) {

    DoScript();
#ifdef pp_LUA
    DoScriptLua();
#endif
    UpdateDisplay();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    // TODO: reenable stereoscopic rendering
    // if (stereotype == STEREO_NONE) {
    //   if (use_vr == 0) {
        DoNonStereo();
    //   }
    // } else {
    //   DoStereo();
    // }

    glfwSwapBuffers(smv_gui.window);
    glfwPollEvents();
  }
  return 0;
}

int guiDestroy() {
  glfwDestroyWindow(smv_gui.window);
  glfwTerminate();
  return 0;
}

/* ------------------ SetupWindow ------------------------ */
// Set up the window itself (not the GUI elements).
int SetupWindow() {
  if (!glfwInit()) {
    int code = glfwGetError(NULL);
    fprintf(stderr, "Failed initialise GLFW %i\n", code);
    exit(-1);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  int screenWidth = 800;
  int screenHeight = 500;
  // Create the window and store its id in the global gui struct.
  smv_gui.window =
      glfwCreateWindow(screenWidth, screenHeight, "Smokeview", NULL, NULL);
  if (!smv_gui.window) {
    // Failure to create window is not a recoverable error, terminate
    // immediately.
    fprintf(stderr, "Failed to create window\n");
    glfwTerminate();
    exit(-1);
  }
  smv_gui.motion_cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
  glfwSetErrorCallback(glfw_error_callback);
  // TODO: we shouldn't need to use these.
  // max_screenWidth = glutGet(GLUT_SCREEN_WIDTH);
  // max_screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

  // font_ptr          = GLUT_BITMAP_HELVETICA_12;
  // colorbar_font_ptr = GLUT_BITMAP_HELVETICA_10;

  // if(trainer_mode==1){
  //   int TRAINER_WIDTH;
  //   int TRAINER_HEIGHT;
  //   int scrW, scrH;

  //   TRAINER_WIDTH=300;
  //   TRAINER_HEIGHT=50;

  //   // scrW = max_screenWidth  - TRAINER_WIDTH;
  //   // scrH = max_screenHeight - TRAINER_HEIGHT;
  //   SetWindowSize(&scrW,&scrH);
  //   max_screenWidth = screenWidth;
  //   max_screenHeight = screenHeight;
  // }
  // Set the current OpenGL context to the window.
  glfwMakeContextCurrent(smv_gui.window);
  glewExperimental = GL_FALSE;
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    printf("OpenGL Error: 5s\n", error);
  }
  InitOpenGL_gui();
  glfwSwapInterval(1);
  return 0;
}

// Set up the interface elements.
int SetupGui() {
  // uiInitOptions options;
  // uiTab *tab;
  // memset(&options, 0, sizeof (uiInitOptions));
  // const char *err = uiInit(&options);
  // if (err != NULL) {
  //   fprintf(stderr, "%s\n", err);
  //   uiFreeInitError(err);
  //   return 1;
  // }
  // DialogMotionSetup(smv_gui);
  // GluiColorbarSetup(mainwindow_id);
  // GluiMotionSetup(smv_gui.window);
  // GluiBoundsSetup(mainwindow_id);
  // GluiShooterSetup(mainwindow_id);
  // GluiGeometrySetup(mainwindow_id);
  // GluiClipSetup(smv_gui.window);
  // GluiWuiSetup(mainwindow_id);
  // GluiLabelsSetup(mainwindow_id);
  // GluiDeviceSetup(mainwindow_id);
  // GluiTourSetup(mainwindow_id);
  // GluiAlertSetup(mainwindow_id);
  // GluiStereoSetup(mainwindow_id);
  // Glui3dSmokeSetup(mainwindow_id);
  return 0;
}

int SetupCallbacks() {
  glfwSetKeyCallback(smv_gui.window, KeyboardCBGlfw);
  glfwSetCursorPosCallback(smv_gui.window, MouseCB);
  glfwSetMouseButtonCallback(smv_gui.window, MouseButtonCB);
  glfwSetWindowSizeCallback(smv_gui.window, ReshapeCB);
  // Frame buffer size will vary a lot particularly with high-DPI.
  glfwSetFramebufferSizeCallback(smv_gui.window, FrameResizeCB);
  // The content scale callback will trigger if the OS high-DPI handling
  // changes. glfwSetWindowContentScaleCallback(smv_gui.window, ContentScaleCB);
  // glfwSetScrollCallback(smv_gui.window,ScrollCB);
  return 0;
}

/* ------------------ InitOpenGL ------------------------ */

void InitOpenGL_gui() {
  int err;

  {
    GLint m[4];
    glGetIntegerv(GL_VIEWPORT, m);
    printf("%i %i %i %i\n", m[0], m[1], m[2], m[3]);
  }

  err = 0;

  err = glewInit();
  if (err == GLEW_OK) {
    err = 0;
  } else {
    printf("   GLEW initialization failed\n");
    err = 1;
  }
  err = InitShaders();
  if (err != 0) {
    printf("%s\n", "  GPU shader initialization failed");
  }

  // }

  // light_position0[0]=1.0f;
  // light_position0[1]=1.0f;
  // light_position0[2]=4.0f;
  // light_position0[3]=0.f;

  // light_position1[0]=-1.0f;
  // light_position1[1]=1.0f;
  // light_position1[2]=4.0f;
  // light_position1[3]=0.f;

  // {
  //   glGetIntegerv(GL_RED_BITS,&nredbits);
  //   glGetIntegerv(GL_GREEN_BITS,&ngreenbits);
  //   glGetIntegerv(GL_BLUE_BITS,&nbluebits);

  //   nredshift = 8 - nredbits;
  //   if(nredshift<0)nredshift=0;
  //   ngreenshift = 8 - ngreenbits;
  //   if(ngreenshift<0)ngreenshift=0;
  //   nblueshift=8-nbluebits;
  //   if(nblueshift<0)nblueshift=0;
  // }
  // opengldefined=1;
  // if(option==PRINT)PRINTF("%s\n\n",_("complete"));
}

int guiInit() {
  // Setup the window
  SetupWindow();
  // Setup the dialog boxes etc.
  SetupGui();
  // Setup GLFW callbacks
  SetupCallbacks();
  return 0;
}
