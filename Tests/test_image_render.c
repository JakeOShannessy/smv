#include "options.h"
// TODO: sort out imports
#include "MALLOCC.h"
#include "getdata.h"
#ifdef _WIN32
#include <windows.h>
#endif

#include GL_H
#include GLU_H
#include <EGL/egl.h>
#include <assert.h>
#include <gd.h>
#include <stdlib.h>
#include "file_util.h"

#define PNG 0
#define JPEG 1
#define IMAGE_NONE 2

int show_help;
int hash_option;
int show_version;
char append_string[1024];

GLubyte *ReadImage(int woffset, int width, int hoffset, int height) {
  GLubyte *opengl_image;
  NEWMEMORY(opengl_image, width * height * sizeof(GLubyte) * 3);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  /* get the image from the OpenGL frame buffer */

  glReadPixels(woffset, hoffset, width, height, GL_RGB, GL_UNSIGNED_BYTE,
               opengl_image);
  GLenum gl_error_code = glGetError();
  if(gl_error_code != GL_NO_ERROR) {
    const GLubyte *string = gluErrorString(gl_error_code);
    fprintf(stderr, "OpenGL error: %s\n", string);
    return NULL;
  }
  else {
    return opengl_image;
  }
}

int RenderToFile(GLubyte *opengl_image, const char *filepath, int rendertype,
                 int woffset, int width, int hoffset, int height) {
  FILE *file = fopen(filepath, "wb");
  if(file == NULL) {
    fprintf(stderr, "*** Error: unable to render screen image to %s", filepath);
    return 1;
  }

  // Create a local pointer into the image to use for iterating
  GLubyte *p = opengl_image;

  gdImagePtr gd_image = gdImageCreateTrueColor(width, height);

  for(int i = height - 1; i >= 0; i--) {
    for(int j = 0; j < width; j++) {
      unsigned int r, g, b;
      int rgb_local;

      r = *p++;
      g = *p++;
      b = *p++;
      rgb_local = (r << 16) | (g << 8) | b;
      gdImageSetPixel(gd_image, j, i, rgb_local);
    }
  }

  switch(rendertype) {
  case PNG:
    gdImagePng(gd_image, file);
    break;
  case JPEG:
    gdImageJpeg(gd_image, file, -1);
    break;
  default:
    assert(0);
    break;
  }
  // TODO: verify that the file has been created.

  gdImageDestroy(gd_image);
  fclose(file);
  return 0;
}

int Image2FileNew(const char *filepath, int rendertype, int woffset, int width,
                  int hoffset, int height) {
  GLubyte *opengl_image = ReadImage(woffset, width, hoffset, height);
  if(opengl_image == NULL) return 1;
  int return_code = RenderToFile(opengl_image, filepath, rendertype, woffset,
                                 width, hoffset, height);
  FREEMEMORY(opengl_image);
  return return_code;
}

static const EGLint configAttribs[] = {EGL_SURFACE_TYPE,
                                       EGL_PBUFFER_BIT,
                                       EGL_BLUE_SIZE,
                                       8,
                                       EGL_GREEN_SIZE,
                                       8,
                                       EGL_RED_SIZE,
                                       8,
                                       EGL_DEPTH_SIZE,
                                       8,
                                       EGL_RENDERABLE_TYPE,
                                       EGL_OPENGL_BIT,
                                       EGL_NONE};

static const int pbufferWidth = 9;
static const int pbufferHeight = 9;

static const EGLint pbufferAttribs[] = {
    EGL_WIDTH, pbufferWidth, EGL_HEIGHT, pbufferHeight, EGL_NONE,
};

int setup_gl() {
  // 1. Initialize EGL
  EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major, minor;

  eglInitialize(eglDpy, &major, &minor);

  // 2. Select an appropriate configuration
  EGLint numConfigs;
  EGLConfig eglCfg;

  eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);

  // 4. Bind the API
  eglBindAPI(EGL_OPENGL_API);

  // 5. Create a context and make it current
  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);

  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

  // from now on use your OpenGL context

  // 6. Terminate EGL when finished
  eglTerminate(eglDpy);
  return 0;
}

int main(int argc, char **argv) {
  initMALLOC();
  int return_code = 0;
  // width_beg=woffset;
  // width_end=width+woffset;
  // height_beg=hoffset;
  // height_end=hoffset+height;
  // if(clip_rendered_scene==1){
  //   width_beg+=render_clip_left;
  //   width_end-=render_clip_right;
  //   height_beg+=render_clip_bottom;
  //   height_end-=render_clip_top;
  // }
  // width2 = width_end-width_beg;
  // height2 = height_end-height_beg;
  const char *filepath = "test.png";
  return_code = Image2FileNew(filepath, PNG, 0, 100, 0, 100);
  UNLINK(filepath);
  return return_code;
}
