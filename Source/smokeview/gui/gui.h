void ImGui_ImplGLUT_MouseWheelFunc(int button, int dir, int x, int y);
void ImGui_ImplGLUT_MotionFunc(int x, int y);
int setupImgui();
void imgui_display_start();
void imgui_display_end();
int imgui_keyboard(unsigned char key, int x, int y);
int imgui_reshape(int width, int height);
int imgui_mouse(int button, int state, int x, int y);
int imgui_motion(int x, int y);
