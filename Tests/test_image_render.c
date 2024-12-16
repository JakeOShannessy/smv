#include "options.h"
#ifndef _WIN32
#include "dmalloc.h"
#include "getdata.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__linux__)
#include <EGL/egl.h>
#endif
#include GL_H
#include GLU_H
#include "file_util.h"
#include <assert.h>
#include <gd.h>
#include <math.h>
#include <stdlib.h>

#define PNG 0
#define JPEG 1
#define IMAGE_NONE 2

#if defined(__linux__)
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
#else

int RenderToFile(const char *filepath, int rendertype, int woffset, int width,
                 int hoffset, int height) {
  FILE *file = fopen(filepath, "wb");
  if(file == NULL) {
    fprintf(stderr, "*** Error: unable to render screen image to %s", filepath);
    return 1;
  }

  gdImagePtr gd_image = gdImageCreateTrueColor(width, height);

  if(gd_image == NULL) {
    fprintf(stderr, "*** Error: unable to create image");
    return 1;
  }

  // for(int i = height - 1; i >= 0; i--) {
  //   for(int j = 0; j < width; j++) {
  //     unsigned int r, g, b;
  //     int rgb_local;

  //     r = 75;
  //     g = 0;
  //     b = 0;
  //     rgb_local = (r << 16) | (g << 8) | b;
  //     gdImageSetPixel(gd_image, j, i, rgb_local);
  //   }
  // }

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

// int main(int argc, char **argv) {
//   FILE *f2 = fopen("abc.png", "wb");
//   fclose(f2);
//   int i;
//   FILE *out;

//   gdImagePtr im;
//   gdImagePtr prev = NULL;
//   int black;

//   im = gdImageCreate(100, 100);
//   if(!im) {
//     fprintf(stderr, "can't create image");
//     return 1;
//   }

//   out = fopen("anim.gif", "wb");
//   if(!out) {
//     fprintf(stderr, "can't create file %s", "anim.gif");
//     return 1;
//   }

//   gdImageColorAllocate(im, 255, 255, 255); /* allocate white as side effect
//   */ gdImageGifAnimBegin(im, out, 1, -1);

//   for(i = 0; i < 20; i++) {
//     int r, g, b;
//     im = gdImageCreate(100, 100);
//     r = rand() % 255;
//     g = rand() % 255;
//     b = rand() % 255;

//     gdImageColorAllocate(im, 255, 255, 255); /* allocate white as side effect
//     */ black = gdImageColorAllocate(im, r, g, b); printf("(%i, %i, %i)\n", r,
//     g, b); gdImageFilledRectangle(im, rand() % 100, rand() % 100, rand() %
//     100,
//                            rand() % 100, black);
//     gdImageGifAnimAdd(im, out, 1, 0, 0, 10, 1, prev);

//     if(prev) {
//       gdImageDestroy(prev);
//     }
//     prev = im;
//   }

//   gdImageGifAnimEnd(out);
//   fclose(out);

//   RenderToFile("some_test.png", PNG, 0, 100, 0, 100);
//   return 0;
// }

gdImagePtr loadImage(const char *name) {
  FILE *fp = fopen(name, "rb");
  if(!fp) {
    fprintf(stderr, "Can't open jpeg file: %s\n", name);
    return NULL;
  }
  gdImagePtr im = gdImageCreateFromJpeg(fp);
  fclose(fp);
  return im;
}

int savePngImage(gdImagePtr im, const char *name) {
  FILE *fp = fopen(name, "wb");
  if(!fp) {
    fprintf(stderr, "Can't save png image fromtiff.png\n");
    return 0;
  }
  gdImagePng(im, fp);
  fclose(fp);
  return 1;
}

int main(int argc, char **arg) {
  gdImagePtr im, im2;
  int new_width, new_height;
  double angle, a2;

  char *file_in = arg[1];
  angle = strtod("30", 0);
  im = loadImage(file_in);

  if(!im) {
    fprintf(stderr, "Can't load PNG file <%s>", file_in);
    return 1;
  }

  /*
          cos adj hyp (cos = adjacent / hypotenuse)
          sin op hyp (sin adjacent / hypotenuse)
          + 10 pixels margin
   */

  /* to radian */
  a2 = angle * .0174532925;

  new_width =
      fabs(ceil(cos(a2) * gdImageSX(im))) + fabs(sin(a2) * gdImageSY(im));
  new_height =
      fabs(ceil(cos(a2) * gdImageSY(im))) + fabs(sin(a2) * gdImageSX(im));

  im2 = gdImageCreateTrueColor(new_width, new_height);
  if(!im2) {
    fprintf(stderr, "Can't create a new image");
    gdImageDestroy(im);
    return 1;
  }

  gdImageAlphaBlending(im2, 0);
  gdImageFilledRectangle(im2, 0, 0, gdImageSX(im2), gdImageSY(im2),
                         gdTrueColorAlpha(127, 0, 0, 127));

  gdImageCopyRotated(im2, im, new_width / 2, new_height / 2, 0, 0,
                     gdImageSX(im), gdImageSY(im), angle);
  gdImageSaveAlpha(im2, 1);
  if(!savePngImage(im2, "rotated.png")) {
    fprintf(stderr, "Can't save PNG file rotated.png");
    gdImageDestroy(im);
    gdImageDestroy(im2);
    return 1;
  }

  gdImageDestroy(im2);
  gdImageDestroy(im);
  return 0;
}
#endif
