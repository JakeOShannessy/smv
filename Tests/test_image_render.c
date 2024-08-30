#include "options.h"
// TODO: sort out imports
#include "MALLOCC.h"
#include "getdata.h"
#ifdef _WIN32
#include <windows.h>
#endif

#include GL_H
#include GLU_H
#include <assert.h>
#include <gd.h>
#include <stdlib.h>

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

int main(int argc, char **argv) {
  initMALLOC();
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
  return Image2FileNew("test.png", PNG, 0, 100, 0, 100);
}
