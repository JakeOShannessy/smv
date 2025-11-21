#ifndef BLANKING_H_DEFINED
#define BLANKING_H_DEFINED

#include <stdbool.h>

struct blanking_flags {
  bool node_html : 1;
  /**
   * @brief Is the cell, Cell(i,j,k) gas?
   *
   */
  bool cell : 1;
  /**
   * @brief Are any of the 8 cells adjacent to Node(i,j,k) gas?
   *
   */
  bool node : 1;
  /**
   * @brief Are _both_ of the 2 cells adjacent to FaceX(i,j,k) gas? If either
   * cell is solid, this is false. Note that FaceX(i,j,k) is the x_min face of
   * Cell(i,j,k).
   *
   */
  bool node_x : 1;
  /**
   * @brief Are _both_ of the 2 cells adjacent to FaceY(i,j,k) gas? If either
   * cell is solid, this is false. Note that FaceX(i,j,k) is the x_min face of
   * Cell(i,j,k).
   *
   */
  bool node_y : 1;
  /**
   * @brief Are _both_ of the 2 cells adjacent to FaceZ(i,j,k) gas? If either
   * cell is solid, this is false. Note that FaceX(i,j,k) is the x_min face of
   * Cell(i,j,k).
   *
   */
  bool node_z : 1;
};

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(struct blanking_flags) == 1, "struct blanking_flags should be exactly 1 byte");
#endif
#endif

#endif
