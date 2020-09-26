#include "options.h"

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include GLUT_H
#include <math.h>
#include "ui.h"

#include "smokeviewvars.h"
#include "IOvolsmoke.h"

#define TRANSLATE_XY 101
#define ROTATE_2AXIS 102
#define GLUI_Z         2
#define MESH_LIST      4
#define EYE_ROTATE     5
#define EYE_ROTATE_90  6
#define EYELEVEL       7
#define FLOORLEVEL     8
#define ROTATE_90    103
#define RESET_VIEW   104

#define LABEL_VIEW 4

#define CUSTOM_ROTATION_X 9
#define CUSTOM_ROTATION_Y 10
#define CUSTOM_ROTATION_Z 11
#define LIST_VIEW 5
#define ADD_VIEW 6
#define DELETE_VIEW 7
#define RESTORE_VIEW 8
#define REPLACE_VIEW 9
#define STARTUP 10
#define CYCLEVIEWS 11
#define ZOOM 12
#define APERTURE 15
#define CURSOR 13
#define SAVE_SETTINGS 14
#define WINDOW_RESIZE 16
#define WINDOWSIZE_LIST 17
#define SNAPSCENE 21
#define SET_VIEW_XYZ 22
#define ZAXIS_CUSTOM 25
#define USE_GVEC 28
#define GSLICE_TRANSLATE 24
#define GSLICE_NORMAL 27
#define PLAY_MOVIE 29
#define MOVIE_NAME 30
#define CLOSE_MOTION 1
#ifdef pp_RENDER360_DEBUG
#define SHOWALL_SCREENS 31
#define HIDEALL_SCREENS 32
#endif
#define WINDOW_COLORS 33
#define COLOR_FLIP 34
#define CLIP_SHOW_ROTATE 35
#define ZAXIS_UP 41
#define NEARFARCLIP 42
#define CUSTOM_VIEW 43

#define RENDER_TYPE 0
#define RENDER_RESOLUTION 1
#define RENDER_SKIP 2
#define RENDER_START_HIGHRES 11
#define RENDER_STOP 4
#define RENDER_LABEL 5
#define RENDER_MULTIPLIER 6
#define MOVIE_FILETYPE 7
#define RENDER_MODE 8
#define RENDER_START_TOP 13
#define RENDER_DEBUG_360 14

#define SLICE_ROLLOUT           0
#define VIEWPOINTS_ROLLOUT      1
#define WINDOW_ROLLOUT          2
#define SCALING_ROLLOUT         3
#define TRANSLATEROTATE_ROLLOUT 4
#define ROTATION_ROLLOUT        5
#define ORIENTATION_ROLLOUT     6
#define POSITION_VIEW_ROLLOUT   7

#define MOTION_ROLLOUT 0
#define VIEW_ROLLOUT   1
#define RENDER_ROLLOUT 2
#define MOVIE_ROLLOUT  3

#define RENDER_FILE_ROLLOUT   0
#define RENDER_SIZE_ROLLOUT   1
#define RENDER_SCREEN_ROLLOUT 2
#define RENDER_CLIP_ROLLOUT   3

#define RENDER_360CB 9
#ifdef pp_HTML
#define RENDER_HTML 15
#endif

#ifdef pp_DEG
unsigned char deg360[] = { '3','6','0',DEG_SYMBOL,0 };
unsigned char deg90[] = {'9', '0', DEG_SYMBOL, 0};
#else
unsigned char deg360[] = { '3','6','0',0 };
unsigned char deg90[] = {'9', '0', 0};
#endif


static int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

static int onShouldQuit(void *data)
{
	uiWindow *mainwin = uiWindow(data);

	uiControlDestroy(uiControl(mainwin));
	return 1;
}

/* ------------------ GluiMotionSetup ------------------------ */

