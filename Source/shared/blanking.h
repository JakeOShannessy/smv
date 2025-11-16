#ifndef BLANKING_H_DEFINED
#define BLANKING_H_DEFINED
struct blanking_flags {
  unsigned char node_html : 1;
  /**
   * @brief Is the cell, Cell(i,j,k) gas?
   *
   */
  unsigned char cell : 1;
  /**
   * @brief Are any of the 8 cells adjacent to Node(i,j,k) gas?
   *
   */
  unsigned char node : 1;
  /**
   * @brief Are either of the 2 cells adjacent to FaceX(i,j,k) gas? Note that
   * FaceX(i,j,k) is the x_min face of Cell(i,j,k).
   *
   */
  unsigned char node_x : 1;
  /**
   * @brief Are either of the 2 cells adjacent to FaceY(i,j,k) gas? Note that
   * FaceX(i,j,k) is the x_min face of Cell(i,j,k).
   *
   */
  unsigned char node_y : 1;
  /**
   * @brief Are either of the 2 cells adjacent to FaceZ(i,j,k) gas? Note that
   * FaceX(i,j,k) is the x_min face of Cell(i,j,k).
   *
   */
  unsigned char node_z : 1;
};
#endif