int DialogMotionSetup(Gui *smv_gui) {
  int i;
#define TRANSLATE_SPEED 0.005
  int *rotation_index;
  float *eye_xyz;

  if(camera_label!=NULL){
    FREEMEMORY(camera_label);
  }
  // NewMemory((void **)&camera_label,sizeof(GLUI_String));

  strcpy(camera_label,"current");

  eye_xyz=camera_current->eye;

  uiWindow *motion_window;
  motion_window = uiNewWindow(_("Motion/View/Render"), 640, 480, 1);
  uiWindowOnClosing(motion_window, onClosing, NULL);
  uiOnShouldQuit(onShouldQuit, motion_window);

  uiTab *tab;
  tab = uiNewTab();
  uiWindowSetChild(motion_window, uiControl(tab));
  uiWindowSetMargined(motion_window, 1);

  // uiTabAppend(tab, "Basic Controls", makeBasicControlsPage());
  // uiTabSetMargined(tab, 0, 1);

  // uiTabAppend(tab, "Numbers and Lists", makeNumbersPage());
  // uiTabSetMargined(tab, 1, 1);

  // uiTabAppend(tab, "Data Choosers", makeDataChoosersPage());
  // uiTabSetMargined(tab, 2, 1);

  uiControlShow(uiControl(motion_window));
  uiMain();
  // return 0;

//   glui_motion = GLUI_Master.create_glui(_("Motion/View/Render"),0,0,0);
//   glui_motion->hide();

//   ROLLOUT_motion = glui_motion->add_rollout("Motion",false, MOTION_ROLLOUT, MVRRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_motion, glui_motion);
//   ADDPROCINFO(mvrprocinfo, nmvrprocinfo, ROLLOUT_motion, MOTION_ROLLOUT, glui_motion);

//   ROLLOUT_translaterotate=glui_motion->add_rollout_to_panel(ROLLOUT_motion, _("Translate/Rotate"), true, TRANSLATEROTATE_ROLLOUT, MotionRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_translaterotate, glui_motion);
//   ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_translaterotate, TRANSLATEROTATE_ROLLOUT, glui_motion);

//   PANEL_translate2 = glui_motion->add_panel_to_panel(ROLLOUT_translaterotate,_("Translate"));
//   d_eye_xyz[0]=0.0;
//   d_eye_xyz[1]=0.0;
//   d_eye_xyz[2]=0.0;
//   dsave_eye_xyz[0]=0.0;
//   dsave_eye_xyz[1]=0.0;
//   dsave_eye_xyz[2]=0.0;

//   TRANSLATE_xy=glui_motion->add_translation_to_panel(PANEL_translate2,_("Horizontal"),GLUI_TRANSLATION_XY,d_eye_xyz,TRANSLATE_XY,SceneMotionCB);
//   TRANSLATE_xy->set_speed(TRANSLATE_SPEED);

//   glui_motion->add_column_to_panel(PANEL_translate2,false);

//   TRANSLATE_z=glui_motion->add_translation_to_panel(PANEL_translate2,_("Vertical"),GLUI_TRANSLATION_Y,eye_xyz+2,GLUI_Z,SceneMotionCB);
//   TRANSLATE_z->set_speed(TRANSLATE_SPEED);

//   PANEL_rotate = glui_motion->add_panel_to_panel(ROLLOUT_translaterotate,_("Rotate"));

//   ROTATE_2axis=glui_motion->add_translation_to_panel(PANEL_rotate,_("2 axis"),GLUI_TRANSLATION_XY,motion_ab,ROTATE_2AXIS,SceneMotionCB);
//   glui_motion->add_column_to_panel(PANEL_rotate,false);

//   ROTATE_eye_z=glui_motion->add_translation_to_panel(PANEL_rotate,_("View"),GLUI_TRANSLATION_X,motion_dir,EYE_ROTATE,SceneMotionCB);
//   ROTATE_eye_z->set_speed(180.0/(float)screenWidth);
//   ROTATE_eye_z->disable();

//   ROLLOUT_view = glui_motion->add_rollout_to_panel(ROLLOUT_motion, _("Position/View"), false, POSITION_VIEW_ROLLOUT, MotionRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_view, glui_motion);
//   ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_view, POSITION_VIEW_ROLLOUT, glui_motion);

//   PANEL_specify = glui_motion->add_panel_to_panel(ROLLOUT_view, _("Position"));

//   SPINNER_set_view_x = glui_motion->add_spinner_to_panel(PANEL_specify, "x:", GLUI_SPINNER_FLOAT,   set_view_xyz, SET_VIEW_XYZ, SceneMotionCB);
//   SPINNER_set_view_y = glui_motion->add_spinner_to_panel(PANEL_specify, "y:", GLUI_SPINNER_FLOAT, set_view_xyz+1, SET_VIEW_XYZ, SceneMotionCB);
//   SPINNER_set_view_z = glui_motion->add_spinner_to_panel(PANEL_specify, "z:", GLUI_SPINNER_FLOAT, set_view_xyz+2, SET_VIEW_XYZ, SceneMotionCB);

//   PANEL_custom_view = glui_motion->add_panel_to_panel(ROLLOUT_view, _("View"));

//   CHECKBOX_use_customview = glui_motion->add_checkbox_to_panel(PANEL_custom_view,_("Use azimuth/elevation"),&use_customview, CUSTOM_VIEW, SceneMotionCB);
//   SPINNER_customview_azimuth = glui_motion->add_spinner_to_panel(PANEL_custom_view,"azimuth:",GLUI_SPINNER_FLOAT,&customview_azimuth,CUSTOM_VIEW,SceneMotionCB);
//   SPINNER_customview_elevation = glui_motion->add_spinner_to_panel(PANEL_custom_view,"elevation:", GLUI_SPINNER_FLOAT, &customview_elevation, CUSTOM_VIEW, SceneMotionCB);
//   //glui_motion->add_spinner_to_panel(PANEL_custom_view,"     up:", GLUI_SPINNER_FLOAT, &customview_up, CUSTOM_VIEW, SceneMotionCB);
//   {
//     char rotate_label[100];

//     sprintf(rotate_label,"%s rotate",deg90);
//     BUTTON_rotate90 = glui_motion->add_button_to_panel(PANEL_custom_view, rotate_label, ROTATE_90, SceneMotionCB);
//   }
//   glui_motion->add_button_to_panel(PANEL_custom_view, "Reset", RESET_VIEW, SceneMotionCB);

//   ROLLOUT_rotation_type = glui_motion->add_rollout_to_panel(ROLLOUT_motion,_("Specify Rotation"),false,ROTATION_ROLLOUT,MotionRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_rotation_type, glui_motion);
//   ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_rotation_type, ROTATION_ROLLOUT, glui_motion);

//   PANEL_radiorotate = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type, "Rotation type:");
//   RADIO_rotation_type=glui_motion->add_radiogroup_to_panel(PANEL_radiorotate,&rotation_type,0, RotationTypeCB);
//   RADIOBUTTON_1c=glui_motion->add_radiobutton_to_group(RADIO_rotation_type,_("2 axis"));
//   RADIOBUTTON_1d=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("eye centered"));
//   RADIOBUTTON_1e=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("level (1 axis)"));
//   RADIOBUTTON_1e=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("3 axis"));
//   RotationTypeCB(rotation_type);
//   rotation_index=&camera_current->rotation_index;
//   *rotation_index=glui_rotation_index_ini;

//   LIST_mesh2 = glui_motion->add_listbox_to_panel(ROLLOUT_rotation_type,_("Rotate about:"),rotation_index,MESH_LIST,SceneMotionCB);
//   LIST_mesh2->add_item(ROTATE_ABOUT_CLIPPING_CENTER, _("center of clipping planes"));
//   LIST_mesh2->add_item(ROTATE_ABOUT_USER_CENTER,_("user specified center"));
//   for(i=0;i<nmeshes;i++){
//     meshdata *meshi;

//     meshi = meshinfo + i;
//     LIST_mesh2->add_item(i,meshi->label);
//   }
//   LIST_mesh2->add_item(nmeshes,_("world center"));
//   LIST_mesh2->set_int_val(*rotation_index);

//   PANEL_user_center = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type, "rotation center");
//   CHECKBOX_show_rotation_center=glui_motion->add_checkbox_to_panel(PANEL_user_center,_("Show"),&show_rotation_center, CLIP_SHOW_ROTATE, SceneMotionCB);
//   xcenCUSTOMsmv = DENORMALIZE_X(xcenCUSTOM);
//   ycenCUSTOMsmv = DENORMALIZE_Y(ycenCUSTOM);
//   zcenCUSTOMsmv = DENORMALIZE_Z(zcenCUSTOM);
//   SPINNER_xcenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"x:",GLUI_SPINNER_FLOAT,&xcenCUSTOMsmv,CUSTOM_ROTATION_X,SceneMotionCB);
//   SPINNER_ycenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"y:",GLUI_SPINNER_FLOAT,&ycenCUSTOMsmv,CUSTOM_ROTATION_Y,SceneMotionCB);
//   SPINNER_zcenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"z:",GLUI_SPINNER_FLOAT,&zcenCUSTOMsmv,CUSTOM_ROTATION_Z,SceneMotionCB);
//   SPINNER_xcenCUSTOM->set_float_limits(DENORMALIZE_X(0.0),DENORMALIZE_X(1.0));
//   SPINNER_ycenCUSTOM->set_float_limits(DENORMALIZE_Y(0.0),DENORMALIZE_Y(1.0));
//   SPINNER_zcenCUSTOM->set_float_limits(DENORMALIZE_Z(0.0),DENORMALIZE_Z(1.0));

//   SceneMotionCB(MESH_LIST);

//   PANEL_anglebuttons = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type,"",GLUI_PANEL_NONE);
//   BUTTON_90_z=glui_motion->add_button_to_panel(PANEL_anglebuttons,"90 deg",EYE_ROTATE_90,SceneMotionCB);
//   BUTTON_90_z->disable();
//   BUTTON_90_z->set_alignment(GLUI_ALIGN_LEFT);
// //  glui_motion->add_column_to_panel(PANEL_anglebuttons,false);
//   BUTTON_snap=glui_motion->add_button_to_panel(PANEL_anglebuttons,_("Snap"),SNAPSCENE,SceneMotionCB);

//   //glui_motion->add_column(false);

//   ROLLOUT_orientation=glui_motion->add_rollout_to_panel(ROLLOUT_motion,_("Orientation"),false,ORIENTATION_ROLLOUT,MotionRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_orientation, glui_motion);
//   ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_orientation, ORIENTATION_ROLLOUT, glui_motion);

//   PANEL_change_zaxis = glui_motion->add_panel_to_panel(ROLLOUT_orientation,_("z axis"));

//   if(zaxis_custom==0){
//     float vv[3];

//     if(have_gvec==1){
//       vv[0] = -gvecphys[0];
//       vv[1] = -gvecphys[1];
//       vv[2] = -gvecphys[2];
//     }
//     else{
//       vv[0] = -gvecphys_orig[0];
//       vv[1] = -gvecphys_orig[1];
//       vv[2] = -gvecphys_orig[2];
//     }
//     XYZ2AzElev(vv, zaxis_angles, zaxis_angles+1);
//   }
//   SPINNER_zaxis_angles[0] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("azimuth:"),GLUI_SPINNER_FLOAT, zaxis_angles,ZAXIS_CUSTOM, SceneMotionCB);
//   SPINNER_zaxis_angles[1] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("elevation:"),GLUI_SPINNER_FLOAT,zaxis_angles+1,ZAXIS_CUSTOM,SceneMotionCB);
//   SPINNER_zaxis_angles[2] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("angle (about z axis):"),GLUI_SPINNER_FLOAT,zaxis_angles+2,ZAXIS_CUSTOM,SceneMotionCB);
//   SPINNER_zaxis_angles[0]->set_float_limits(-180.0,180.0);
//   SPINNER_zaxis_angles[1]->set_float_limits(-90.0,90.0);
//   SPINNER_zaxis_angles[2]->set_float_limits(-180.0,180.0);

//   glui_motion->add_button_to_panel(PANEL_change_zaxis, "z vector up", ZAXIS_UP, SceneMotionCB);
//   if(have_gvec==1){
//     glui_motion->add_button_to_panel(PANEL_change_zaxis, "Gravity vector down", USE_GVEC, SceneMotionCB);
//     CHECKBOX_showgravity_vector = glui_motion->add_checkbox_to_panel(PANEL_change_zaxis, "Show gravity, axis vectors", &showgravity_vector);
//   }
//   else{
//     CHECKBOX_showgravity_vector = glui_motion->add_checkbox_to_panel(PANEL_change_zaxis,_("Show axis vectors"),&showgravity_vector);
//   }
//   SceneMotionCB(ZAXIS_CUSTOM);
//   ROLLOUT_orientation->close();
//   zaxis_custom=0;

//   ROLLOUT_gslice = glui_motion->add_rollout_to_panel(ROLLOUT_motion, _("Slice motion"),false,SLICE_ROLLOUT,MotionRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_gslice, glui_motion);
//   ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_gslice,SLICE_ROLLOUT, glui_motion);

//   if(gslice_xyz[0]<-1000000.0&&gslice_xyz[1]<-1000000.0&&gslice_xyz[2]<-1000000.0){
//     gslice_xyz[0]=(xbar0+DENORMALIZE_X(xbar))/2.0;
//     gslice_xyz[1]=(ybar0+DENORMALIZE_Y(ybar))/2.0;
//     gslice_xyz[2]=(zbar0+DENORMALIZE_Z(zbar))/2.0;
//   }

//   PANEL_gslice_center = glui_motion->add_panel_to_panel(ROLLOUT_gslice,_("rotation center"),true);
//   SPINNER_gslice_center_x=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"x:",GLUI_SPINNER_FLOAT,gslice_xyz,GSLICE_TRANSLATE, GSliceCB);
//   SPINNER_gslice_center_y=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"y:",GLUI_SPINNER_FLOAT,gslice_xyz+1,GSLICE_TRANSLATE, GSliceCB);
//   SPINNER_gslice_center_z=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"z:",GLUI_SPINNER_FLOAT,gslice_xyz+2,GSLICE_TRANSLATE, GSliceCB);
//   SPINNER_gslice_center_x->set_float_limits(xbar0,DENORMALIZE_X(xbar),GLUI_LIMIT_CLAMP);
//   SPINNER_gslice_center_y->set_float_limits(ybar0,DENORMALIZE_Y(ybar),GLUI_LIMIT_CLAMP);
//   SPINNER_gslice_center_z->set_float_limits(zbar0,DENORMALIZE_Z(zbar),GLUI_LIMIT_CLAMP);
//   GSliceCB(GSLICE_TRANSLATE);

//   PANEL_gslice_normal = glui_motion->add_panel_to_panel(ROLLOUT_gslice,_("normal"),true);
//   SPINNER_gslice_normal_az=glui_motion->add_spinner_to_panel(PANEL_gslice_normal,"az:",GLUI_SPINNER_FLOAT,gslice_normal_azelev,GSLICE_NORMAL, GSliceCB);
//   SPINNER_gslice_normal_elev=glui_motion->add_spinner_to_panel(PANEL_gslice_normal,"elev:",GLUI_SPINNER_FLOAT,gslice_normal_azelev+1,GSLICE_NORMAL, GSliceCB);
//   GSliceCB(GSLICE_NORMAL);

//   PANEL_gslice_show = glui_motion->add_panel_to_panel(ROLLOUT_gslice,_("show"),true);
//   CHECKBOX_gslice_data=glui_motion->add_checkbox_to_panel(PANEL_gslice_show,_("data"),&vis_gslice_data);
//   glui_motion->add_checkbox_to_panel(PANEL_gslice_show,"triangle outline",&show_gslice_triangles);
//   glui_motion->add_checkbox_to_panel(PANEL_gslice_show,"triangulation",&show_gslice_triangulation);
//   glui_motion->add_checkbox_to_panel(PANEL_gslice_show,"plane normal",&show_gslice_normal);

//   ROLLOUT_viewA = glui_motion->add_rollout(_("View"), false, VIEW_ROLLOUT, MVRRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_viewA, glui_motion);
//   ADDPROCINFO(mvrprocinfo, nmvrprocinfo, ROLLOUT_viewA, VIEW_ROLLOUT, glui_motion);

//   ROLLOUT_viewpoints = glui_motion->add_rollout_to_panel(ROLLOUT_viewA,_("Viewpoints"), false,VIEWPOINTS_ROLLOUT,MotionRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_viewpoints, glui_motion);
//   ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_viewpoints,VIEWPOINTS_ROLLOUT, glui_motion);

//   LIST_viewpoints = glui_motion->add_listbox_to_panel(ROLLOUT_viewpoints, _("Select:"), &i_view_list, LIST_VIEW, ViewpointCB);
//   LIST_viewpoints->set_alignment(GLUI_ALIGN_CENTER);

//   PANEL_reset = glui_motion->add_panel_to_panel(ROLLOUT_viewpoints, "", false);

//   PANEL_reset1 = glui_motion->add_panel_to_panel(PANEL_reset, "", false);

//   BUTTON_delete_view = glui_motion->add_button_to_panel(PANEL_reset1, _("Delete"), DELETE_VIEW, ViewpointCB);
//   delete_view_is_disabled = 0;
//   BUTTON_startup = glui_motion->add_button_to_panel(PANEL_reset1, _("Apply at startup"), STARTUP, ViewpointCB);
//   BUTTON_cycle_views = glui_motion->add_button_to_panel(PANEL_reset1, _("Cycle"), CYCLEVIEWS, ViewpointCB);

//   glui_motion->add_column_to_panel(PANEL_reset, true);
//   PANEL_reset2 = glui_motion->add_panel_to_panel(PANEL_reset, "", false);

//   BUTTON_replace_view = glui_motion->add_button_to_panel(PANEL_reset2, _("Replace"), REPLACE_VIEW, ViewpointCB);
//   BUTTON_add_view = glui_motion->add_button_to_panel(PANEL_reset2, _("Add"), ADD_VIEW, ViewpointCB);
//   EDIT_view_label = glui_motion->add_edittext_to_panel(PANEL_reset2, _("Edit:"), GLUI_EDITTEXT_TEXT, camera_label, LABEL_VIEW, ViewpointCB);

//   ROLLOUT_projection = glui_motion->add_rollout_to_panel(ROLLOUT_viewA,_("Window properties"), false,WINDOW_ROLLOUT,MotionRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_projection, glui_motion);
//   ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_projection,WINDOW_ROLLOUT, glui_motion);

//   RADIO_projection = glui_motion->add_radiogroup_to_panel(ROLLOUT_projection, &projection_type, PROJECTION, SceneMotionCB);
//   RADIOBUTTON_1a = glui_motion->add_radiobutton_to_group(RADIO_projection, _("Perspective"));
//   RADIOBUTTON_1b = glui_motion->add_radiobutton_to_group(RADIO_projection, _("Size preserving"));
//   SPINNER_zoom = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("Zoom"), GLUI_SPINNER_FLOAT, &zoom, ZOOM, SceneMotionCB);
//   SPINNER_zoom->set_float_limits(zoom_min, zoom_max, GLUI_LIMIT_CLAMP);
//   aperture_glui = Zoom2Aperture(zoom);
//   aperture_min = Zoom2Aperture(zoom_max);
//   aperture_max = Zoom2Aperture(zoom_min);
//   SPINNER_aperture = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("aperture"), GLUI_SPINNER_FLOAT, &aperture_glui,
//     APERTURE, SceneMotionCB);
//   glui_motion->add_separator_to_panel(ROLLOUT_projection);

//   LIST_windowsize = glui_motion->add_listbox_to_panel(ROLLOUT_projection, _("Size:"), &windowsize_pointer, WINDOWSIZE_LIST, SceneMotionCB);
//   LIST_windowsize->add_item(0, _("Custom"));
//   LIST_windowsize->add_item(1, "-");
//   LIST_windowsize->add_item(2, "320x240");
//   LIST_windowsize->add_item(3, "640x480");
//   LIST_windowsize->add_item(7, "720x480");
//   if(max_screenWidth >= 800 && max_screenHeight >= 480)LIST_windowsize->add_item(4, "800x640");
//   if(max_screenWidth >= 1024 && max_screenHeight >= 768)  LIST_windowsize->add_item(5, "1024x768");
//   if(max_screenWidth >= 1280 && max_screenHeight >= 720)  LIST_windowsize->add_item(9, "1280x720");
//   if(max_screenWidth >= 1280 && max_screenHeight >= 1024)  LIST_windowsize->add_item(6, "1280x1024");
//   if(max_screenWidth >= 1440 && max_screenHeight >= 1024)  LIST_windowsize->add_item(10, "1440x1080");
//   if(max_screenWidth >= 1920 && max_screenHeight >= 1080)  LIST_windowsize->add_item(8, "1920x1080");
//   UpdateWindowSizeList();

//   SPINNER_window_width = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("width"), GLUI_SPINNER_INT, &glui_screenWidth);
//   SPINNER_window_width->set_int_limits(100, max_screenWidth);
//   SPINNER_window_height = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("height"), GLUI_SPINNER_INT, &glui_screenHeight);
//   SPINNER_window_height->set_int_limits(100, max_screenHeight);

//   PANEL_colors = glui_motion->add_panel_to_panel(ROLLOUT_projection, "Colors", true);

//   ROLLOUT_foreground = glui_motion->add_rollout_to_panel(PANEL_colors,_("Background"), false);
//   INSERT_ROLLOUT(ROLLOUT_foreground, glui_motion);
//   SPINNER_foreground_red = glui_motion->add_spinner_to_panel(ROLLOUT_foreground,_("red"),GLUI_SPINNER_INT,glui_foregroundbasecolor,WINDOW_COLORS,SceneMotionCB);
//   SPINNER_foreground_green = glui_motion->add_spinner_to_panel(ROLLOUT_foreground, _("green"), GLUI_SPINNER_INT, glui_foregroundbasecolor+1, WINDOW_COLORS, SceneMotionCB);
//   SPINNER_foreground_blue = glui_motion->add_spinner_to_panel(ROLLOUT_foreground, _("blue"), GLUI_SPINNER_INT, glui_foregroundbasecolor+2, WINDOW_COLORS, SceneMotionCB);
//   SPINNER_foreground_red->set_int_limits(0, 255);
//   SPINNER_foreground_green->set_int_limits(0, 255);
//   SPINNER_foreground_blue->set_int_limits(0, 255);

//   ROLLOUT_background = glui_motion->add_rollout_to_panel(PANEL_colors, _("Foreground"), false);
//   INSERT_ROLLOUT(ROLLOUT_background, glui_motion);
//   SPINNER_background_red = glui_motion->add_spinner_to_panel(ROLLOUT_background,_("red"),GLUI_SPINNER_INT,glui_backgroundbasecolor,WINDOW_COLORS,SceneMotionCB);
//   SPINNER_background_green = glui_motion->add_spinner_to_panel(ROLLOUT_background,_("green"),GLUI_SPINNER_INT,glui_backgroundbasecolor+1,WINDOW_COLORS,SceneMotionCB);
//   SPINNER_background_blue = glui_motion->add_spinner_to_panel(ROLLOUT_background,_("blue"),GLUI_SPINNER_INT,glui_backgroundbasecolor+2,WINDOW_COLORS,SceneMotionCB);
//   SPINNER_background_red->set_int_limits(0, 255);
//   SPINNER_background_green->set_int_limits(0, 255);
//   SPINNER_background_blue->set_int_limits(0, 255);

//   BUTTON_flip = glui_motion->add_button_to_panel(PANEL_colors, _("Flip"), COLOR_FLIP, SceneMotionCB);

//   BUTTON_window_update = glui_motion->add_button_to_panel(ROLLOUT_projection, _("Apply"), WINDOW_RESIZE, SceneMotionCB);

//   ROLLOUT_scale = glui_motion->add_rollout_to_panel(ROLLOUT_viewA,_("Scaling"),false,SCALING_ROLLOUT,MotionRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_scale, glui_motion);
//   ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_scale,SCALING_ROLLOUT, glui_motion);

//   SPINNER_scalex=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_A(_("Scale")," x"),GLUI_SPINNER_FLOAT,mscale);
//   SPINNER_scalex->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

//   SPINNER_scaley=glui_motion->add_spinner_to_panel(ROLLOUT_scale, _A(_("Scale"), " y"),GLUI_SPINNER_FLOAT,mscale+1);
//   SPINNER_scaley->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

//   SPINNER_scalez=glui_motion->add_spinner_to_panel(ROLLOUT_scale, _A(_("Scale"), " z"),GLUI_SPINNER_FLOAT,mscale+2);
//   SPINNER_scalez->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

//   SPINNER_nearclip=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_("Near depth"),GLUI_SPINNER_FLOAT,&nearclip, NEARFARCLIP, SceneMotionCB);

//   SPINNER_farclip=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_("Far depth"),GLUI_SPINNER_FLOAT,&farclip, NEARFARCLIP, SceneMotionCB);

//   ROLLOUT_render = glui_motion->add_rollout(_("Render"), false, RENDER_ROLLOUT, MVRRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_render, glui_motion);
//   ADDPROCINFO(mvrprocinfo, nmvrprocinfo, ROLLOUT_render, RENDER_ROLLOUT, glui_motion);

//   BUTTON_render_start = glui_motion->add_button_to_panel(ROLLOUT_render, _("Start rendering"), RENDER_START_TOP, RenderCB);
//   glui_motion->add_button_to_panel(ROLLOUT_render, _("Stop rendering"), RENDER_STOP, RenderCB);

//   ROLLOUT_name = glui_motion->add_rollout_to_panel(ROLLOUT_render, "File name/type", false, RENDER_FILE_ROLLOUT, SubRenderRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_name, glui_motion);
//   ADDPROCINFO(subrenderprocinfo,nsubrenderprocinfo,ROLLOUT_name,RENDER_FILE_ROLLOUT, glui_motion);

//   EDIT_render_file_base = glui_motion->add_edittext_to_panel(ROLLOUT_name, "prefix:", GLUI_EDITTEXT_TEXT, render_file_base);
//   EDIT_render_file_base->set_w(200);

//   PANEL_render_file = glui_motion->add_panel_to_panel(ROLLOUT_name, "", false);

//   PANEL_file_suffix = glui_motion->add_panel_to_panel(PANEL_render_file, "suffix:", true);
//   RADIO_render_label = glui_motion->add_radiogroup_to_panel(PANEL_file_suffix, &render_label_type, RENDER_LABEL, RenderCB);
//   RADIOBUTTON_1f = glui_motion->add_radiobutton_to_group(RADIO_render_label, "frame number");
//   RADIOBUTTON_1g = glui_motion->add_radiobutton_to_group(RADIO_render_label, "time (s)");

//   glui_motion->add_column_to_panel(PANEL_render_file, false);

//   PANEL_file_type = glui_motion->add_panel_to_panel(PANEL_render_file, "type:", true);
//   RADIO_render_type = glui_motion->add_radiogroup_to_panel(PANEL_file_type, &render_filetype, RENDER_TYPE, RenderCB);
//   glui_motion->add_radiobutton_to_group(RADIO_render_type, "png");
//   glui_motion->add_radiobutton_to_group(RADIO_render_type, "jpg");

// #ifdef pp_HTML
//   glui_motion->add_button_to_panel(PANEL_render_file, "Render to html", RENDER_HTML, RenderCB);
// #endif

//   LIST_render_skip = glui_motion->add_listbox_to_panel(ROLLOUT_render, _("Show:"), &render_skip, RENDER_SKIP, RenderCB);
//   LIST_render_skip->add_item(RENDER_CURRENT_SINGLE, _("Current"));
//   LIST_render_skip->add_item(1, _("All frames"));
//   LIST_render_skip->add_item(2, _("Every 2nd frame"));
//   LIST_render_skip->add_item(3, _("Every 3rd frame"));
//   LIST_render_skip->add_item(4, _("Every 4th frame"));
//   LIST_render_skip->add_item(5, _("Every 5th frame"));
//   LIST_render_skip->add_item(10, _("Every 10th frame"));
//   LIST_render_skip->add_item(20, _("Every 20th frame"));
//   LIST_render_skip->set_int_val(render_skip);

//   ROLLOUT_image_size = glui_motion->add_rollout_to_panel(ROLLOUT_render, "size/type", false, RENDER_SIZE_ROLLOUT, SubRenderRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_image_size, glui_motion);
//   ADDPROCINFO(subrenderprocinfo,nsubrenderprocinfo,ROLLOUT_image_size,RENDER_SIZE_ROLLOUT, glui_motion);

//   RADIO_render_resolution = glui_motion->add_radiogroup_to_panel(ROLLOUT_image_size, &render_resolution);
//   glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("320x240"));
//   glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("640x480"));
//   {
//     RADIOBUTTON_render_current=glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("current"));
//     RADIOBUTTON_render_high=glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("high"));
//   }
//   RADIOBUTTON_render_360=glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("360"));

//   render_size_index = RenderWindow;
//   RenderCB(RENDER_RESOLUTION);

//   glui_resolution_multiplier=CLAMP(resolution_multiplier,2,10);
//   SPINNER_resolution_multiplier = glui_motion->add_spinner_to_panel(ROLLOUT_image_size, "multiplier:", GLUI_SPINNER_INT, &glui_resolution_multiplier, RENDER_MULTIPLIER, RenderCB);
//   SPINNER_resolution_multiplier->set_int_limits(2, 10);
//   RenderCB(RENDER_MULTIPLIER);

//   PANEL_360 = glui_motion->add_panel_to_panel(ROLLOUT_image_size, (char *)deg360, true);
//   SPINNER_window_height360 = glui_motion->add_spinner_to_panel(PANEL_360, "height", GLUI_SPINNER_INT, &nheight360, RENDER_360CB, RenderCB);
//   SPINNER_window_height360->set_int_limits(100, max_screenHeight);
//   RenderCB(RENDER_360CB);
//   glui_motion->add_spinner_to_panel(PANEL_360, "margin", GLUI_SPINNER_INT, &margin360_size);

// #ifdef pp_RENDER360_DEBUG
//   PANEL_360_debug = glui_motion->add_panel_to_panel(PANEL_360, "grid", true);
//   glui_motion->add_checkbox_to_panel(PANEL_360_debug, "show", &debug_360);
//   SPINNER_360_skip_x = glui_motion->add_spinner_to_panel(PANEL_360_debug, "horizontal skip", GLUI_SPINNER_INT, &debug_360_skip_x, RENDER_DEBUG_360, RenderCB);
//   SPINNER_360_skip_y = glui_motion->add_spinner_to_panel(PANEL_360_debug, "vertical skip", GLUI_SPINNER_INT, &debug_360_skip_y, RENDER_DEBUG_360, RenderCB);
//   RenderCB(RENDER_DEBUG_360);

//   NewMemory((void **)&CHECKBOX_screenvis, nscreeninfo * sizeof(GLUI_Checkbox *));

//   ROLLOUT_screenvis = glui_motion->add_rollout_to_panel(ROLLOUT_render, "screenvis", false, RENDER_SCREEN_ROLLOUT, SubRenderRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_screenvis, glui_motion);
//   ADDPROCINFO(subrenderprocinfo,nsubrenderprocinfo,ROLLOUT_screenvis,RENDER_SCREEN_ROLLOUT, glui_motion);

//   CHECKBOX_screenview = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "view screens", &screenview);
//   CHECKBOX_screenvis[0] = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "bottom", screenvis);

//   ROLLOUT_lower = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "lower", false);
//   INSERT_ROLLOUT(ROLLOUT_lower, glui_motion);
//   CHECKBOX_screenvis[1] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "1", screenvis + 1);
//   CHECKBOX_screenvis[2] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "2", screenvis + 2);
//   CHECKBOX_screenvis[3] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "3", screenvis + 3);
//   CHECKBOX_screenvis[4] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "4", screenvis + 4);
//   CHECKBOX_screenvis[5] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "5", screenvis + 5);
//   CHECKBOX_screenvis[6] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "6", screenvis + 6);
//   CHECKBOX_screenvis[7] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "7", screenvis + 7);
//   CHECKBOX_screenvis[8] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "8", screenvis + 8);


//   ROLLOUT_middle = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "middle", false);
//   INSERT_ROLLOUT(ROLLOUT_middle, glui_motion);
//   CHECKBOX_screenvis[9] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "1", screenvis + 9);
//   CHECKBOX_screenvis[10] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "2", screenvis + 10);
//   CHECKBOX_screenvis[11] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "3", screenvis + 11);
//   CHECKBOX_screenvis[12] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "4", screenvis + 12);
//   CHECKBOX_screenvis[13] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "5", screenvis + 13);
//   CHECKBOX_screenvis[14] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "6", screenvis + 14);
//   CHECKBOX_screenvis[15] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "7", screenvis + 15);
//   CHECKBOX_screenvis[16] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "8", screenvis + 16);

//   ROLLOUT_upper = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "upper", false);
//   INSERT_ROLLOUT(ROLLOUT_upper, glui_motion);
//   CHECKBOX_screenvis[17] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "1", screenvis + 17);
//   CHECKBOX_screenvis[18] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "2", screenvis + 18);
//   CHECKBOX_screenvis[19] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "3", screenvis + 19);
//   CHECKBOX_screenvis[20] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "4", screenvis + 20);
//   CHECKBOX_screenvis[21] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "5", screenvis + 21);
//   CHECKBOX_screenvis[22] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "6", screenvis + 22);
//   CHECKBOX_screenvis[23] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "7", screenvis + 23);
//   CHECKBOX_screenvis[24] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "8", screenvis + 24);

//   CHECKBOX_screenvis[25] = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "top", screenvis + 25);
//   BUTTON_screen_showall = glui_motion->add_button_to_panel(ROLLOUT_screenvis, _("Show all"), SHOWALL_SCREENS, ViewpointCB);
//   BUTTON_screen_hideall = glui_motion->add_button_to_panel(ROLLOUT_screenvis, _("Hide all"), HIDEALL_SCREENS, ViewpointCB);
// #endif

//   UpdateGluiFileLabel(render_label_type);

//   ROLLOUT_scene_clip = glui_motion->add_rollout_to_panel(ROLLOUT_render, "Clipping region", false, RENDER_CLIP_ROLLOUT, SubRenderRolloutCB);
//   INSERT_ROLLOUT(ROLLOUT_scene_clip, glui_motion);
//   ADDPROCINFO(subrenderprocinfo,nsubrenderprocinfo,ROLLOUT_scene_clip,RENDER_CLIP_ROLLOUT, glui_motion);

//   SPINNER_clip_left = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "left:", GLUI_SPINNER_INT, &render_clip_left);
//   SPINNER_clip_left->set_int_limits(0, screenWidth);

//   SPINNER_clip_right = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "right:", GLUI_SPINNER_INT, &render_clip_right);
//   SPINNER_clip_right->set_int_limits(0, screenWidth);

//   SPINNER_clip_bottom = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "bottom:", GLUI_SPINNER_INT, &render_clip_bottom);
//   SPINNER_clip_bottom->set_int_limits(0, screenHeight);

//   SPINNER_clip_top = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "top:", GLUI_SPINNER_INT, &render_clip_top);
//   SPINNER_clip_top->set_int_limits(0, screenHeight);

//   CHECKBOX_clip_rendered_scene = glui_motion->add_checkbox_to_panel(ROLLOUT_scene_clip, "clip rendered scene", &clip_rendered_scene);

//   if(have_ffmpeg == 1){
//     ROLLOUT_make_movie = glui_motion->add_rollout("Movie", false, MOVIE_ROLLOUT, MVRRolloutCB);
//     INSERT_ROLLOUT(ROLLOUT_make_movie, glui_motion);
//     ADDPROCINFO(mvrprocinfo,nmvrprocinfo,ROLLOUT_make_movie,MOVIE_ROLLOUT, glui_motion);

//     CHECKBOX_overwrite_movie = glui_motion->add_checkbox_to_panel(ROLLOUT_make_movie, "Overwrite movie", &overwrite_movie);
//     glui_motion->add_button_to_panel(ROLLOUT_make_movie, _("Render normal"), RENDER_START_NORMAL, RenderCB);
//     BUTTON_make_movie = glui_motion->add_button_to_panel(ROLLOUT_make_movie, "Make movie", MAKE_MOVIE, RenderCB);
//     if(have_ffplay==1){
//       BUTTON_play_movie = glui_motion->add_button_to_panel(ROLLOUT_make_movie, "Play movie", PLAY_MOVIE, RenderCB);
//       EnableDisablePlayMovie();
//     }
//     glui_motion->add_separator_to_panel(ROLLOUT_make_movie);

//     EDIT_movie_name = glui_motion->add_edittext_to_panel(ROLLOUT_make_movie, "Movie prefix:", GLUI_EDITTEXT_TEXT, movie_name, MOVIE_NAME, RenderCB);
//     EDIT_movie_name->set_w(200);
//     PANEL_movie_type = glui_motion->add_panel_to_panel(ROLLOUT_make_movie, "Movie type:", true);
//     RADIO_movie_type = glui_motion->add_radiogroup_to_panel(PANEL_movie_type, &movie_filetype, MOVIE_FILETYPE, RenderCB);
//     RADIOBUTTON_movie_type[0]=glui_motion->add_radiobutton_to_group(RADIO_movie_type, "avi");
//     RADIOBUTTON_movie_type[1]=glui_motion->add_radiobutton_to_group(RADIO_movie_type, "mp4");
//     RADIOBUTTON_movie_type[2]=glui_motion->add_radiobutton_to_group(RADIO_movie_type, "wmv");
//     RADIOBUTTON_movie_type[3]=glui_motion->add_radiobutton_to_group(RADIO_movie_type, "mov");
//     SPINNER_framerate = glui_motion->add_spinner_to_panel(ROLLOUT_make_movie, "Frame rate", GLUI_SPINNER_INT, &movie_framerate);
//     SPINNER_framerate->set_int_limits(1, 100);
//     SPINNER_movie_crf = glui_motion->add_spinner_to_panel(ROLLOUT_make_movie, "quality", GLUI_SPINNER_INT, &movie_crf);
//     SPINNER_movie_crf->set_int_limits(0,51);
//     SPINNER_bitrate = glui_motion->add_spinner_to_panel(ROLLOUT_make_movie, "Bit rate (Kb/s)", GLUI_SPINNER_INT, &movie_bitrate);
//     SPINNER_bitrate->set_int_limits(1, 100000);
//     glui_motion->add_button_to_panel(ROLLOUT_make_movie, "Output ffmpeg command", OUTPUT_FFMPEG, RenderCB);
//     RenderCB(MOVIE_FILETYPE);
//   }

//   PANEL_close = glui_motion->add_panel("",GLUI_PANEL_NONE);

//   BUTTON_motion_1=glui_motion->add_button_to_panel(PANEL_close,_("Save settings"),SAVE_SETTINGS, MotionDlgCB);

//   glui_motion->add_column_to_panel(PANEL_close,false);

//   BUTTON_motion_2=glui_motion->add_button_to_panel(PANEL_close,_("Close"),1, MotionDlgCB);

//   ShowHideTranslate(rotation_type);
//   glui_motion->set_main_gfx_window( main_window );
  return 0;
}
