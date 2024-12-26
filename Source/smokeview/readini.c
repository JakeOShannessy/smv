
#include "options.h"
#include "glew.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "stdio_buffer.h"
#include "glui_motion.h"
#include "glui_bounds.h"
#include "shared_structures.h"
#include "readimage.h"
#include "readhvac.h"
#include "readgeom.h"
#include "readslice.h"
#include "readobject.h"
#include "readlabel.h"
#include "readsmoke.h"
#include "IOobjects.h"
#include "IOscript.h"


/* ------------------ GetNewBoundIndex ------------------------ */

int GetNewBoundIndex(int old_index){
#define OLD_PERCENTILE 0
#define OLD_SET        1
#define OLD_GLOBAL     2

#define NEW_SET        0
#define NEW_GLOBAL     1
#define NEW_PERCENTILE 3

  int bound_map[] = {NEW_GLOBAL, NEW_SET, NEW_GLOBAL};

  assert(old_index>=0&&old_index<=2);
  old_index=CLAMP(old_index,0, 2);
  return bound_map[old_index];
}

/* ------------------ SetHVACDuctBounds ------------------------ */

void SetHVACDuctBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *quantity){
  int i;

  for(i = 0; i < nhvacductbounds; i++){
    boundsdata *boundi;

    boundi = hvacductbounds + i;
    if(strcmp(quantity, "") == 0 || strcmp(quantity, boundi->shortlabel) == 0){
      hvacductbounds[i].dlg_setvalmin = glui_setpatchmin;
      hvacductbounds[i].dlg_setvalmax = glui_setpatchmax;
      GLUISetMinMax(BOUND_HVACDUCT, boundi->shortlabel, set_valmin, valmin, set_valmax, valmax);
      update_glui_bounds = 1;
    }
  }
}

/* ------------------ SetBoundBounds ------------------------ */

void SetBoundBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *quantity){
  int i;

  GLUI2GlobalBoundaryBounds(quantity);
  for(i = 0; i<npatchbounds; i++){
    boundsdata *boundi;

    boundi = patchbounds+i;
    if(strcmp(quantity, "")==0||strcmp(quantity, boundi->shortlabel)==0){
      patchbounds[i].dlg_setvalmin = glui_setpatchmin;
      patchbounds[i].dlg_setvalmax = glui_setpatchmax;
      GLUISetMinMax(BOUND_PATCH, boundi->shortlabel, set_valmin, valmin, set_valmax, valmax);
      update_glui_bounds = 1;
    }
  }
}

/* ------------------ SetSliceBounds ------------------------ */

void SetSliceBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *buffer2){
  int i;

  for(i = 0; i<nslicebounds; i++){
    if(strcmp(buffer2, "")==0||strcmp(slicebounds[i].shortlabel, buffer2)==0){
      slicebounds[i].dlg_setvalmin = set_valmin;
      slicebounds[i].dlg_setvalmax = set_valmax;
      slicebounds[i].dlg_valmin = valmin;
      slicebounds[i].dlg_valmax = valmax;
      GLUISetMinMax(BOUND_SLICE, slicebounds[i].shortlabel, set_valmin, valmin, set_valmax, valmax);
      update_glui_bounds = 1;
      if(strcmp(slicebounds[i].shortlabel, buffer2)==0){
        break;
      }
    }
  }
}

/* ------------------ SetPatchMin ------------------------ */

void SetPatchMin(int set_valmin, float valmin, char *buffer2){
  int i;

  for(i = 0; i < npatchbounds; i++){
    if(strcmp(buffer2, "") == 0 || strcmp(patchbounds[i].shortlabel, buffer2) == 0){
      patchbounds[i].dlg_setvalmin = set_valmin;
      patchbounds[i].dlg_valmin = valmin;
      GLUISetMin(BOUND_PATCH, patchbounds[i].shortlabel, set_valmin, valmin);
      update_glui_bounds = 1;
      if(strcmp(patchbounds[i].shortlabel, buffer2) == 0)break;
    }
  }
}

/* ------------------ SetPatchMax ------------------------ */

void SetPatchMax(int set_valmax, float valmax, char *buffer2){
  int i;

  for(i = 0; i < npatchbounds; i++){
    if(strcmp(buffer2, "") == 0 || strcmp(patchbounds[i].shortlabel, buffer2) == 0){
      patchbounds[i].dlg_setvalmax = set_valmax;
      patchbounds[i].dlg_valmax = valmax;
      GLUISetMax(BOUND_PATCH, patchbounds[i].shortlabel, set_valmax, valmax);
      update_glui_bounds = 1;
      if(strcmp(patchbounds[i].shortlabel, buffer2) == 0)break;
    }
  }
}
/* ------------------ SetSliceMin ------------------------ */

void SetSliceMin(int set_valmin, float valmin, char *buffer2){
  int i;

  for(i = 0; i < nslicebounds; i++){
    if(strcmp(buffer2, "") == 0 || strcmp(slicebounds[i].shortlabel, buffer2) == 0){
      slicebounds[i].dlg_setvalmin = set_valmin;
      slicebounds[i].dlg_valmin    = valmin;
      GLUISetMin(BOUND_SLICE, slicebounds[i].shortlabel, set_valmin, valmin);
      update_glui_bounds = 1;
      if(strcmp(slicebounds[i].shortlabel, buffer2) == 0)break;
    }
  }
}

/* ------------------ SetSliceMax ------------------------ */

void SetSliceMax(int set_valmax, float valmax, char *buffer2){
  int i;

  for(i = 0; i < nslicebounds; i++){
    if(strcmp(buffer2, "") == 0 || strcmp(slicebounds[i].shortlabel, buffer2) == 0){
      slicebounds[i].dlg_setvalmax = set_valmax;
      slicebounds[i].dlg_valmax    = valmax;
      GLUISetMax(BOUND_SLICE, slicebounds[i].shortlabel, set_valmax, valmax);
      update_glui_bounds = 1;
      if(strcmp(slicebounds[i].shortlabel, buffer2) == 0)break;
    }
  }
}


/* ------------------ SetHVACNodeBounds ------------------------ */

void SetHVACNodeBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *quantity){
  int i;

  for(i = 0; i < nhvacnodebounds; i++){
    boundsdata *boundi;

    boundi = hvacnodebounds + i;
    if(strcmp(quantity, "") == 0 || strcmp(quantity, boundi->shortlabel) == 0){
      hvacnodebounds[i].dlg_setvalmin = glui_setpatchmin;
      hvacnodebounds[i].dlg_setvalmax = glui_setpatchmax;
      GLUISetMinMax(BOUND_HVACNODE, boundi->shortlabel, set_valmin, valmin, set_valmax, valmax);
      update_glui_bounds = 1;
    }
  }
}


/* ------------------ ReadIni2 ------------------------ */

int ReadIni2(smv_case *scase, const char *inifile, int localfile){
  int i;
  FILE *stream;
  int have_tours=0, have_tour7=0;

  updatemenu = 1;
  updatefacelists = 1;

  if((stream = fopen(inifile, "r")) == NULL)return 1;
  if(readini_output==1){
    if(verbose_output==1)PRINTF("reading %s ", inifile);
  }

  for(i = 0; i<nunitclasses_ini; i++){
    f_units *uc;

    uc = unitclasses_ini + i;
    FREEMEMORY(uc->units);
  }
  FREEMEMORY(unitclasses_ini);
  nunitclasses_ini = 0;

  if(localfile == 1){
    UpdateINIList(scase);
  }

  if(localfile == 1){
    update_selectedtour_index = 0;
  }

  /* find number of each kind of file */

  while(!feof(stream)){
    char buffer[255], buffer2[255];

    CheckMemory;
    if(fgets(buffer, 255, stream) == NULL)break;

    if(MatchINI(buffer, "RESEARCHMODE") == 1){
      int dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %f %i %i %i %i %i %i %i", &research_mode, &dummy, &colorbar_shift, &ncolorlabel_digits, &force_fixedpoint, &ngridloc_digits, &sliceval_ndigits, &force_exponential, &force_decimal, &force_zero_pad);
      colorbar_shift = CLAMP(colorbar_shift, COLORBAR_SHIFT_MIN, COLORBAR_SHIFT_MAX);
      if(research_mode==1&&research_mode_override==0)research_mode=0;
      ncolorlabel_digits = CLAMP(ncolorlabel_digits, COLORBAR_NDECIMALS_MIN, COLORBAR_NDECIMALS_MAX);
      sliceval_ndigits   = CLAMP(sliceval_ndigits, 0, 10);
      ngridloc_digits    = CLAMP(ngridloc_digits, GRIDLOC_NDECIMALS_MIN, GRIDLOC_NDECIMALS_MAX);
      ONEORZERO(research_mode);
      ONEORZERO(force_fixedpoint);
      ONEORZERO(force_exponential);
      ONEORZERO(force_decimal);
      ONEORZERO(force_zero_pad);
      if(force_fixedpoint == 1 && force_exponential == 1)force_exponential = 0;
      update_research_mode=1;
      continue;
    }
#ifdef pp_FRAME
    if(MatchINI(buffer, "FRAMETHREADS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &nframe_threads);
    }
#endif
    if(MatchINI(buffer, "LOADMESH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i", &show_intersection_box, &show_intersected_meshes);

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %f %i %f", use_meshclip + 0, meshclip + 0, use_meshclip + 1, meshclip + 1);

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %f %i %f", use_meshclip + 2, meshclip + 2, use_meshclip + 3, meshclip + 3);

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %f %i %f", use_meshclip + 4, meshclip + 4, use_meshclip + 5, meshclip + 5);

      for(i = 0;i < 6;i++){
        if(use_meshclip[i] != 0)use_meshclip[i] = 1;
      }
      if(show_intersection_box != 0)show_intersection_box = 1;
      if(show_intersected_meshes != 0)show_intersected_meshes = 1;
      update_meshclip = 1;
    }
    if(MatchINI(buffer, "GEOMDOMAIN") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i ", &showgeom_inside_domain, &showgeom_outside_domain);
      showgeom_inside_domain = CLAMP(showgeom_inside_domain, 0, 1);
      showgeom_outside_domain = CLAMP(showgeom_outside_domain, 0, 1);
      continue;
    }
    if(MatchINI(buffer, "SLICEDUP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i", &slicedup_option, &vectorslicedup_option,&boundaryslicedup_option);
      continue;
    }
    if(MatchINI(buffer, "SHOWBOUNDS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i", &bounds_each_mesh, &show_bound_diffs);
      continue;
    }
    if(MatchINI(buffer, "BLENDMODE")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i,%i", &slices3d_max_blending, &hrrpuv_max_blending,&showall_3dslices);
    }
    if(MatchINI(buffer, "FREEZEVOLSMOKE")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i", &freeze_volsmoke,&autofreeze_volsmoke);
      continue;
    }
    if(MatchINI(buffer, "VISBOUNDARYTYPE")==1){
      int *vbt = vis_boundary_type;

      update_ini_boundary_type = 1;
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i %i %i %i", vbt,vbt+1,vbt+2,vbt+3,vbt+4,vbt+5,vbt+6, &show_mirror_boundary, &show_mirror_boundary);
      show_all_interior_patch_data = vbt[INTERIORwall];
      hide_all_interior_patch_data = 1 - show_all_interior_patch_data;
      continue;
    }
    if(MatchINI(buffer, "GEOMBOUNDARYPROPS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %f %f %i", &show_boundary_shaded, &show_boundary_outline, &show_boundary_points, &geomboundary_linewidth, &geomboundary_pointsize, &boundary_edgetype);
      ONEORZERO(show_boundary_shaded);
      ONEORZERO(show_boundary_outline);
      ONEORZERO(show_boundary_points);
      ONEORZERO(boundary_edgetype);
      continue;
    }
    if(MatchINI(buffer, "HVACVIEW") == 1&&hvaccoll.hvacinfo!=NULL&&hvaccoll.nhvacinfo > 0){
      int nh, dummy;
      float rdummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %f %i",
        &nh, &hvac_metro_view, &dummy, &dummy, &rdummy, &hvac_cell_view);
      ONEORZERO(hvac_metro_view);
      ONEORZERO(hvac_cell_view);

      nh = MIN(hvaccoll.nhvacinfo, nh);
      for(i = 0; i < nh; i++){
        hvacdata *hvaci;
        int dc[3], nc[3];
        int j;

        hvaci = hvaccoll.hvacinfo + i;
        fgets(buffer, 255, stream);
        sscanf(buffer, " %i %i %i %i %i %f %f %f %f %f",
          &hvaci->display,  &hvaci->show_node_labels, &hvaci->show_duct_labels,
          &hvaci->show_component, &hvaci->show_filters, &hvaci->duct_width,
          &hvaci->node_size, &hvaci->component_size, &hvaci->filter_size, &hvaci->cell_node_size);
        fgets(buffer, 255, stream);
        sscanf(buffer, " %i %i %i %i %i %i", dc, dc + 1, dc + 2, nc, nc + 1, nc + 2);
        for(j=0;j<3;j++){
          hvaci->duct_color[j] = CLAMP(dc[j], 0, 255);
          hvaci->node_color[j] = CLAMP(nc[j], 0, 255);
        }
        hvaci->duct_width     = MAX(1.0, hvaci->duct_width);
        hvaci->node_size     = MAX(1.0, hvaci->node_size);
        hvaci->cell_node_size = MAX(1.0, hvaci->cell_node_size);
        hvaci->component_size = MAX(0.1, hvaci->component_size);
        hvaci->filter_size    = MAX(0.1, hvaci->filter_size);
        ONEORZERO(hvaci->show_node_labels);
        ONEORZERO(hvaci->show_duct_labels);
      }
    }
    if(MatchINI(buffer, "SHOWSLICEVALS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i", show_slice_values, show_slice_values+1, show_slice_values+2);
    }
    if(MatchINI(buffer, "GEOMCELLPROPS")==1){
      int vector_slice[3] = {-1, -1, -1};
      float dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i",
        &slice_celltype);
      slice_celltype = CLAMP(slice_celltype,0,MAX_CELL_TYPES-1);

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i",
        slice_edgetypes,slice_edgetypes+1,slice_edgetypes+2);

      for(i=0;i<3;i++){
        slice_edgetypes[i] = CLAMP(slice_edgetypes[i],0,2);
      }

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %f %i %i %i",
        show_slice_shaded,show_slice_shaded+1,show_slice_shaded+2,&dummy, vector_slice, vector_slice+1, vector_slice+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i",
        show_slice_outlines,show_slice_outlines+1,show_slice_outlines+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i",
        show_slice_points,show_slice_points+1,show_slice_points+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i",
             show_vector_slice, show_vector_slice+1, show_vector_slice+2);

      for(i=0;i<MAX_CELL_TYPES;i++){
        show_slice_shaded[i]   = CLAMP(show_slice_shaded[i],0,1);
        show_slice_outlines[i] = CLAMP(show_slice_outlines[i],0,1);
        show_slice_points[i]   = CLAMP(show_slice_points[i],0,1);
        if(vector_slice[i]>=0)show_vector_slice[i] = CLAMP(vector_slice[i], 0, 1);
      }
      continue;
    }
    if(MatchINI(buffer, "NORTHANGLE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &vis_northangle);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %f", northangle_position, northangle_position + 1, northangle_position + 2);
      continue;
    }
    if(MatchINI(buffer, "SHOWAVATAR")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &show_avatar);
      ONEORZERO(show_avatar);
      continue;
    }
    if(MatchINI(buffer, "TREEPARMS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i", &mintreesize, &vis_xtree, &vis_ytree, &vis_ztree);
      mintreesize = MAX(mintreesize, 2);
      vis_xtree = CLAMP(vis_xtree, 0, 1);
      vis_ytree = CLAMP(vis_ytree, 0, 1);
      vis_ztree = CLAMP(vis_ztree, 0, 1);
      continue;
    }
    if(MatchINI(buffer, "COLORBAR_SPLIT") == 1){
      int ii;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i ", colorsplit, colorsplit + 1, colorsplit + 2, colorsplit + 3, colorsplit + 4, colorsplit + 5);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i ", colorsplit + 6, colorsplit + 7, colorsplit + 8, colorsplit + 9, colorsplit + 10, colorsplit + 11);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %f ", splitvals, splitvals + 1, splitvals + 2);

      for(ii = 0; ii < 12; ii++){
        colorsplit[ii] = CLAMP(colorsplit[ii], 0, 255);
      }
      if(scriptinfo == NULL){
        update_splitcolorbar = 1;
      }
      else{
        GLUISplitCB(SPLIT_COLORBAR);
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWGRAVVECTOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &showgravity_vector);
      continue;
    }
    if(MatchINI(buffer, "ZAXISANGLES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %f", zaxis_angles, zaxis_angles + 1, zaxis_angles + 2);
      zaxis_angles_orig[0] = zaxis_angles[0];
      zaxis_angles_orig[1] = zaxis_angles[1];
      zaxis_angles_orig[2] = zaxis_angles[2];
      zaxis_custom = 1;
      update_zaxis_custom = 1;
      continue;
    }
    if(MatchINI(buffer, "GEOMSHOW") == 1){
      int dummy, dummy2;
      float rdummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i %f %f %i %i %f %f %f",
        &dummy, &dummy2, &show_faces_shaded, &show_faces_outline, &smooth_geom_normal,
        &geom_force_transparent, &geom_transparency,&geom_linewidth, &use_geom_factors, &show_cface_normals, &geom_pointsize, &geom_dz_offset, &geom_norm_offset);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i", &dummy, &dummy, &dummy, &dummy);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %i %i %i %i", &geom_vert_exag, &rdummy, &dummy, &dummy2, &show_geom_boundingbox, &show_geom_bndf );
      continue;
    }
    if(MatchINI(buffer, "SHOWTRIANGLECOUNT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &show_triangle_count);
      continue;
    }
    if(MatchINI(buffer, "SHOWDEVICEVALS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i %i %i %i",
        &showdevice_val, &showvdevice_val, &devicetypes_index, &colordevice_val, &vectortype, &viswindrose, &showdevice_type, &showdevice_unit,&showdevice_id);
      devicetypes_index = CLAMP(devicetypes_index, 0, MAX(ndevicetypes - 1,0));
      update_glui_devices = 1;
      if(viswindrose==1)update_windrose = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWSLICEPLOT")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %f %f %i %i %f %f %f %i %i",
         slice_xyz, slice_xyz+1, slice_xyz+2,
         &plot2d_size_factor, &vis_slice_plot, &slice_plot_bound_option,
         slice_dxyz, slice_dxyz+1, slice_dxyz+2, &average_plot2d_slice_region, &show_plot2d_slice_position
      );
      continue;
    }
    if(MatchINI(buffer, "SHOWHRRPLOT")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %f %f %i", &glui_hrr, &hoc_hrr, &scase->fuel_hoc, &plot2d_size_factor, &vis_hrr_plot);
      continue;
    }
    if(MatchINI(buffer, "SHOWDEVICEPLOTS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %f %f %f %f %f %f %f",
             &vis_device_plot, &show_plot2d_xlabels, &show_plot2d_ylabels, &plot2d_size_factor, &plot2d_line_width, &plot2d_point_size,
             plot2d_xyz_offset, plot2d_xyz_offset+1, plot2d_xyz_offset+2, &plot2d_font_spacing
      );
      update_glui_devices = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWGENPLOTXLABEL") == 1){
      char *xlabelptr;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i, %f", &plot2d_show_xaxis_labels, &plot2d_xaxis_position);
      fgets(buffer, 255, stream);
      xlabelptr = TrimFrontBack(buffer);
      strcpy(plot2d_xaxis_label, xlabelptr);
    }
    if(MatchINI(buffer, "SHOWGENPLOTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &nplot2dini);

      FREEMEMORY(plot2dini);

      if(nplot2dini==0)continue;
      NewMemory((void **)&plot2dini, nplot2dini*sizeof(plot2ddata));

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i %f %i %i",
             &plot2d_show_plot_title, &plot2d_show_curve_labels, &plot2d_show_curve_values,
             &plot2d_show_xaxis_bounds, &plot2d_show_yaxis_bounds, &idevice_add, &plot2d_time_average,
             &plot2d_show_yaxis_units, &plot2d_show_plots
             );
      update_device_timeaverage = 1;
      GLUIUpdateDeviceAdd();
      for(i=0;i<nplot2dini;i++){
        plot2ddata *plot2di;
        char *labelptr;
        int j;

        plot2di = plot2dini + i;
        plot2di->plot_index = i;
        plot2di->mult_devc  = 0;
        fgets(buffer, 255, stream);
        TrimBack(buffer);
        labelptr = TrimFront(buffer);
        strcpy(plot2di->plot_label, labelptr);

        fgets(buffer, 255, stream);
        sscanf(buffer, " %f %f %f %i %i %i", plot2di->xyz, plot2di->xyz+1, plot2di->xyz+2, &plot2di->show, &plot2di->ncurves, &plot2di->mult_devc);
        fgets(buffer, 255, stream);
        sscanf(buffer, " %f %i %f %i %f %i %f %i ",
               plot2di->valmin,   plot2di->use_valmin,   plot2di->valmax,   plot2di->use_valmax,
               plot2di->valmin+1, plot2di->use_valmin+1, plot2di->valmax+1, plot2di->use_valmax+1);
        plot2di->bounds_defined = 0;
        for(j=0; j<plot2di->ncurves; j++){
          int color[3];
          float linewidth1;
          int file_index, col_index;
          curvedata *curve;
          float factor;
          int apply_factor;
          int use_foreground_color;

          fgets(buffer, 255, stream);
          TrimBack(buffer);
          linewidth1 = 1.0;
          factor = 1.0;
          apply_factor = 0;
          use_foreground_color = 0;
          sscanf(buffer, " %i %i %i %i %i %f %f %i %i",    &file_index, &col_index, color, color+1, color+2, &linewidth1, &factor, &apply_factor, &use_foreground_color);

          plot2di->curve[j].csv_file_index = file_index;
          plot2di->curve[j].csv_col_index  = col_index;
          curve                            = plot2di->curve+j;
          strcpy(curve->c_type, scase->csvcoll.csvfileinfo[file_index].c_type);
          curve->color[0]                  = color[0];
          curve->color[1]                  = color[1];
          curve->color[2]                  = color[2];
          curve->linewidth                 = linewidth1;
          curve->curve_factor              = factor;
          curve->apply_curve_factor        = apply_factor;
          curve->vals                      = NULL;
          curve->use_foreground_color      = use_foreground_color;
          if(strcmp(curve->c_type, "devc")==0){
            curve->quantity = scase->csvcoll.csvfileinfo[file_index].csvinfo[col_index].label.longlabel;
          }
          else{
            curve->quantity = NULL;
          }
        }
      }
      update_glui_devices = 1;
      update_plot2dini = 1;
      continue;
    }
    if(MatchINI(buffer, "GENPLOTLABELS") == 1){
      fgets(buffer, 255, stream);
      for(i=0;i<nplot2dini;i++){
        plot2ddata *plot2di;
        int j;

        plot2di = plot2dini + i;
        fgets(buffer, 255, stream);
        for(j=0; j<plot2di->ncurves; j++){
          curvedata *curve;

          curve = plot2di->curve + j;
          fgets(buffer, 255, stream);
          fgets(buffer, 255, stream);
          strcpy(curve->scaled_label, TrimFrontBack(buffer));
          fgets(buffer, 255, stream);
          strcpy(curve->scaled_unit, TrimFrontBack(buffer));
        }
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWMISSINGOBJECTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &show_missing_objects);
      ONEORZERO(show_missing_objects);
      continue;
    }
    if(MatchINI(buffer, "DEVICEVECTORDIMENSIONS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", &vector_baselength, &vector_basediameter, &vector_headlength, &vector_headdiameter);
      continue;
    }
    if(MatchINI(buffer, "DEVICEBOUNDS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", &device_valmin, &device_valmax);
      continue;
    }
    if(MatchINI(buffer, "DEVICEORIENTATION") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f", &show_device_orientation, &orientation_scale);
      show_device_orientation = CLAMP(show_device_orientation, 0, 1);
      orientation_scale = CLAMP(orientation_scale, 0.1, 10.0);
      continue;
    }
    if(MatchINI(buffer, "GVERSION") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &vis_title_gversion);
      ONEORZERO(vis_title_gversion);
    }
    if(MatchINI(buffer, "GVECDOWN") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &gvec_down);
      ONEORZERO(gvec_down);
    }
    if(MatchINI(buffer, "SCALEDFONT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &scaled_font2d_height, &scaled_font2d_height2width, &scaled_font2d_thickness);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &scaled_font3d_height, &scaled_font3d_height2width, &scaled_font3d_thickness);
    }
    if(MatchINI(buffer, "NEWDRAWFACE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &blockage_draw_option);
      updatefacelists = 1;
      blockage_draw_option = CLAMP(blockage_draw_option, 0, 3);
      continue;
    }
    if(MatchINI(buffer, "TLOAD") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f %i %i", &use_tload_begin, &scase->tload_begin, &use_tload_end, &scase->tload_end, &use_tload_skip, &tload_skip);
      continue;
    }
    if(MatchINI(buffer, "VOLSMOKE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i",
        &glui_compress_volsmoke, &use_multi_threading, &load_at_rendertimes, &volbw, &show_volsmoke_moving);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f %f %f %f",
        &global_temp_min, &global_temp_cutoff, &global_temp_max, &fire_opacity_factor, &mass_extinct, &gpu_vol_factor, &nongpu_vol_factor);
      global_temp_cutoff_default = global_temp_cutoff;
      ONEORZERO(glui_compress_volsmoke);
      ONEORZERO(use_multi_threading);
      ONEORZERO(load_at_rendertimes);
      fire_opacity_factor = CLAMP(fire_opacity_factor, 1.0, 10.0);
      mass_extinct = CLAMP(mass_extinct, 100.0, 100000.0);
      InitVolRenderSurface(NOT_FIRSTCALL);
      continue;
    }
    if(MatchINI(buffer, "WINDROSEMERGE")==1){
      float *xyzt;

      xyzt = windrose_merge_dxyzt;
      fgets(buffer, 255, stream);
      sscanf(buffer," %i %f %f %f %f",&windrose_merge_type,xyzt,xyzt+1,xyzt+2,xyzt+3);
      xyzt[0]=MAX(xyzt[0],0.0);
      xyzt[1]=MAX(xyzt[1],0.0);
      xyzt[2]=MAX(xyzt[2],0.0);
      xyzt[3]=MAX(xyzt[3],0.0);
    }
    if(MatchINI(buffer, "WINDROSEDEVICE")==1){
      float rad_windrose;

      fgets(buffer, 255, stream);
      sscanf(buffer," %i %i %i %i %i %i %i %i %i",
        &viswindrose, &showref_windrose, &windrose_xy_vis, &windrose_xz_vis, &windrose_yz_vis, &windstate_windrose, &showlabels_windrose,
        &windrose_first,&windrose_next);
      viswindrose         = CLAMP(viswindrose, 0, 1);
      showref_windrose    = CLAMP(showref_windrose, 0, 1);
      windrose_xy_vis      = CLAMP(windrose_xy_vis, 0, 1);
      windrose_xz_vis      = CLAMP(windrose_xz_vis, 0, 1);
      windrose_yz_vis      = CLAMP(windrose_yz_vis, 0, 1);
      windstate_windrose  = CLAMP(windstate_windrose, 0, 1);
      showlabels_windrose = CLAMP(showlabels_windrose, 0, 1);
      if(windrose_first < 0)windrose_first = 0;
      if(windrose_next < 1)windrose_next = 1;
      if(viswindrose==1)update_windrose = 1;

      fgets(buffer, 255, stream);
      sscanf(buffer," %i %i %i %f %i %i",    &nr_windrose, &ntheta_windrose, &scale_windrose, &rad_windrose, &scale_increment_windrose, &scale_max_windrose);
      nr_windrose              = ABS(nr_windrose);
      ntheta_windrose          = ABS(ntheta_windrose);
      if(localfile==1)radius_windrose          = ABS(rad_windrose);
      scale_windrose           = CLAMP(scale_windrose,0,1);
      scale_increment_windrose = CLAMP(scale_increment_windrose, 1, 50);
      scale_max_windrose       = CLAMP(scale_max_windrose, 0, 100);
      continue;
    }
    if(MatchINI(buffer, "BOUNDARYTWOSIDE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &showpatch_both);
      ONEORZERO(showpatch_both);
    }
    if(MatchINI(buffer, "MESHOFFSET") == 1){
      int meshnum;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &meshnum);
      if(meshnum >= 0 && meshnum<scase->meshescoll.nmeshes){
        meshdata *meshi;

        meshi = scase->meshescoll.meshinfo + meshnum;
        meshi->mesh_offset_ptr = meshi->mesh_offset;
      }
      continue;
    }
    if(MatchINI(buffer, "STARTUPLANG") == 1){
      char *bufptr;

      fgets(buffer, 255, stream);
      TrimBack(buffer);
      bufptr = TrimFront(buffer);
      strncpy(startup_lang_code, bufptr, 2);
      startup_lang_code[2] = '\0';
      if(tr_name == NULL){
        int langlen;

        langlen = strlen(bufptr);
        NewMemory((void **)&tr_name, langlen + 48 + 1);
        strcpy(tr_name, bufptr);
      }
      continue;
    }
    if(MatchINI(buffer, "MESHVIS") == 1){
      int nm;
      meshdata *meshi;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nm);
      for(i = 0; i<nm; i++){
        if(i>scase->meshescoll.nmeshes - 1)break;
        meshi = scase->meshescoll.meshinfo + i;
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &meshi->blockvis);
        ONEORZERO(meshi->blockvis);
      }
      continue;
    }
    if(MatchINI(buffer, "SPHERESEGS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &device_sphere_segments);
      device_sphere_segments = CLAMP(device_sphere_segments, 6, 48);
      InitSphere(device_sphere_segments, 2 * device_sphere_segments);
      InitCircle(2 * device_sphere_segments, &object_circ);
      continue;
    }
    if(MatchINI(buffer, "DIRECTIONCOLOR") == 1){
      float *dc;

      dc = direction_color;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", dc, dc + 1, dc + 2);
      dc[3] = 1.0;
      direction_color_ptr = GetColorPtr(direction_color);
      GetSliceParmInfo(scase, &sliceparminfo);
      UpdateSliceMenuShow(&sliceparminfo);
      continue;
    }

    if(MatchINI(buffer, "OFFSETSLICE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &offset_slice);
      ONEORZERO(offset_slice);
      continue;
    }
    if(MatchINI(buffer, "VECLENGTH") == 1){
      float vf = 1.0;
      int idummy;
      float dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %f %i %i %i", &idummy, &vf, &dummy, &vec_uniform_length, &vec_uniform_spacing, &color_vector_black);
      vecfactor = vf;
      continue;
    }
    if(MatchINI(buffer, "VECCONTOURS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &show_node_slices_and_vectors,&show_cell_slices_and_vectors);
      ONEORZERO(show_node_slices_and_vectors);
      ONEORZERO(show_cell_slices_and_vectors);
      continue;
    }
    if(MatchINI(buffer, "ISOTRAN2") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &transparent_state);
      continue;
    }
    if(MatchINI(buffer, "SHOWTRIANGLES") == 1){
      int dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i %i %i", &show_iso_shaded, &show_iso_outline, &show_iso_points, &show_iso_normal, &dummy, &smooth_iso_normal, &sort_iso_triangles);
      ONEORZERO(show_iso_shaded);
      ONEORZERO(show_iso_outline);
      ONEORZERO(show_iso_points);
      ONEORZERO(show_iso_normal);
      ONEORZERO(smooth_iso_normal);
      ONEORZERO(show_iso_normal);
      ONEORZERO(sort_iso_triangles);
      sort_geometry = sort_iso_triangles;
      visAIso = show_iso_shaded * 1 + show_iso_outline * 2 + show_iso_points * 4;
      continue;
    }
    if(MatchINI(buffer, "SHOWSTREAK") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i", &streak5show, &streak5step, &showstreakhead, &streak_index);
      ONEORZERO(streak5show);
      if(streak5show == 0)streak_index = -2;
      ONEORZERO(showstreakhead);
      update_streaks = 1;
      continue;
    }

    if(MatchINI(buffer, "SHOWTERRAIN") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &scase->visTerrainType, &terrain_slice_overlap);
      continue;
    }
    if(MatchINI(buffer, "STEREO") == 1){
      fgets(buffer, 255, stream);
      stereotypeOLD = stereotype;
      sscanf(buffer, "%i", &stereotype);
      stereotype = CLAMP(stereotype, 0, 5);
      if(stereotype == STEREO_TIME&&videoSTEREO != 1)stereotype = STEREO_NONE;
      GLUIUpdateStereo();
      continue;
    }
    if(MatchINI(buffer, "TERRAINPARMS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", terrain_rgba_zmin, terrain_rgba_zmin + 1, terrain_rgba_zmin + 2);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", terrain_rgba_zmax, terrain_rgba_zmax + 1, terrain_rgba_zmax + 2);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &vertical_factor);

      for(i = 0; i<3; i++){
        terrain_rgba_zmin[i] = CLAMP(terrain_rgba_zmin[i], 0, 2255);
        terrain_rgba_zmax[i] = CLAMP(terrain_rgba_zmax[i], 0, 2255);
      }
      vertical_factor = CLAMP(vertical_factor, 0.25, 4.0);
      UpdateTerrain(0);
      UpdateTerrainColors();
      continue;
    }
    if(MatchINI(buffer, "SMOKESENSORS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &show_smokesensors, &test_smokesensors);
      continue;
    }
#ifdef pp_GPU
    if(gpuactive == 1 && MatchINI(buffer, "USEGPU") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &usegpu);
      ONEORZERO(usegpu);
      continue;
    }
#endif
    if(MatchINI(buffer, "V2_PLOT3D") == 1||
       MatchINI(buffer, "V_PLOT3D")==1){
      int is_old_bound;

      is_old_bound=0;
      if(MatchINI(buffer, "V_PLOT3D")==1){
        is_old_bound = 1;
      }
      int tempval;
      int n3d;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &tempval);
      if(tempval<0)tempval = 0;
      n3d = tempval;
      if(n3d>MAXPLOT3DVARS)n3d = MAXPLOT3DVARS;
      for(i = 0; i<n3d; i++){
        int iplot3d, isetmin, isetmax;
        float p3mintemp, p3maxtemp;

        fgets(buffer, 255, stream);
        strcpy(buffer2, "");
        sscanf(buffer, "%i %i %f %i %f %s", &iplot3d, &isetmin, &p3mintemp, &isetmax, &p3maxtemp, buffer2);
        if(is_old_bound==1){
          isetmin = GetNewBoundIndex(isetmin);
          isetmax = GetNewBoundIndex(isetmax);
        }
        if(isetmin == BOUND_SET_MIN || isetmax == BOUND_SET_MAX){
          research_mode = 0;
          update_research_mode = 1;
        }
        iplot3d--;
        if(iplot3d >= 0 && iplot3d<MAXPLOT3DVARS){
          setp3min_all[iplot3d] = isetmin;
          setp3max_all[iplot3d] = isetmax;
          p3min_all[iplot3d]    = p3mintemp;
          p3max_all[iplot3d]    = p3maxtemp;
          if(scase->plot3dinfo!=NULL){
            GLUISetMinMax(BOUND_PLOT3D, scase->plot3dinfo[0].label[iplot3d].shortlabel, isetmin, p3mintemp, isetmax, p3maxtemp);
            update_glui_bounds = 1;
          }
        }
      }
      continue;
    }
    if(MatchINI(buffer, "UNLOAD_QDATA") == 1){
      int unload_qdata;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &unload_qdata);
      cache_plot3d_data = 1 - unload_qdata;
      ONEORZERO(cache_plot3d_data);
      update_cache_data = 1;
      continue;
    }
    if(MatchINI(buffer, "CACHE_DATA") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i", &cache_boundary_data, &cache_part_data, &cache_plot3d_data, &cache_slice_data);
      ONEORZERO(cache_boundary_data);
      ONEORZERO(cache_part_data);
      ONEORZERO(cache_plot3d_data);
      ONEORZERO(cache_slice_data);
      update_cache_data = 1;
      continue;
    }
    if(MatchINI(buffer, "TREECOLORS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", trunccolor, trunccolor + 1, trunccolor + 2);
      sscanf(buffer, "%f %f %f", treecolor, treecolor + 1, treecolor + 2);
      sscanf(buffer, "%f %f %f", treecharcolor, treecharcolor + 1, treecharcolor + 2);
      for(i = 0; i<3; i++){
        treecolor[i] = CLAMP(treecolor[i], 0.0, 1.0);
        treecolor_uc[i] = 255 * treecolor[i];
        treecharcolor[i] = CLAMP(treecharcolor[i], 0.0, 1.0);
        treecharcolor_uc[i] = 255 * treecharcolor[i];
        trunccolor[i] = CLAMP(trunccolor[i], 0.0, 1.0);
        trunccolor_uc[i] = 255 * trunccolor[i];
      }
      treecolor[3] = 1.0;
      treecharcolor[3] = 1.0;
      trunccolor[3] = 1.0;
      treecolor_uc[3] = 255;
      treecharcolor_uc[3] = 255;
      trunccolor_uc[3] = 255;
      continue;
    }
    if(MatchINI(buffer, "TRAINERVIEW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &trainerview);
      if(trainerview != 2 && trainerview != 3)trainerview = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWTRANSPARENTVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_transparent_vents);
      ONEORZERO(show_transparent_vents);
      continue;
    }
    if(MatchINI(buffer, "COLORBARTYPE") == 1){
      char *label;

      fgets(buffer, 255, stream);
      label = strchr(buffer, '%');
      if(label != NULL){
        update_colorbartype = 1;
        label++;
        TrimBack(label);
        label = TrimFront(label);
        strcpy(colorbarname, label);
      }
      continue;
    }
    if(MatchINI(buffer, "COLORMAP") == 1){
      char *ctype, *cmaptype, *cmap;
      colorbardata *cb;

      fgets(buffer, 255, stream);
      ctype = strtok(buffer, " ");
      cmaptype = strtok(NULL, " ");
      cmap = strtok(NULL, " ");
      if(strcmp(ctype, "FIRE") == 0){
        sscanf(cmaptype, "%i", &fire_colormap_type);
        cb = GetColorbar(&colorbars, cmap);
        if(cb == NULL)continue;
        fire_colormap_type_save = fire_colormap_type;
        fire_colorbar_index_ini = cb - colorbars.colorbarinfo;
        update_fire_colorbar_index = 1;
      }
      else if(strcmp(ctype, "CO2") == 0){
        sscanf(cmaptype, "%i", &co2_colormap_type);
        cb = GetColorbar(&colorbars, cmap);
        if(cb == NULL)continue;
        co2_colorbar_index_ini = cb - colorbars.colorbarinfo;
        update_co2_colorbar_index = 1;
      }
      else{
        continue;
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWEXTREMEDATA") == 1){
      int below = -1, above = -1, dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &dummy, &below, &above);
      if(below != 1)below = 0;
      if(above != 1)above = 0;
      show_extreme_mindata = below;
      show_extreme_maxdata = above;
      continue;
    }
    if(MatchINI(buffer, "EXTREMECOLORS") == 1){
      int mmin[3], mmax[3];

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i %i",
        mmin, mmin + 1, mmin + 2,
        mmax, mmax + 1, mmax + 2);
      for(i = 0; i<3; i++){
        rgb_below_min[i] = CLAMP(mmin[i], 0, 255);
        rgb_above_max[i] = CLAMP(mmax[i], 0, 255);
      }
      continue;
    }
    if(MatchINI(buffer, "SLICEAVERAGE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &slice_average_flag, &slice_average_interval, &vis_slice_average);
      ONEORZERO(slice_average_flag);
      if(slice_average_interval<0.0)slice_average_interval = 0.0;
      continue;
    }
    if(MatchINI(buffer, "SKYBOX") == 1){
      skyboxdata *skyi;

      FreeSkybox();
      nskyboxinfo = 1;
      NewMemory((void **)&skyboxinfo, nskyboxinfo*sizeof(skyboxdata));
      skyi = skyboxinfo;

      for(i = 0; i<6; i++){
        char *skybox_texture;

        fgets(buffer, 255, stream);
        skybox_texture = TrimFrontBack(buffer);
        LoadSkyTexture(skybox_texture, skyi->face + i);
      }
    }
    if(MatchINI(buffer, "C_PLOT3D")==1){
      float valmin, valmax;
      int setvalmin, setvalmax;
      int tempval, j;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &tempval);
      tempval = MIN(tempval, nplot3dbounds_cpp);

      for(j=0;j<tempval;j++){
        int iplot3d;

        cpp_boundsdata *boundi;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %f %i %f", &iplot3d, &setvalmin, &valmin, &setvalmax, &valmax);
        iplot3d--;
        if(iplot3d>=0 && iplot3d<tempval){
          boundi = plot3dbounds_cpp+iplot3d;
          boundi->set_chopmin = setvalmin;
          boundi->chopmin     = valmin;
          boundi->set_chopmax = setvalmax;
          boundi->chopmax     = valmax;
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "DEVICENORMLENGTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &devicenorm_length);
      if(devicenorm_length<0.0 || devicenorm_length>1.0)devicenorm_length = 0.1;
      continue;
    }
    if(MatchINI(buffer, "SHOWHRRLABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &vis_hrr_label);
      ONEORZERO(vis_hrr_label);
      continue;
    }
    if(MatchINI(buffer, "SHOWHRRCUTOFF") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_firecutoff);
      ONEORZERO(show_firecutoff);
      continue;
    }
    if(MatchINI(buffer, "SHOWFIRECUTOFF") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_firecutoff);
      ONEORZERO(show_firecutoff);
      continue;
    }
    if(MatchINI(buffer, "TWOSIDEDVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &show_bothsides_int, &show_bothsides_ext);
      ONEORZERO(show_bothsides_int);
      ONEORZERO(show_bothsides_ext);
      continue;
    }
    if(MatchINI(buffer, "SHOWSLICEINOBST") == 1){
      if((localfile==0&&scase->solid_ht3d==0)||localfile==1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &scase->show_slice_in_obst);
        scase->show_slice_in_obst=CLAMP(scase->show_slice_in_obst,0,2);
      }
      continue;
    }
    if(MatchINI(buffer, "SKIPEMBEDSLICE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &skip_slice_in_embedded_mesh);
      ONEORZERO(skip_slice_in_embedded_mesh);
      continue;
    }
    if(MatchINI(buffer, "TRAINERMODE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &trainer_mode);
      continue;
    }
    if(MatchINI(buffer, "COMPRESSAUTO") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &compress_autoloaded);
      continue;
    }
    if(MatchINI(buffer, "PLOT3DAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &n3dsmokes);
        for(i = 0; i<n3dsmokes; i++){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &seq_id);
          GetStartupPlot3D(seq_id);
        }
        update_load_files = 1;
        continue;
      }
      if(MatchINI(buffer, "VSLICEAUTO") == 1){
        int n3dsmokes = 0;
        int seq_id;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &n3dsmokes);
        for(i = 0; i<n3dsmokes; i++){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &seq_id);
          GetStartupVSlice(seq_id);
        }
        update_load_files = 1;
        continue;
      }
      if(MatchINI(buffer, "SLICEAUTO") == 1){
        int n3dsmokes = 0;
        int seq_id;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &n3dsmokes);
        for(i = 0; i<n3dsmokes; i++){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &seq_id);
          GetStartupSlice(seq_id);
        }
        update_load_files = 1;
        continue;
      }
    if(MatchINI(buffer, "MSLICEAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0; i<n3dsmokes; i++){

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);

        if(seq_id >= 0 && seq_id<scase->slicecoll.nmultisliceinfo){
          multislicedata *mslicei;

          mslicei = scase->slicecoll.multisliceinfo + seq_id;
          mslicei->autoload = 1;
          mslicei->loadable = 1;
        }
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "PARTAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0; i<n3dsmokes; i++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);
        GetStartupPart(seq_id);
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "ISOAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0; i<n3dsmokes; i++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);
        GetStartupISO(seq_id);
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "S3DAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0; i<n3dsmokes; i++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);
        GetStartupSmoke(seq_id);
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "PATCHAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0; i<n3dsmokes; i++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);
        GetStartupBoundary(seq_id);
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "LOADFILESATSTARTUP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &loadfiles_at_startup);
      continue;
    }
    if(MatchINI(buffer, "SHOWALLTEXTURES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &showall_textures);
      continue;
    }
    if(MatchINI(buffer, "ENABLETEXTURELIGHTING") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &enable_texture_lighting);
      continue;
    }
    if(MatchINI(buffer, "PROJECTION") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &projection_type);
      projection_type = CLAMP(projection_type, 0, 1);
      GLUISceneMotionCB(PROJECTION);
      GLUIUpdateProjectionType();
      continue;
    }
    if(MatchINI(buffer, "V_PARTICLES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f", &setpartmin, &glui_partmin, &setpartmax, &glui_partmax);
      if(setpartmin==PERCENTILE_MIN){
        setpartmin = GLOBAL_MIN;
      }
      if(setpartmax==PERCENTILE_MAX){
        setpartmax = GLOBAL_MAX;
      }
      continue;
    }
    if(MatchINI(buffer, "V2_PARTICLES") == 1||
       MatchINI(buffer, "V_PARTICLES")==1||
       MatchINI(buffer, "V5_PARTICLES")==1
    ){
      int is_old_bound = 0;

      if(MatchINI(buffer, "V_PARTICLES")==1){
        is_old_bound = 1;
      }
      int ivmin, ivmax;
      float vmin, vmax;
      char short_label[256], *s1;

      strcpy(short_label, "");
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f %s", &ivmin, &vmin, &ivmax, &vmax, short_label);
      if(is_old_bound==1){
        ivmin = GetNewBoundIndex(ivmin);
        ivmax = GetNewBoundIndex(ivmax);
      }
      if(ivmin==BOUND_SET_MIN||ivmax==BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }

#define MAXVAL 100000000.0
#define MINVAL -100000000.0

      if(vmax > MAXVAL)vmax = 1.0;
      if(vmin < MINVAL)vmin = 0.0;

      if(ivmin == PERCENTILE_MIN && vmin > vmax)continue;
      if(ivmax == PERCENTILE_MAX && vmin > vmax)continue;

      if(npart5prop>0){
        int label_index = 0;

        TrimBack(short_label);
        s1 = TrimFront(short_label);
        if(strlen(s1)>0)label_index = GetPartPropIndexS(s1);
        if(label_index >= 0 && label_index<npart5prop){
          partpropdata *propi;

          propi = part5propinfo + label_index;
          propi->setvalmin = ivmin;
          propi->setvalmax = ivmax;
          propi->valmin = vmin;
          propi->valmax = vmax;
          if(is_old_bound==1){
            switch(ivmin){
            case PERCENTILE_MIN:
              propi->user_min = vmin;
              break;
              case GLOBAL_MIN:
                propi->dlg_global_valmin = vmin;
                break;
              case SET_MIN:
                propi->user_min = vmin;
                break;
              default:
                assert(FFALSE);
                break;
            }
            switch(ivmax){
            case PERCENTILE_MAX:
              propi->user_max = vmax;
              break;
              case GLOBAL_MAX:
                propi->dlg_global_valmax = vmax;
                break;
              case SET_MAX:
                propi->user_max = vmax;
                break;
              default:
                assert(FFALSE);
                break;
            }
          }
          else{
            switch(ivmin){
            case BOUND_PERCENTILE_MIN:
              propi->user_min = vmin;
              break;
              case BOUND_LOADED_MIN:
              case BOUND_GLOBAL_MIN:
                propi->dlg_global_valmin = vmin;
                break;
              case BOUND_SET_MIN:
                propi->user_min = vmin;
                break;
              default:
                assert(FFALSE);
                break;
            }
            switch(ivmax){
            case BOUND_PERCENTILE_MAX:
              propi->user_max = vmax;
              break;
              case BOUND_LOADED_MAX:
              case BOUND_GLOBAL_MAX:
                propi->dlg_global_valmax = vmax;
                break;
              case BOUND_SET_MAX:
                propi->user_max = vmax;
                break;
              default:
                assert(FFALSE);
                break;
            }
          }
#define MAX_PART_TYPES 100
          if(strcmp(short_label, "")==0){
            int npart_types;

            npart_types = GLUIGetNValtypes(BOUND_PART);
            if(npart_types>0){
              int  *ivmins, *ivmaxs;
              float *vmins, *vmaxs;
              int ivalmins[MAX_PART_TYPES],  ivalmaxs[MAX_PART_TYPES];
              float valmins[MAX_PART_TYPES], valmaxs[MAX_PART_TYPES];

              if(npart_types>MAX_PART_TYPES){
                NewMemory((void **)&ivmins, npart_types*sizeof(int));
                NewMemory((void **)&vmins,  npart_types*sizeof(float));
                NewMemory((void **)&ivmaxs, npart_types*sizeof(int));
                NewMemory((void **)&vmaxs,  npart_types*sizeof(float));
              }
              else{
                ivmins = ivalmins;
                ivmaxs = ivalmaxs;
                vmins = valmins;
                vmaxs = valmaxs;
              }
              for(i = 0; i<npart_types; i++){
                ivmins[i] = ivmin;
                ivmaxs[i] = ivmax;
                vmins[i]  = vmin;
                vmaxs[i]  = vmax;
              }
              GLUISetMinMaxAll(BOUND_PART, ivmins, vmins, ivmaxs, vmaxs, npart_types);
              if(npart_types>MAX_PART_TYPES){
                FREEMEMORY(ivmins);
                FREEMEMORY(vmins);
                FREEMEMORY(ivmaxs);
                FREEMEMORY(vmaxs);
              }
            }
          }
          else{
            GLUISetMinMax(BOUND_PART, short_label, ivmin, vmin, ivmax, vmax);
          }
          update_glui_bounds=1;
        }
      }
      continue;
    }
    if(MatchINI(buffer, "C_PARTICLES")==1){
      float valmin, valmax;
      int setvalmin, setvalmax;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      if(strcmp(buffer2, "")!=0&&strcmp(buffer2,"Uniform")!=0){
#ifdef pp_PARTBOUND_MULTI
        THREADcontrol(partbound_threads, THREAD_JOIN);
#endif
        for(i = 0; i<npartbounds_cpp; i++){
          cpp_boundsdata *boundi;

          boundi = partbounds_cpp+i;
          if(strcmp(buffer2,boundi->label)==0){
            boundi->set_chopmin = setvalmin;
            boundi->chopmin     = valmin;
            boundi->set_chopmax = setvalmax;
            boundi->chopmax     = valmax;
            break;
          }
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "V2_SLICE")==1||
       MatchINI(buffer, "V_SLICE")==1){
      int is_old_bound;
      char *colon;

      is_old_bound=0;
      if(MatchINI(buffer, "V_SLICE")==1){
        is_old_bound = 1;
      }
      float valmin, valmax;
      int set_valmin, set_valmax;
      char *level_val;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &set_valmin, &valmin, &set_valmax, &valmax, buffer2);

      if(is_old_bound==1){
        set_valmin = GetNewBoundIndex(set_valmin);
        set_valmax = GetNewBoundIndex(set_valmax);
      }
      if(set_valmin==BOUND_SET_MIN||set_valmax==BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }
      colon = strstr(buffer, ":");
      level_val = NULL;
      if(colon != NULL){
        level_val = colon + 1;
        TrimBack(level_val);
        *colon = 0;
        if(strlen(level_val)>1){
          sscanf(level_val, "%f %f %i", &slice_line_contour_min, &slice_line_contour_max, &slice_line_contour_num);
        }
        level_val = NULL;
      }
      TrimBack(buffer2);
      SetSliceBounds(set_valmin, valmin, set_valmax, valmax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "C_SLICE_HIDE")==1){
      int chop_hide;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i  %s", &chop_hide, buffer2);
      if(strcmp(buffer, "")!=0){
        for(i = 0; i<nslicebounds_cpp; i++){
          cpp_boundsdata *boundi;

          boundi = slicebounds_cpp+i;
          if(strcmp(buffer2,boundi->label)==0){
            boundi->chop_hide = chop_hide;
            break;
          }
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "C_SLICE")==1){
      float valmin, valmax;
      int setvalmin, setvalmax;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      if(strcmp(buffer, "")!=0){
        for(i = 0; i<nslicebounds_cpp; i++){
          cpp_boundsdata *boundi;

          boundi = slicebounds_cpp+i;
          if(strcmp(buffer2,boundi->label)==0){
            boundi->set_chopmin = setvalmin;
            boundi->chopmin     = valmin;
            boundi->set_chopmax = setvalmax;
            boundi->chopmax     = valmax;
            break;
          }
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "V_ISO") == 1){
      float valmin, valmax;
      int setvalmin, setvalmax;
      char *isolabel;

#define SETVALMIN_ORIG 1
#define SETVALMAX_ORIG 1

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      isolabel = TrimFrontBack(buffer2);
      if(strcmp(isolabel, "") != 0){
        for(i = 0; i<niso_bounds; i++){
          if(strcmp(isolabel, isobounds[i].label->shortlabel) != 0)continue;
          isobounds[i].ini_defined = 1;
          isobounds[i].ini_setvalmin = setvalmin;
          isobounds[i].ini_setvalmax = setvalmax;
          if(setvalmin == SETVALMIN_ORIG)isobounds[i].ini_valmin = valmin;
          if(setvalmax == SETVALMAX_ORIG)isobounds[i].ini_valmax = valmax;
          update_iso_ini = 1;
          break;
        }
      }
      continue;
    }
    if(MatchINI(buffer, "C_ISO") == 1){
      float valmin, valmax;
      int setvalmin, setvalmax;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      if(strcmp(buffer, "") != 0){
        for(i = 0; i<niso_bounds; i++){
          if(strcmp(isobounds[i].shortlabel, buffer2) != 0)continue;
          isobounds[i].setchopmin = setvalmin;
          isobounds[i].setchopmax = setvalmax;
          isobounds[i].chopmin = valmin;
          isobounds[i].chopmax = valmax;
          break;
        }
      }
      else{
        for(i = 0; i<niso_bounds; i++){
          isobounds[i].setchopmin = setvalmin;
          isobounds[i].setchopmax = setvalmax;
          isobounds[i].chopmin = valmin;
          isobounds[i].chopmax = valmax;
        }
      }
      continue;
    }
    if(MatchINI(buffer, "V2_BOUNDARY") == 1||
       MatchINI(buffer, "V_BOUNDARY")==1){
      int is_old_bound;

      is_old_bound=0;
      if(MatchINI(buffer, "V_BOUNDARY")==1){
        is_old_bound = 1;
      }
      fgets(buffer, 255, stream);
      TrimBack(buffer);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &glui_setpatchmin, &glui_patchmin, &glui_setpatchmax, &glui_patchmax, buffer2);
      if(is_old_bound==1){

        glui_setpatchmin = GetNewBoundIndex(glui_setpatchmin);
        glui_setpatchmax = GetNewBoundIndex(glui_setpatchmax);
      }
      if(glui_setpatchmin==BOUND_SET_MIN||glui_setpatchmax==BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }
      SetBoundBounds(glui_setpatchmin, glui_patchmin, glui_setpatchmax, glui_patchmax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "C_BOUNDARY")==1){
      float valmin, valmax;
      int setvalmin, setvalmax;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      if(strcmp(buffer, "")!=0){
        for(i = 0; i<npatchbounds_cpp; i++){
          cpp_boundsdata *boundi;

          boundi = patchbounds_cpp+i;
          if(strcmp(buffer2,boundi->label)==0){
            boundi->set_chopmin = setvalmin;
            boundi->chopmin     = valmin;
            boundi->set_chopmax = setvalmax;
            boundi->chopmax     = valmax;
            break;
          }
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "V2_HVACDUCT") == 1){
      int sethvacductmin, sethvacductmax;
      float hvacductmin, hvacductmax;

      fgets(buffer, 255, stream);
      TrimBack(buffer);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &sethvacductmin, &hvacductmin, &sethvacductmax, &hvacductmax, buffer2);
      if(sethvacductmin != BOUND_SET_MIN)sethvacductmin = BOUND_LOADED_MIN;
      if(sethvacductmax != BOUND_SET_MAX)sethvacductmax = BOUND_LOADED_MAX;
      if(sethvacductmin == BOUND_SET_MIN || sethvacductmax == BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }
      SetHVACDuctBounds(sethvacductmin, hvacductmin, sethvacductmax, hvacductmax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "V2_HVACNODE") == 1){
      int sethvacnodemin, sethvacnodemax;
      float hvacnodemin, hvacnodemax;

      fgets(buffer, 255, stream);
      TrimBack(buffer);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &sethvacnodemin, &hvacnodemin, &sethvacnodemax, &hvacnodemax, buffer2);
      if(sethvacnodemin != BOUND_SET_MIN)sethvacnodemin = BOUND_LOADED_MIN;
      if(sethvacnodemax != BOUND_SET_MAX)sethvacnodemax = BOUND_LOADED_MAX;
      if(sethvacnodemin == BOUND_SET_MIN || sethvacnodemax == BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }
      SetHVACNodeBounds(sethvacnodemin, hvacnodemin, sethvacnodemax, hvacnodemax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "V_ZONE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f", &setzonemin, &zoneusermin, &setzonemax, &zoneusermax);
      if(setzonemin == PERCENTILE_MIN)setzonemin = GLOBAL_MIN;
      if(setzonemax == PERCENTILE_MIN)setzonemax = GLOBAL_MIN;
      if(setzonemin == SET_MIN)zonemin = zoneusermin;
      if(setzonemax == SET_MAX)zonemax = zoneusermax;
      GLUIUpdateZoneBounds();
      continue;
    }
    if(MatchINI(buffer, "V_TARGET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f %s", &settargetmin, &targetmin, &settargetmax, &targetmax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "OUTLINECOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", glui_outlinecolor, glui_outlinecolor + 1, glui_outlinecolor + 2);
      glui_outlinecolor[0] = CLAMP(glui_outlinecolor[0], 0, 255);
      glui_outlinecolor[1] = CLAMP(glui_outlinecolor[1], 0, 255);
      glui_outlinecolor[2] = CLAMP(glui_outlinecolor[2], 0, 255);
      continue;
    }
    if(MatchINI(buffer, "ZONEVIEW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &zone_hvac_diam);
      if(zone_hvac_diam < 0.0)zone_hvac_diam = 0.0;
      continue;
    }
    if(MatchINI(buffer, "OUTLINEMODE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &highlight_flag, &outline_color_flag);
      if(scase->meshescoll.nmeshes<2){
        ONEORZERO(highlight_flag);
      }
      continue;
    }
    if(MatchINI(buffer, "SLICEDATAOUT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &output_slicedata);
      ONEORZERO(output_slicedata);
      continue;
    }
    if(MatchINI(buffer, "SMOKE3DZIPSTEP") == 1 ||
       MatchINI(buffer, "SLICEZIPSTEP")   == 1 ||
       MatchINI(buffer, "ISOZIPSTEP")     == 1 ||
       MatchINI(buffer, "BOUNDZIPSTEP")   == 1 ||
       MatchINI(buffer, "ZIPSTEP")        == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &tload_zipstep);
      tload_zipstep = MAX(tload_zipstep, 1);
      tload_zipskip = tload_zipstep - 1;
      continue;
    }
    if(MatchINI(buffer, "LOADINC") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &load_incremental);
      continue;
    }
    if(MatchINI(buffer, "MSCALE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", mscale, mscale + 1, mscale + 2);
      continue;
    }
    if(MatchINI(buffer, "RENDERCLIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i",
        &clip_rendered_scene, &render_clip_left, &render_clip_right, &render_clip_bottom, &render_clip_top);
      continue;
    }
    if(MatchINI(buffer, "CLIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", &nearclip, &farclip);
      continue;
    }
    if(MatchINI(buffer, "SHOWTRACERSALWAYS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_tracers_always);
      ONEORZERO(show_tracers_always);
      continue;
    }
    if(MatchINI(buffer, "SHOWPARTTAG") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &select_part);
      ONEORZERO(select_part);
      continue;
    }
    if(MatchINI(buffer, "PARTSKIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &partdrawskip);
      partdrawskip = MAX(partdrawskip, 1);
    }
    if(MatchINI(buffer, "PART5COLOR") == 1){
      for(i = 0; i<npart5prop; i++){
        partpropdata *propi;

        propi = part5propinfo + i;
        propi->display = 0;
      }
      part5colorindex = 0;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &i);
      if(i >= 0 && i<npart5prop){
        partpropdata *propi;

        part5colorindex = i;
        propi = part5propinfo + i;
        propi->display = 1;
      }
      continue;
    }
    if(MatchINI(buffer, "PART5PROPDISP") == 1){
      char *token;

      for(i = 0; i<npart5prop; i++){
        partpropdata *propi;
        int j;

        propi = part5propinfo + i;
        fgets(buffer, 255, stream);

        TrimBack(buffer);
        token = strtok(buffer, " ");
        j = 0;
        while(token != NULL&&j<scase->npartclassinfo){
          int visval;

          sscanf(token, "%i", &visval);
          propi->class_vis[j] = visval;
          token = strtok(NULL, " ");
          j++;
        }
      }
      CheckMemory;
      continue;
    }
    if(MatchINI(buffer, "COLORBAR") == 1){
      float *rgb_ini_copy;
      int nn;
      int dummy;

      CheckMemory;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i", &nrgb_ini, &dummy, &colorbar_select_index, &colorbar_selection_width);
      if(nrgb_ini!=12){
        fprintf(stderr, "***warning: COLORBAR ini parameter skipped - only colorbars with 12 entries are supported.\n");
        continue;
      }
      colorbar_selection_width = CLAMP(colorbar_selection_width, COLORBAR_SELECTION_WIDTH_MIN, COLORBAR_SELECTION_WIDTH_MAX);
      FREEMEMORY(rgb_ini);
      if(NewMemory((void **)&rgb_ini, 4 * nrgb_ini*sizeof(float)) == 0)return 2;
      rgb_ini_copy = rgb_ini;
      for(nn = 0; nn<nrgb_ini; nn++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f %f ", rgb_ini_copy, rgb_ini_copy + 1, rgb_ini_copy + 2);
        rgb_ini_copy += 3;
      }
      InitRGB();
      if(colorbar_select_index >= 0 && colorbar_select_index <= 255){
        update_colorbar_select_index = 1;
      }
      continue;
    }
    if(MatchINI(buffer, "COLOR2BAR") == 1){
      float *rgb_ini_copy;
      int nn;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &nrgb2_ini);
      if(nrgb2_ini<8){
        fprintf(stderr, "*** Error: must have at lease 8 colors in COLOR2BAR\n");
        SMV_EXIT(1);
      }
      FREEMEMORY(rgb2_ini);
      if(NewMemory((void **)&rgb2_ini, 4 * nrgb_ini*sizeof(float)) == 0)return 2;
      rgb_ini_copy = rgb2_ini;
      for(nn = 0; nn<nrgb2_ini; nn++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f %f ", rgb_ini_copy, rgb_ini_copy + 1, rgb_ini_copy + 2);
        rgb_ini_copy += 3;
      }
      continue;
    }
    if(MatchINI(buffer, "PLOT2DHRRBOUNDS") == 1){
      int dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f", &dummy, &plot2d_hrr_min, &dummy, &plot2d_hrr_max);
      continue;
    }
    if(MatchINI(buffer, "P3DSURFACETYPE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &p3dsurfacetype);
      continue;
    }
    if(MatchINI(buffer, "P3DSURFACESMOOTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &p3dsurfacesmooth);
      continue;
    }
    if(MatchINI(buffer, "CULLFACES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &cullfaces);
      continue;
    }
    if(MatchINI(buffer, "PARTPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &partpointsize);
      continue;
    }
    if(MatchINI(buffer, "ISOPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &isopointsize);
      continue;
    }
    if(MatchINI(buffer, "ISOLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &isolinewidth);
      continue;
    }
    if(MatchINI(buffer, "PLOT3DPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &plot3dpointsize);
      continue;
    }
    if(MatchINI(buffer, "GRIDLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &gridlinewidth);
      continue;
    }
    if(MatchINI(buffer, "TICKLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &ticklinewidth);
      continue;
    }
    if(MatchINI(buffer, "PLOT3DLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &plot3dlinewidth);
      continue;
    }
    if(MatchINI(buffer, "VECTORCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &color_vector_black);
      continue;
    }
    if(MatchINI(buffer, "VECTORPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &vectorpointsize);
      continue;
    }
    if(MatchINI(buffer, "VECTORLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", &vectorlinewidth, &slice_line_contour_width);
      continue;
    }
    if(MatchINI(buffer, "STREAKLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &streaklinewidth);
      continue;
    }
    if(MatchINI(buffer, "LINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &scase->linewidth);
      solidlinewidth = scase->linewidth;
      continue;
    }
    if(MatchINI(buffer, "VENTLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &scase->ventlinewidth);
      continue;
    }
    if(MatchINI(buffer, "BOUNDARYOFFSET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &boundaryoffset);
      continue;
    }
    if(MatchINI(buffer, "SLICEOFFSET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %i", &sliceoffset_factor, &slice_dz, &agl_offset_actual);
      continue;
    }
    if(MatchINI(buffer, "TITLESAFE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &titlesafe_offset);
      continue;
    }
    if(MatchINI(buffer, "VENTOFFSET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &ventoffset_factor);
      continue;
    }
    if(MatchINI(buffer, "SHOWBLOCKS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visBlocks);
      continue;
    }
    if(MatchINI(buffer, "SHOWSENSORS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i ", &visSensor, &visSensorNorm);
      ONEORZERO(visSensor);
      ONEORZERO(visSensorNorm);
      continue;
    }
    if(MatchINI(buffer, "SHOWVENTFLOW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i", &visVentHFlow, &visventslab, &visventprofile, &visVentVFlow, &visVentMFlow);
      continue;
    }
    if(MatchINI(buffer, "SHOWVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visVents);
      continue;
    }
    if(MatchINI(buffer, "SHOWROOMS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visCompartments);
      continue;
    }
    if(MatchINI(buffer, "SHOWTIMELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visTimelabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWHMSTIMELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &vishmsTimelabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWFRAMETIMELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visFrameTimelabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWFRAMELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visFramelabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWHRRLABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &vis_hrr_label);
      continue;
    }
    if(MatchINI(buffer, "RENDERFILETYPE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &render_filetype, &movie_filetype, &render_resolution);
      RenderCB(RENDER_RESOLUTION);
      continue;
    }
    if(MatchINI(buffer, "MOVIEFILETYPE") == 1){
      int quicktime_dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i", &movie_filetype,&movie_framerate,&movie_bitrate,&quicktime_dummy,&movie_crf);
      continue;
    }
    if(MatchINI(buffer, "MOVIEPARMS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &movie_queue_index, &movie_nprocs, &movie_slice_index);
      movie_queue_index = CLAMP(movie_queue_index, 0, nmovie_queues-1);
      movie_slice_index = CLAMP(movie_slice_index, 0, nslicemenuinfo-1);
      update_movie_parms = 1;
      continue;
    }
    if(MatchINI(buffer, "RENDERFILELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &render_label_type);
      ONEORZERO(render_label_type);
      continue;
    }
    if(MatchINI(buffer, "CELLCENTERTEXT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &show_slice_values_all_regions);
      continue;
    }
    if(MatchINI(buffer, "SHOWSLICEVALS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &show_slice_values_all_regions);
      continue;
    }
    if(MatchINI(buffer, "SHOWGRIDLOC") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visgridloc);
      continue;
    }
    if(MatchINI(buffer, "SHOWGRID") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visGrid);
      continue;
    }
    if(MatchINI(buffer, "SHOWFLOOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visFloor);
      continue;
    }
    if(MatchINI(buffer, "SPEED") == 1){
      fgets(buffer, 255, stream);
      //  sscanf(buffer,"%f %f",&speed_crawl,&speed_walk);
      continue;
    }
    if(MatchINI(buffer, "FONTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &fontindex);
      fontindex = CLAMP(fontindex, 0, SCALED_FONT);
      FontMenu(fontindex);
      continue;
    }
    if(MatchINI(buffer, "ZOOM") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f ", &zoomindex, &zoom);
      if(zoomindex<0)zoomindex = ZOOMINDEX_ONE;
      if(zoomindex>MAX_ZOOMS+1)zoomindex = ZOOMINDEX_ONE;
      zooms[zoomindex] = zoom;
      zoomini = zoom;
      updatezoomini = 1;
      ZoomMenu(zoomindex);
      continue;
    }
    if(MatchINI(buffer, "APERATURE") == 1 ||
       MatchINI(buffer, "APERTURE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &apertureindex);
      apertureindex = CLAMP(apertureindex, 0, 4);
      ApertureMenu(apertureindex);
      continue;
    }
    if(MatchINI(buffer, "SHOWTARGETS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &vis_target_data);
      continue;
    }
    if(MatchINI(buffer, "SHOWWALLS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visWalls,&vis_wall_data);
      continue;
    }
    if(MatchINI(buffer, "SHOWCEILING") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visCeiling);
      continue;
    }
    if(MatchINI(buffer, "SHOWTITLE") == 1){
      int dummy_val;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &vis_title_smv_version,&dummy_val,&vis_title_fds);
      continue;
    }
    if(MatchINI(buffer, "SHOWCHID") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &vis_title_CHID);
      continue;
    }
    if(MatchINI(buffer, "SHOWTRANSPARENT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visTransparentBlockage);
      continue;
    }
    if(MatchINI(buffer, "SHOWCADOPAQUE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &viscadopaque);
      continue;
    }
    if(MatchINI(buffer, "VECTORPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &vectorpointsize);
      continue;
    }
    if(MatchINI(buffer, "VECTORSKIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &vectorskip);
      if(vectorskip<1)vectorskip = 1;
      update_vectorskip = 1;
      continue;
    }
    if(MatchINI(buffer, "SPRINKLERABSSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &sprinklerabssize);
      continue;
    }
    if(MatchINI(buffer, "SENSORABSSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &sensorabssize);
      continue;
    }
    if(MatchINI(buffer, "SENSORRELSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &sensorrelsize);
      if(sensorrelsize<sensorrelsizeMIN)sensorrelsize = sensorrelsizeMIN;
      continue;
    }
    if(MatchINI(buffer, "SETBW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &setbw, &setbwdata);
      continue;
    }
    if(MatchINI(buffer, "FLIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &background_flip);
      continue;
    }
    if(MatchINI(buffer, "COLORBAR_FLIP") == 1 ||
       MatchINI(buffer, "COLORBARFLIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &colorbar_flip,&colorbar_autoflip);
      continue;
    }
    if(MatchINI(buffer, "TRANSPARENT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f", &use_transparency_data, &transparent_level);
      continue;
    }
    if(MatchINI(buffer, "SORTSLICES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &sortslices);
      ONEORZERO(sortslices);
      continue;
    }
    if(MatchINI(buffer, "VENTCOLOR") == 1){
      float ventcolor_temp[4];

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", ventcolor_temp, ventcolor_temp + 1, ventcolor_temp + 2);
      ventcolor_temp[3] = 1.0;
      ventcolor = GetColorPtr(ventcolor_temp);
      scase->updatefaces = 1;
      scase->updateindexcolors = 1;
      continue;
    }
    if(MatchINI(buffer, "STATICPARTCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", static_color, static_color + 1, static_color + 2);
      continue;
    }
    if(MatchINI(buffer, "HEATOFFCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", heatoffcolor, heatoffcolor + 1, heatoffcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "HEATONCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", heatoncolor, heatoncolor + 1, heatoncolor + 2);
      continue;
    }
    if(MatchINI(buffer, "SENSORCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", sensorcolor, sensorcolor + 1, sensorcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "SENSORNORMCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", sensornormcolor, sensornormcolor + 1, sensornormcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "SPRINKONCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", sprinkoncolor, sprinkoncolor + 1, sprinkoncolor + 2);
      continue;
    }
    if(MatchINI(buffer, "SPRINKOFFCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", sprinkoffcolor, sprinkoffcolor + 1, sprinkoffcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "BACKGROUNDCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", backgroundbasecolor, backgroundbasecolor + 1, backgroundbasecolor + 2);
      GLUISetColorControls();
      continue;
    }
    if(MatchINI(buffer, "SURFCOLORS")==1){
      int ncolors;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &ncolors, &use_surf_color);
      for(i = 0; i<ncolors; i++){
        surfdata *surfi;
        int *ini_surf_color;
        char *surflabel;

        fgets(buffer, 255, stream);
        surflabel = strchr(buffer, ':');
        if(surflabel==NULL)continue;
        surflabel = TrimFrontBack(surflabel+1);
        surfi = GetSurface(scase, surflabel);
        if(surfi==NULL)continue;
        ini_surf_color = surfi->geom_surf_color;
        sscanf(buffer, "%i %i %i", ini_surf_color, ini_surf_color+1, ini_surf_color+2);
        ini_surf_color[0] = CLAMP(ini_surf_color[0], 0, 255);
        ini_surf_color[1] = CLAMP(ini_surf_color[1], 0, 255);
        ini_surf_color[2] = CLAMP(ini_surf_color[2], 0, 255);
      }
      continue;
    }
    if(MatchINI(buffer, "OBSTSURFCOLORS")==1){
      int ncolors;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &ncolors);
      for(i = 0; i<ncolors; i++){
        surfdata *surfi;
        char *surflabel;
        float s_color[4];

        fgets(buffer, 255, stream);
        surflabel = strchr(buffer, ':');
        if(surflabel==NULL)continue;
        surflabel = TrimFrontBack(surflabel+1);
        surfi = GetSurface(scase, surflabel);
        if(surfi==NULL)continue;
        s_color[0] = -1.0;
        s_color[1] = -1.0;
        s_color[2] = -1.0;
        s_color[3] = -1.0;
        sscanf(buffer, "%f %f %f %f", s_color, s_color+1, s_color+2, s_color+3);
        if(s_color[3]<0.0)s_color[3] = 1.0;
        if(s_color[0]<0.0||s_color[1]<0.0||s_color[2]<0.0)continue;
        s_color[0] = CLAMP(s_color[0], 0.0, 1.0);
        s_color[1] = CLAMP(s_color[1], 0.0, 1.0);
        s_color[2] = CLAMP(s_color[2], 0.0, 1.0);
        surfi->color = GetColorPtr(s_color);
        surfi->transparent_level=s_color[3];
      }
      continue;
    }
    if(MatchINI(buffer, "GEOMSELECTCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%u %u %u",  geom_vertex1_rgb,  geom_vertex1_rgb+1,  geom_vertex1_rgb+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%u %u %u",  geom_vertex2_rgb,  geom_vertex2_rgb+1,  geom_vertex2_rgb+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%u %u %u", geom_triangle_rgb, geom_triangle_rgb+1, geom_triangle_rgb+2);
      for(i = 0; i<3; i++){
        geom_vertex1_rgb[i]  = CLAMP(geom_vertex1_rgb[i], 0, 255);
        geom_vertex2_rgb[i]  = CLAMP(geom_vertex2_rgb[i], 0, 255);
        geom_triangle_rgb[i] = CLAMP(geom_triangle_rgb[i], 0, 255);
      }
      continue;
    }
    if(MatchINI(buffer, "GEOMAXIS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", &glui_surf_axis_length, &glui_surf_axis_width);
      continue;
    }
    if(MatchINI(buffer, "FOREGROUNDCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", foregroundbasecolor, foregroundbasecolor + 1, foregroundbasecolor + 2);
      GLUISetColorControls();
      continue;
    }
    if(MatchINI(buffer, "BLOCKCOLOR") == 1){
      float blockcolor_temp[4];

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", blockcolor_temp, blockcolor_temp + 1, blockcolor_temp + 2);
      blockcolor_temp[3] = 1.0;
      block_ambient2 = GetColorPtr(blockcolor_temp);
      scase->updatefaces = 1;
      scase->updateindexcolors = 1;
      continue;
    }
    if(MatchINI(buffer, "BLOCKLOCATION") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &blocklocation);
      continue;
    }
    if(MatchINI(buffer, "BEAM") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %i %i %i",
          &showbeam_as_line,&beam_line_width,&use_beamcolor,beam_color,beam_color+1,beam_color+2);
      showbeam_as_line = CLAMP(showbeam_as_line,0,1);
      use_beamcolor = CLAMP(use_beamcolor,0,1);
      beam_color[0] = CLAMP(beam_color[0], 0, 255);
      beam_color[1] = CLAMP(beam_color[1], 0, 255);
      beam_color[2] = CLAMP(beam_color[2], 0, 255);
      continue;
    }

    if(MatchINI(buffer, "BLOCKSHININESS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &block_shininess);
      scase->updatefaces = 1;
      scase->updateindexcolors = 1;
      continue;
    }
    if(MatchINI(buffer, "BLOCKSPECULAR") == 1){
      float blockspec_temp[4];

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", blockspec_temp, blockspec_temp + 1, blockspec_temp + 2);
      blockspec_temp[3] = 1.0;
      block_specular2 = GetColorPtr(blockspec_temp);
      scase->updatefaces = 1;
      scase->updateindexcolors = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWOPENVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visOpenVents, &visOpenVentsAsOutline);
      continue;
    }
    if(MatchINI(buffer, "SHOWDUMMYVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visDummyVents);
      continue;
    }
    if(MatchINI(buffer, "SHOWOTHERVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &scase->visOtherVents);
      ONEORZERO(scase->visOtherVents);
      continue;
    }
    if(MatchINI(buffer, "SHOWCVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visCircularVents, &circle_outline);
      visCircularVents = CLAMP(visCircularVents, 0, 2);
      circle_outline = CLAMP(circle_outline, 0, 1);
      continue;
    }
    if(MatchINI(buffer, "SHOWTICKS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visFDSticks);
      continue;
    }
    if(MatchINI(buffer, "USERTICKS") == 1){
      fgets(buffer, 255, stream);

      sscanf(buffer, "%i %i %i %i %i %i %f %i", &visUSERticks, &auto_user_tick_placement, &user_tick_sub,
        &user_tick_show_x, &user_tick_show_y, &user_tick_show_z, &user_tick_direction, &ntick_decimals);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", user_tick_origin, user_tick_origin + 1, user_tick_origin + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", user_tick_min, user_tick_min + 1, user_tick_min + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", user_tick_max, user_tick_max + 1, user_tick_max + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", user_tick_step, user_tick_step + 1, user_tick_step + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &user_tick_show_x, &user_tick_show_y, &user_tick_show_z);
      continue;
    }
    if(MatchINI(buffer, "SHOWLABELS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visLabels);
      continue;
    }
    if(MatchINI(buffer, "BOUNDCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", boundcolor, boundcolor + 1, boundcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "TIMEBARCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", timebarcolor, timebarcolor + 1, timebarcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "CONTOURTYPE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &contour_type, &colorbar_linewidth);
      contour_type = CLAMP(contour_type, 0, 2);
      colorbar_linewidth = CLAMP(colorbar_linewidth, 1, 10);
      continue;
    }
    if(MatchINI(buffer, "P3CONT3DSMOOTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &p3cont3dsmooth);
      continue;
    }
    if(MatchINI(buffer, "SURFINC") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &surfincrement);
      continue;
    }
    if(MatchINI(buffer, "FRAMERATE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visFramerate);
      continue;
    }
    if(MatchINI(buffer, "SHOWFRAMERATE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visFramerate);
      continue;
    }
    if(MatchINI(buffer, "SHOWFRAME") == 1 &&
       MatchINI(buffer, "SHOWFRAMERATE") != 1 &&
       MatchINI(buffer, "SHOWFRAMELABEL") != 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visFrame);
      ONEORZERO(visFrame);
      continue;
    }
    if(MatchINI(buffer, "FRAMERATEVALUE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &frameratevalue);
      FrameRateMenu(frameratevalue);
      continue;
    }
    if(MatchINI(buffer, "SHOWSPRINKPART") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visSprinkPart);
      continue;
    }
    if(MatchINI(buffer, "SHOWAXISLABELS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visaxislabels);
      continue;
    }
#ifdef pp_memstatus
    if(MatchINI(buffer, "SHOWMEMLOAD") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visAvailmemory);
      continue;
    }
#endif
    if(MatchINI(buffer, "SHOWBLOCKLABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visMeshlabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWZONEPLANE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visZonePlane, &show_zonelower);
      continue;
    }
    if(MatchINI(buffer, "SHOWVZONE") == 1){
      int vis;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &vis, &show_zonelower);
      if(vis==1){
        visZonePlane = ZONE_YPLANE;
      }
      else{
        visZonePlane = ZONE_HIDDEN;
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWZONEFIRE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &viszonefire);
      if(viszonefire != 0)viszonefire = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWSZONE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visSZone);
      continue;
    }
    if(MatchINI(buffer, "SHOWHZONE") == 1){
      int vis;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &vis);
      if(vis==1){
        visZonePlane = ZONE_ZPLANE;
      }
      else{
        visZonePlane = ZONE_HIDDEN;
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWHAZARDCOLORS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &zonecolortype);
      zonecolortype = CLAMP(zonecolortype, 0, 2);
      continue;
    }
#ifdef pp_SKY
    if(MatchINI(buffer, "SHOWSKY") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visSky);
      ONEORZERO(visSky);
      continue;
    }
#endif
    if(MatchINI(buffer, "SHOWSMOKEPART") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visSmokePart);
      continue;
    }
    if(MatchINI(buffer, "RENDEROPTION") == 1){
      int nheight360_temp = 0;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &render_window_size, &resolution_multiplier, &nheight360_temp);
      if(nheight360_temp > 0){
        nheight360 = nheight360_temp;
        nwidth360 = 2 * nheight360;
      }
      RenderMenu(render_window_size);
      continue;
    }
    if(MatchINI(buffer, "SHOWISONORMALS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_iso_normal);
      if(show_iso_normal != 1)show_iso_normal = 0;
      continue;
    }
    if(MatchINI(buffer, "SHOWISO") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visAIso);
      visAIso &= 7;
      show_iso_shaded = (visAIso & 1) / 1;
      show_iso_outline = (visAIso & 2) / 2;
      show_iso_points = (visAIso & 4) / 4;
      continue;
    }
    if(trainer_mode == 0 && windowresized == 0){
      if(MatchINI(buffer, "WINDOWWIDTH") == 1){
        int scrWidth;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &scrWidth);
        if(scrWidth <= 0){
          scrWidth = GLUTGetScreenWidth();
        }
        if(scrWidth != screenWidth){
          SetScreenSize(&scrWidth, NULL);
          screenWidthINI = scrWidth;
          update_screensize = 1;
        }
        continue;
      }
      if(MatchINI(buffer, "WINDOWHEIGHT") == 1){
        int scrHeight;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &scrHeight);
        if(scrHeight <= 0){
          scrHeight = GLUTGetScreenHeight();
        }
        if(scrHeight != screenHeight){
          SetScreenSize(NULL, &scrHeight);
          screenHeightINI = scrHeight;
          update_screensize = 1;
        }
        continue;
      }
    }
    if(MatchINI(buffer, "SHOWTIMEBAR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visTimebar,&timebar_overlap);
      continue;
    }
    if(MatchINI(buffer, "SHOWCOLORBARS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visColorbarVertical_val, &visColorbarHorizontal_val);
      if(visColorbarVertical_val==1)visColorbarHorizontal_val=0;
      if(visColorbarHorizontal_val==1)visColorbarVertical_val=0;
  // if colorbars are hidden then research mode needs to be off
      update_visColorbars=1;
      continue;
    }
    if(MatchINI(buffer, "EYEVIEW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &rotation_type);
      continue;
    }
    if(MatchINI(buffer, "NOPART") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nopart);
      continue;
    }
    if(MatchINI(buffer, "PARTFAST")==1){
      fgets(buffer, 255, stream);
      if(current_script_command==NULL){
        sscanf(buffer, "%i %i %i", &partfast, &use_partload_threads, &n_partload_threads);
      }
#ifdef pp_PARTFRAME
      use_partload_threads = 0;
#endif
      continue;
    }
    if(MatchINI(buffer, "WINDOWOFFSET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &titlesafe_offsetBASE);
      continue;
    }
    if(MatchINI(buffer, "LIGHTING")==1){
      fgets(buffer, 255, stream);
      if(HaveSmokeSensor()==0){ // don't use lighting if there are devices of type smokesensor
        sscanf(buffer, "%i", &use_lighting);
        update_use_lighting = 1;
      }
      continue;
    }
    if(MatchINI(buffer, "LIGHTFACES")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%d", &light_faces);
      ONEORZERO(light_faces);
      continue;
    }
    if(MatchINI(buffer, "LIGHTANGLES0")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %i", &glui_light_az0, &glui_light_elev0, &use_light0);
      continue;
    }
    if(MatchINI(buffer, "LIGHTANGLES1")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %i", &glui_light_az1, &glui_light_elev1, &use_light1);
      continue;
    }
    if(MatchINI(buffer, "LIGHTPROP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", ambientlight, ambientlight + 1, ambientlight + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", diffuselight, diffuselight + 1, diffuselight + 2);
      continue;
    }
    if(MatchINI(buffer, "LABELSTARTUPVIEW") == 1){
      char *front;

      fgets(buffer, 255, stream);
      front = TrimFront(buffer);
      TrimBack(front);
      strcpy(viewpoint_label_startup, front);
      update_startup_view = 3;
      continue;
    }
    if(MatchINI(buffer, "USER_ROTATE") == 1){
      if(fgets(buffer, 255, stream) == NULL)break;
      sscanf(buffer, "%i %i %f %f %f", &glui_rotation_index_ini, &show_rotation_center, &xcenCUSTOM, &ycenCUSTOM, &zcenCUSTOM);
      if(glui_rotation_index_ini>=0)glui_rotation_index = ROTATE_ABOUT_FDS_CENTER;
      update_rotation_center_ini = 1;
      continue;
    }
    if(MatchINI(buffer, "INPUT_FILE") == 1){
      size_t len;

      if(fgets(buffer, 255, stream) == NULL)break;
      len = strlen(buffer);
      buffer[len - 1] = '\0';
      TrimBack(buffer);
      len = strlen(buffer);

      FREEMEMORY(INI_fds_filein);
      if(NewMemory((void **)&INI_fds_filein, (unsigned int)(len + 1)) == 0)return 2;
      STRCPY(INI_fds_filein, buffer);
      continue;
    }
    if(MatchINI(buffer, "VIEWPOINT5") == 1 ||
       MatchINI(buffer, "VIEWPOINT6") == 1){
      int p_type;
      float *eye, mat[16], *az_elev;
      int is_viewpoint6 = 0;
      float xyzmaxdiff_local = -1.0;
      float xmin_local = 0.0, ymin_local = 0.0, zmin_local = 0.0;
      char name_ini[32];
      float zoom_in;
      int zoomindex_in;
      cameradata camera_local, *ci;
      char *bufferptr;

      ci = &camera_local;

      if(MatchINI(buffer, "VIEWPOINT6") == 1)is_viewpoint6 = 1;

      eye = ci->eye;
      az_elev = ci->az_elev;

      strcpy(name_ini, "ini");
      InitCamera(ci, name_ini);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %f %f %f %f",
        &ci->rotation_type, &ci->rotation_index, &ci->view_id,
        &xyzmaxdiff_local, &xmin_local, &ymin_local, &zmin_local);

      zoom_in = zoom;
      zoomindex_in = zoomindex;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f %i", eye, eye + 1, eye + 2, &zoom_in, &zoomindex_in);
      if(xyzmaxdiff_local>0.0){
        eye[0] = xmin_local + eye[0] * xyzmaxdiff_local;
        eye[1] = ymin_local + eye[1] * xyzmaxdiff_local;
        eye[2] = zmin_local + eye[2] * xyzmaxdiff_local;
      }
      zoom = zoom_in;
      zoomindex = zoomindex_in;
      if(zoomindex != -1){
        if(zoomindex<0)zoomindex = ZOOMINDEX_ONE;
        if(zooms[MAX_ZOOMS]>0.0&&zoomindex>MAX_ZOOMS)zoomindex = ZOOMINDEX_ONE;
        if(zooms[MAX_ZOOMS]<=0.0&&zoomindex>MAX_ZOOMS-1)zoomindex = ZOOMINDEX_ONE;
        zoom = zooms[zoomindex];
      }
      else{
        if(zoom<zooms[0]){
          zoom = zooms[0];
          zoomindex = 0;
        }
        if(zoomindex!=MAX_ZOOMS&&zoom>zooms[MAX_ZOOMS-1]){
          zoom = zooms[MAX_ZOOMS-1];
          zoomindex = MAX_ZOOMS-1;
        }
      }
      updatezoommenu = 1;
      p_type = 0;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %i", &ci->view_angle, &ci->azimuth, &ci->elevation, &p_type);
      if(p_type != 1)p_type = 0;
      ci->projection_type = p_type;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", &ci->xcen, &ci->ycen, &ci->zcen);
      if(xyzmaxdiff_local>0.0){
        ci->xcen = xmin_local + ci->xcen*xyzmaxdiff_local;
        ci->ycen = ymin_local + ci->ycen*xyzmaxdiff_local;
        ci->zcen = zmin_local + ci->zcen*xyzmaxdiff_local;
      }

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", az_elev, az_elev + 1);

      if(is_viewpoint6 == 1){
        float *q;

        for(i = 0; i<16; i++){
          mat[i] = 0.0;
          if(i % 5 == 0)mat[i] = 1.0;
        }
        q = ci->quaternion;
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %f %f %f", &ci->quat_defined, q, q + 1, q + 2, q + 3);
      }

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", mat, mat + 1, mat + 2, mat + 3);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", mat + 4, mat + 5, mat + 6, mat + 7);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", mat + 8, mat + 9, mat + 10, mat + 11);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", mat + 12, mat + 13, mat + 14, mat + 15);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i %i %i",
        &ci->clip_mode,
        &ci->clip_xmin, &ci->clip_ymin, &ci->clip_zmin,
        &ci->clip_xmax, &ci->clip_ymax, &ci->clip_zmax);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f %f %f",
        &ci->xmin, &ci->ymin, &ci->zmin,
        &ci->xmax, &ci->ymax, &ci->zmax);
      if(xyzmaxdiff_local>0.0){
        ci->xmin = xmin_local + ci->xmin*xyzmaxdiff_local;
        ci->xmax = xmin_local + ci->xmax*xyzmaxdiff_local;
        ci->ymin = ymin_local + ci->ymin*xyzmaxdiff_local;
        ci->zmax = ymin_local + ci->ymax*xyzmaxdiff_local;
        ci->ymin = zmin_local + ci->zmin*xyzmaxdiff_local;
        ci->zmax = zmin_local + ci->zmax*xyzmaxdiff_local;
      }

      fgets(buffer, 255, stream);
      TrimBack(buffer);
      bufferptr = TrimFront(buffer);
      strcpy(ci->name, bufferptr);
      InitCameraList();
      InsertCamera(&camera_list_first, ci, bufferptr);

      GLUIEnableResetSavedView();
      ci->dirty = 1;
      ci->defined = 1;
      continue;
    }
    if(MatchINI(buffer, "COLORTABLE") == 1){
      int nctableinfo;
      colortabledata *ctableinfo = NULL;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nctableinfo);
      nctableinfo = MAX(nctableinfo, 0);
      if(nctableinfo>0){
        NewMemory((void **)&ctableinfo, nctableinfo*sizeof(colortabledata));
        for(i = 0; i<nctableinfo; i++){
          colortabledata *rgbi;
          char *labelptr, *percenptr, label[256];
          int  colori[4];


          rgbi = ctableinfo + i;
          fgets(buffer, 255, stream);
          percenptr = strchr(buffer, '%');
          if(percenptr != NULL){
            labelptr = TrimFront(percenptr + 1);
            TrimBack(labelptr);
            strcpy(rgbi->label, labelptr);
            percenptr[0] = 0;
          }
          else{
            sprintf(label, "Color %i", i + 1);
            strcpy(rgbi->label, label);
          }
          colori[3] = 255;
          sscanf(buffer, "%i %i %i %i", colori, colori + 1, colori + 2, colori + 3);
          rgbi->color[0] = CLAMP(colori[0], 0, 255);
          rgbi->color[1] = CLAMP(colori[1], 0, 255);
          rgbi->color[2] = CLAMP(colori[2], 0, 255);
          rgbi->color[3] = CLAMP(colori[3], 0, 255);
        }
        UpdateColorTable(ctableinfo, nctableinfo);
        FREEMEMORY(ctableinfo);
      }
      continue;
    }

    if(MatchINI(buffer, "ISOCOLORS") == 1){
      int nn, n_iso_c = 0;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %i %i", &iso_shininess, &iso_transparency, &iso_transparency_option, &iso_opacity_change);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", iso_specular, iso_specular + 1, iso_specular + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n_iso_c);
      for(nn = 0; nn<MAX_ISO_COLORS; nn++){
        float *iso_color;

        iso_color = iso_colors + 4 * nn;
        if(nn < n_iso_c){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%f %f %f %f", iso_color, iso_color + 1, iso_color + 2, iso_color + 3);
        }
        iso_color[0] = CLAMP(iso_color[0], 0.0, 1.0);
        iso_color[1] = CLAMP(iso_color[1], 0.0, 1.0);
        iso_color[2] = CLAMP(iso_color[2], 0.0, 1.0);
        iso_color[3] = CLAMP(iso_color[3], 0.0, 1.0);
      }
      UpdateIsoColors();
      GLUIUpdateIsoColorlevel();
      continue;
    }
    if(MatchINI(buffer, "UNITCLASSES") == 1){
      int nuc;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nuc);
      for(i = 0; i<nuc; i++){
        int unit_index;

        fgets(buffer, 255, stream);
        if(i>nunitclasses - 1)continue;
        sscanf(buffer, "%i", &unit_index);
        unitclasses[i].unit_index = unit_index;
      }
      continue;
    }
    if(MatchINI(buffer, "SMOOTHLINES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &antialiasflag);
      continue;
    }
    {
      if(MatchINI(buffer, "SMOKECULL") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &smokecullflag);
        continue;
      }
      if(MatchINI(buffer, "SMOKESKIP") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %i %i %i", &smokeskipm1, &smoke3d_skip, &smoke3d_skipx, &smoke3d_skipy, &smoke3d_skipz);
        smoke3d_skip = CLAMP(smoke3d_skip,1,10);
        smoke3d_skipx = CLAMP(smoke3d_skipx, 1, 10);
        smoke3d_skipy = CLAMP(smoke3d_skipy, 1, 10);
        smoke3d_skipz = CLAMP(smoke3d_skipz, 1, 10);
        continue;
      }
      if(MatchINI(buffer, "SLICESKIP")==1){
        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%i %i %i %i", &slice_skip, &slice_skipx, &slice_skipy, &slice_skipz);
        if(slice_skip<1)slice_skip = 1;
        slice_skipx = slice_skip;
        slice_skipy = slice_skip;
        slice_skipz = slice_skip;
        continue;
      }
      if(MatchINI(buffer, "SMOKEALBEDO") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f", &scase->smoke_albedo);
        scase->smoke_albedo = CLAMP(scase->smoke_albedo, 0.0, 1.0);
        continue;
      }
      if(MatchINI(buffer, "SMOKEFIREPROP") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i", &use_opacity_depth_ini, &use_opacity_multiplier_ini);
        use_opacity_ini = 1;
        continue;
      }
      if(MatchINI(buffer, "SMOKEPROP")==1){
        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%f", &glui_smoke3d_extinct);
        glui_smoke3d_extinct_default = glui_smoke3d_extinct;
        continue;
      }
      if(MatchINI(buffer, "FIRECOLOR") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %i", fire_color_int255, fire_color_int255+1, fire_color_int255+2);
        fire_color_int255[0] = CLAMP(fire_color_int255[0], 0, 255);
        fire_color_int255[1] = CLAMP(fire_color_int255[1], 0, 255);
        fire_color_int255[2] = CLAMP(fire_color_int255[2], 0, 255);
        continue;
      }
      if(MatchINI(buffer, "SMOKECOLOR") == 1){
        int f_gray_smoke = -1;

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %i, %i", smoke_color_int255, smoke_color_int255+1, smoke_color_int255+2, &f_gray_smoke);
        smoke_color_int255[0] = CLAMP(smoke_color_int255[0], 0, 255);
        smoke_color_int255[1] = CLAMP(smoke_color_int255[1], 0, 255);
        smoke_color_int255[2] = CLAMP(smoke_color_int255[2], 0, 255);
        if(f_gray_smoke>=0){
          force_gray_smoke = f_gray_smoke;
          if(force_gray_smoke!=0)force_gray_smoke = 1;
        }
        continue;
      }
      if(MatchINI(buffer, "CO2COLOR") == 1){
         fgets(buffer, 255, stream);
         sscanf(buffer, " %i %i %i", co2_color_int255,co2_color_int255+1,co2_color_int255+2);
         co2_color_int255[0] = CLAMP(co2_color_int255[0], 0, 255);
         co2_color_int255[1] = CLAMP(co2_color_int255[1], 0, 255);
         co2_color_int255[2] = CLAMP(co2_color_int255[2], 0, 255);
         continue;
       }
      if(MatchINI(buffer, "HRRPUVCUTOFF")==1){
        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%f", &scase->global_hrrpuv_cutoff_default);
        scase->global_hrrpuv_cutoff = scase->global_hrrpuv_cutoff_default;
        continue;
      }
      if(MatchINI(buffer, "FIREDEPTH") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f %i %i", &fire_halfdepth,&co2_halfdepth,&emission_factor,&use_fire_alpha, &force_alpha_opaque);
        continue;
      }
      if(MatchINI(buffer, "VIEWTOURFROMPATH") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %f", &viewtourfrompath, &tour_snap, &tour_snap_time);
        continue;
      }
      if(MatchINI(buffer, "VIEWALLTOURS") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &viewalltours);
        continue;
      }
      if(MatchINI(buffer, "SHOWTOURROUTE") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &edittour);
        continue;
      }
      if(MatchINI(buffer, "TIMEOFFSET") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f", &timeoffset);
        continue;
      }
      if(MatchINI(buffer, "SHOWPATHNODES") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &show_path_knots);
        continue;
      }
      if(MatchINI(buffer, "SHOWGEOMTERRAIN")==1){
        int nt;

        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%i %i %i %i %i",
          &nt, &terrain_show_geometry_surface, &terrain_show_geometry_outline, &terrain_show_geometry_points, &terrain_showonly_top);
        if(scase->terrain_texture_coll.terrain_textures!=NULL){
          for(i = 0; i<MIN(nt, scase->terrain_texture_coll.nterrain_textures); i++){
            texturedata *texti;

            texti = scase->terrain_texture_coll.terrain_textures+i;
            if(fgets(buffer, 255, stream)==NULL)break;
            sscanf(buffer, "%i ", &(texti->display));
          }
        }
        continue;
      }

      if(MatchINI(buffer, "SHOWIGNITION") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i", &vis_threshold, &vis_onlythreshold);
        continue;
      }
      if(MatchINI(buffer, "SHOWTHRESHOLD") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %f", &vis_threshold, &vis_onlythreshold, &temp_threshold);
        continue;
      }
      if(MatchINI(buffer, "TOUR_AVATAR") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        continue;
      }
      if(MatchINI(buffer, "TOURCIRCLE") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer,"%f %f %f %f %f %f %f %f",
          tour_circular_center+0, tour_circular_center+1, tour_circular_center+2,
          tour_circular_view+0, tour_circular_view+1, tour_circular_view+2,
          &tour_circular_radius, &tour_circular_angle0);
        continue;
      }

      /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ GCOLORBAR ++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      */

      if(MatchINI(buffer, "GCOLORBAR") == 1){
        colorbardata *cbi;
        int r1, g1, b1;
        int n;
        int ncolorbarini;

        fgets(buffer, 255, stream);
        ncolorbarini = 0;
        sscanf(buffer, "%i", &ncolorbarini);

        ncolorbarini = MAX(ncolorbarini, 0);
        InitDefaultColorbars(&colorbars, ncolorbarini, show_extreme_mindata,
                             rgb_below_min, show_extreme_maxdata,
                             rgb_above_max, colorbarcopyinfo);
        UpdateColorbarDialogs();
        UpdateCurrentColorbar(colorbars.colorbarinfo + colorbartype);
        update_colorbar_dialog = 0;

        colorbars.ncolorbars = colorbars.ndefaultcolorbars + ncolorbarini;
        for(n = colorbars.ndefaultcolorbars; n<colorbars.ncolorbars; n++){
          char *cb_buffptr;

          cbi = colorbars.colorbarinfo + n;
          fgets(buffer, 255, stream);
          TrimBack(buffer);
          cb_buffptr = TrimFront(buffer);
          strcpy(cbi->menu_label, cb_buffptr);
          cbi->type = CB_USER;
          strcpy(cbi->colorbar_type, "user defined");
          cbi->interp = INTERP_LAB;

          fgets(buffer, 255, stream);
          sscanf(buffer, "%i %i", &cbi->nnodes, &cbi->nodehilight);
          if(cbi->nnodes<0)cbi->nnodes = 0;
          if(cbi->nodehilight<0 || cbi->nodehilight >= cbi->nnodes){
            cbi->nodehilight = 0;
          }

          for(i = 0; i<cbi->nnodes; i++){
            int icbar;
            int nn;

            fgets(buffer, 255, stream);
            r1 = -1; g1 = -1; b1 = -1;
            sscanf(buffer, "%i %i %i %i", &icbar, &r1, &g1, &b1);
            cbi->node_index[i] = icbar;
            nn = 3 * i;
            cbi->node_rgb[nn]     = r1;
            cbi->node_rgb[nn + 1] = g1;
            cbi->node_rgb[nn + 2] = b1;
          }
          RemapColorbar(cbi, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
          UpdateColorbarDialogs();
        }

        continue;
      }

      if(MatchINI(buffer, "TOURCOLORS") == 1){
        float *col;

        col = tourcol_selectedpathline;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_selectedpathlineknots;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_selectedknot;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_pathline;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_pathknots;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_text;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_avatar;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        continue;
      }

      if(localfile!=1)continue;

// ---------------------------------------------------------------------------------------------------------
//   keywords below are 'local', only in the casename.ini file
// ---------------------------------------------------------------------------------------------------------

#define WINDROSE_PER_ROW 10
      if(MatchINI(buffer, "WINDROSESHOWHIDE")==1){
        int i1, i2, *vals, nrows;

        FREEMEMORY(windrosez_showhide);
        nwindrosez_showhide = 0;
        fgets(buffer, 255, stream);
        sscanf(buffer, " %i", &nwindrosez_showhide);
        if(nwindrosez_showhide>0){
          nrows = ((nwindrosez_showhide-1)/WINDROSE_PER_ROW+1);
          NewMemory((void **)&windrosez_showhide, nrows*WINDROSE_PER_ROW*sizeof(int));
          for(vals=windrosez_showhide,i=0;i<nrows;i++,vals+=WINDROSE_PER_ROW){
            int j;

            i1 = WINDROSE_PER_ROW*i;
            i2 = MIN(i1+WINDROSE_PER_ROW,nwindrosez_showhide);
            fgets(buffer, 255, stream);
            sscanf(buffer, " %i %i %i %i %i %i %i %i %i %i ",
              vals,vals+1,vals+2,vals+3,vals+4,vals+5,vals+6,vals+7,vals+8,vals+9);
            for(j=i1;j<i2;j++){
              windrosez_showhide[j] = CLAMP(vals[j-i1],0,1);
            }
          }
          update_windrose_showhide = 1;
        }
      }

      if(MatchINI(buffer, "PATCHDATAOUT") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %f %f %f %f %f %f %f", &output_patchdata,
          &patchout_tmin, &patchout_tmax,
          &patchout_xmin, &patchout_xmax,
          &patchout_ymin, &patchout_ymax,
          &patchout_zmin, &patchout_zmax
          );
        ONEORZERO(output_patchdata);
        continue;
      }
      if(MatchINI(buffer, "SMOKE3DCUTOFFS") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f", &load_3dsmoke_cutoff, &scase->load_hrrpuv_cutoff);
        continue;
      }

      /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ LABEL ++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      */

      if(MatchINI(buffer, "LABEL") == 1 ||
         (event_file_exists==0&&MatchINI(buffer, "TICKLABEL") == 1)){

        /*
        LABEL
        x y z r g b tstart tstop
        label

        */
        float *xyz, *rgbtemp, *tstart_stop;
        labeldata labeltemp, *labeli;
        int *useforegroundcolor;
        char *bufferptr;
        int *show_always;
        int ticklabel = 0;
        float *xyztick, *xyztickdir;
        int *showtick;

        if(MatchINI(buffer, "TICKLABEL") == 1)ticklabel = 1;

        labeli = &labeltemp;

        labeli->labeltype = TYPE_INI;
        xyz = labeli->xyz;
        rgbtemp = labeli->frgb;
        xyztick = labeli->tick_begin;
        xyztickdir = labeli->tick_direction;
        showtick = &labeli->show_tick;

        useforegroundcolor = &labeli->useforegroundcolor;
        tstart_stop = labeli->tstart_stop;
        show_always = &labeli->show_always;

        fgets(buffer, 255, stream);
        rgbtemp[0] = -1.0;
        rgbtemp[1] = -1.0;
        rgbtemp[2] = -1.0;
        rgbtemp[3] = 1.0;
        tstart_stop[0] = -1.0;
        tstart_stop[1] = -1.0;
        *useforegroundcolor = -1;
        *show_always = 1;

        sscanf(buffer, "%f %f %f %f %f %f %f %f %i %i",
          xyz, xyz + 1, xyz + 2,
          rgbtemp, rgbtemp + 1, rgbtemp + 2,
          tstart_stop, tstart_stop + 1, useforegroundcolor, show_always);

        if(ticklabel == 1){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%f %f %f %f %f %f %i",
            xyztick, xyztick + 1, xyztick + 2,
            xyztickdir, xyztickdir + 1, xyztickdir + 2,
            showtick);
          *showtick = CLAMP(*showtick, 0, 1);
        }
        else{
          xyztick[0] = 0.0;
          xyztick[1] = 0.0;
          xyztick[2] = 0.0;
          xyztickdir[0] = 1.0;
          xyztickdir[1] = 0.0;
          xyztickdir[2] = 0.0;
          *showtick = 0;
        }
        *show_always = CLAMP(*show_always, 0, 1);
        *useforegroundcolor = CLAMP(*useforegroundcolor, -1, 1);
        if(*useforegroundcolor == -1){
          if(rgbtemp[0]<0.0 || rgbtemp[1]<0.0 || rgbtemp[2]<0.0 || rgbtemp[0]>1.0 || rgbtemp[1]>1.0 || rgbtemp[2]>1.0){
            *useforegroundcolor = 1;
          }
          else{
            *useforegroundcolor = 0;
          }
        }
        fgets(buffer, 255, stream);
        TrimBack(buffer);
        bufferptr = TrimFront(buffer);
        strcpy(labeli->name, bufferptr);
        LabelInsert(&scase->labelscoll, labeli);
        continue;
      }
      if(MatchINI(buffer, "VIEWTIMES") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %i", &scase->tourcoll.tour_tstart, &scase->tourcoll.tour_tstop, &scase->tourcoll.tour_ntimes);
        if(scase->tourcoll.tour_ntimes<2)scase->tourcoll.tour_ntimes = 2;
        ReallocTourMemory(&scase->tourcoll);
        continue;
      }
      if(MatchINI(buffer, "SHOOTER") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", shooter_xyz, shooter_xyz + 1, shooter_xyz + 2);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", shooter_dxyz, shooter_dxyz + 1, shooter_dxyz + 2);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", shooter_uvw, shooter_uvw + 1, shooter_uvw + 2);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", &shooter_velmag, &shooter_veldir, &shooterpointsize);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %i %i %i", &shooter_fps, &shooter_vel_type, &shooter_nparts, &visShooter, &shooter_cont_update);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f", &shooter_duration, &shooter_v_inf);
        continue;
      }
      if(MatchINI(buffer, "SCRIPTFILE") == 1){
        if(fgets(buffer2, 255, stream) == NULL)break;
        InsertScriptFile(RemoveComment(buffer2));
        updatemenu = 1;
        continue;
      }
      if(MatchINI(buffer, "SHOWDEVICES") == 1){
        sv_object *obj_typei;
        char *dev_label;
        int ndevices_ini;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %i", &ndevices_ini, &object_outlines, &object_box);

        for(i = 0; i<scase->objectscoll.nobject_defs; i++){
          obj_typei = scase->objectscoll.object_defs[i];
          obj_typei->visible = 0;
        }
        for(i = 0; i<ndevices_ini; i++){
          fgets(buffer, 255, stream);
          TrimBack(buffer);
          dev_label = TrimFront(buffer);
          obj_typei = GetSmvObject(&scase->objectscoll, dev_label);
          if(obj_typei != NULL){
            obj_typei->visible = 1;
          }
        }
        continue;
      }
      if(MatchINI(buffer, "XYZCLIP") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &clip_mode);
        clip_mode = CLAMP(clip_mode, 0, CLIP_MAX);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %i %f", &clipinfo.clip_xmin, &clipinfo.xmin, &clipinfo.clip_xmax, &clipinfo.xmax);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %i %f", &clipinfo.clip_ymin, &clipinfo.ymin, &clipinfo.clip_ymax, &clipinfo.ymax);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %i %f", &clipinfo.clip_zmin, &clipinfo.zmin, &clipinfo.clip_zmax, &clipinfo.zmax);
        updateclipvals = 1;
        continue;
      }


      /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ TICKS ++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      */

      if(MatchINI(buffer, "TICKS") == 1){
        scase->ntickinfo++;
        if(scase->tickinfo==NULL){
          NewMemory((void **)&scase->tickinfo, (scase->ntickinfo)*sizeof(tickdata));
        }
        else{
          ResizeMemory((void **)&scase->tickinfo, (scase->ntickinfo)*sizeof(tickdata));
        }

        {
          tickdata *ticki;
          float *begt, *endt;
          int *nbarst;
          float term;
          float length = 0.0;
          float *dxyz;
          float sum;

          ticki = scase->tickinfo + scase->ntickinfo - 1;
          begt = ticki->begin;
          endt = ticki->end;
          nbarst = &ticki->nbars;
          dxyz = ticki->dxyz;


          /*
          TICKS
          b1 b2 b3 e1 e2 e3 nb
          ticklength tickdir tickcolor (r g b) tickwidth
          */
          if(fgets(buffer, 255, stream) == NULL)break;
          *nbarst = 0;
          sscanf(buffer, "%f %f %f %f %f %f %i", begt, begt + 1, begt + 2, endt, endt + 1, endt + 2, nbarst);
          if(*nbarst<1)*nbarst = 1;
          if(fgets(buffer, 255, stream) == NULL)break;
          {
            float *rgbtemp;

            rgbtemp = ticki->rgb;
            VEC3EQCONS(rgbtemp, -1.0);
            ticki->width = -1.0;
            sscanf(buffer, "%f %i %f %f %f %f", &ticki->dlength, &ticki->dir, rgbtemp, rgbtemp + 1, rgbtemp + 2, &ticki->width);
            if(rgbtemp[0]<0.0 || rgbtemp[0]>1.0 ||
              rgbtemp[1]<0.0 || rgbtemp[1]>1.0 ||
              rgbtemp[2]<0.0 || rgbtemp[2]>1.0){
              ticki->useforegroundcolor = 1;
            }
            else{
              ticki->useforegroundcolor = 0;
            }
            if(ticki->width<0.0)ticki->width = 1.0;
          }
          for(i = 0; i<3; i++){
            term = endt[i] - begt[i];
            length += term*term;
          }
          if(length <= 0.0){
            endt[0] = begt[0] + 1.0;
            length = 1.0;
          }
          ticki->length = sqrt(length);
          VEC3EQCONS(dxyz, 0.0);
          switch(ticki->dir){
          case XLEFT:
          case XRIGHT:
            dxyz[0] = 1.0;
            break;
          case YBACK:
          case YFRONT:
            dxyz[1] = 1.0;
            break;
          case ZBOTTOM:
          case ZTOP:
            dxyz[2] = 1.0;
            break;
          default:
            assert(FFALSE);
            break;
          }
          if(ticki->dir<0){
            for(i = 0; i<3; i++){
              dxyz[i] = -dxyz[i];
            }
          }
          sum = 0.0;
          sum = dxyz[0] * dxyz[0] + dxyz[1] * dxyz[1] + dxyz[2] * dxyz[2];
          if(sum>0.0){
            sum = sqrt(sum);
            dxyz[0] *= (ticki->dlength / sum);
            dxyz[1] *= (ticki->dlength / sum);
            dxyz[2] *= (ticki->dlength / sum);
          }
        }
        continue;
      }
      if(MatchINI(buffer, "PROPINDEX") == 1){
        int nvals;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &nvals);
        for(i = 0; i<nvals; i++){
          propdata *propi;
          int ind, val;

          fgets(buffer, 255, stream);
          sscanf(buffer, "%i %i", &ind, &val);
          if(ind<0 || ind>scase->propcoll.npropinfo - 1)continue;
          propi = scase->propcoll.propinfo + ind;
          if(val<0 || val>propi->nsmokeview_ids - 1)continue;
          propi->smokeview_id = propi->smokeview_ids[val];
          propi->smv_object = propi->smv_objects[val];
        }
        for(i = 0; i<scase->npartclassinfo; i++){
          partclassdata *partclassi;

          partclassi = scase->partclassinfo + i;
          UpdatePartClassDepend(partclassi);

        }
        continue;
      }
      if(MatchINI(buffer, "partclassdataVIS") == 1){
        int ntemp;
        int j;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &ntemp);

        for(j = 0; j<ntemp; j++){
          partclassdata *partclassj;

          if(j>scase->npartclassinfo)break;

          partclassj = scase->partclassinfo + j;
          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &partclassj->vis_type);
        }
        continue;
      }

      if(MatchINI(buffer, "GSLICEPARMS") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %i %i", &vis_gslice_data, &show_gslice_triangles, &show_gslice_triangulation, &show_gslice_normal);
        ONEORZERO(vis_gslice_data);
        ONEORZERO(show_gslice_triangles);
        ONEORZERO(show_gslice_triangulation);
        ONEORZERO(show_gslice_normal);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f %f", gslice_xyz, gslice_xyz + 1, gslice_xyz + 2);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f", gslice_normal_azelev, gslice_normal_azelev + 1);
        update_gslice = 1;
        continue;
      }
      if(MatchINI(buffer, "GRIDPARMS") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %i", &visx_all, &visy_all, &visz_all);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %i", &iplotx_all, &iploty_all, &iplotz_all);
        if(iplotx_all>nplotx_all - 1)iplotx_all = 0;
        if(iploty_all>nploty_all - 1)iploty_all = 0;
        if(iplotz_all>nplotz_all - 1)iplotz_all = 0;
        continue;
      }
      {
        int tours_flag;
        int nkeyframes;
        float key_time, key_xyz[3], key_view[3];
        float key_pause_time;

        tours_flag = 0;
    //TOUR7
    // index
    //   tourlabel
    //   nkeyframes avatar_index display
    //      time pause_time x y z
    //      vx vy vz
        if(have_tours==0&&MatchINI(buffer, "TOUR7") == 1)tours_flag = 1;
        if(tours_flag == 1){
          have_tour7 = 1;
          if(scase->tourcoll.ntourinfo > 0){
            for(i = 0; i < scase->tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = scase->tourcoll.tourinfo + i;
              FreeTour(touri);
            }
            FREEMEMORY(scase->tourcoll.tourinfo);
          }
          scase->tourcoll.ntourinfo = 0;

          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &scase->tourcoll.ntourinfo);
          scase->tourcoll.ntourinfo++;
          if(scase->tourcoll.ntourinfo > 0){
            if(NewMemory((void **)&scase->tourcoll.tourinfo, scase->tourcoll.ntourinfo*sizeof(tourdata)) == 0)return 2;
            for(i = 0; i < scase->tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = scase->tourcoll.tourinfo + i;
              touri->path_times = NULL;
              touri->display = 0;
            }
          }
          ReallocTourMemory(&scase->tourcoll);
          InitCircularTour(scase->tourcoll.tourinfo,ncircletournodes,INIT);
          {
            keyframe *thisframe, *addedframe;
            tourdata *touri;
            int glui_avatar_index_local;

            for(i = 1; i < scase->tourcoll.ntourinfo; i++){
              int j;

              touri = scase->tourcoll.tourinfo + i;
              InitTour(&scase->tourcoll, touri);
              fgets(buffer, 255, stream);
              TrimBack(buffer);
              strcpy(touri->label, TrimFront(buffer));

              fgets(buffer, 255, stream);
              glui_avatar_index_local = 0;
              sscanf(buffer, "%i %i %i", &nkeyframes, &glui_avatar_index_local, &touri->display2);
              glui_avatar_index_local = CLAMP(glui_avatar_index_local, 0, scase->objectscoll.navatar_types - 1);
              touri->glui_avatar_index = glui_avatar_index_local;
              if(touri->display2 != 1)touri->display2 = 0;
              touri->nkeyframes = nkeyframes;

              if(NewMemory((void **)&touri->keyframe_times, nkeyframes*sizeof(float)) == 0)return 2;
              if(NewMemory((void **)&touri->path_times, scase->tourcoll.tour_ntimes*sizeof(float)) == 0)return 2;
              thisframe = &touri->first_frame;
              for(j = 0; j < nkeyframes; j++){
                key_pause_time = 0.0;
                key_view[0] = 0.0;
                key_view[1] = 0.0;
                key_view[2] = 0.0;
                fgets(buffer, 255, stream);
                sscanf(buffer, "%f %f %f %f %f",
                  &key_time, &key_pause_time, key_xyz, key_xyz + 1, key_xyz + 2);

                fgets(buffer, 255, stream);
                sscanf(buffer, "%f %f %f", key_view, key_view + 1, key_view + 2);
                addedframe = AddFrame(thisframe, key_time, key_pause_time, key_xyz, key_view);
                thisframe = addedframe;
                touri->keyframe_times[j] = key_time;
              }
            }
          }
          if(tours_flag == 1){
            for(i = 0; i < scase->tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = scase->tourcoll.tourinfo + i;
              touri->first_frame.next->prev = &touri->first_frame;
              touri->last_frame.prev->next = &touri->last_frame;
            }
            UpdateTourMenuLabels();
            CreateTourPaths();
            UpdateTimes();
            plotstate = GetPlotState(DYNAMIC_PLOTS);
            selectedtour_index = TOURINDEX_MANUAL;
            selected_frame = NULL;
            selected_tour = NULL;
            if(viewalltours == 1)TourMenu(MENU_TOUR_SHOWALL);
          }
          else{
            scase->tourcoll.ntourinfo = 0;
          }
          strcpy(buffer, "1.00000 1.00000 2.0000 0");
          TrimMZeros(buffer);
          continue;
        }
        if(MatchINI(buffer, "TOURINDEX")){
          if(fgets(buffer, 255, stream) == NULL)break;
          sscanf(buffer, "%i", &selectedtour_index_ini);
          if(selectedtour_index_ini < 0)selectedtour_index_ini = -1;
          update_selectedtour_index = 1;
        }
      }
    }
      {
        int tours_flag;
        int nkeyframes;
        float key_time, key_xyz[3], key_az_path, key_view[3], zzoom;
        float key_pause_time;
        int viewtype, uselocalspeed;

        tours_flag = 0;
        if(have_tour7==0&&MatchINI(buffer, "TOURS") == 1)tours_flag = 1;
        if(tours_flag == 1){
          have_tours = 1;
          if(scase->tourcoll.ntourinfo > 0){
            for(i = 0; i < scase->tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = scase->tourcoll.tourinfo + i;
              FreeTour(touri);
            }
            FREEMEMORY(scase->tourcoll.tourinfo);
          }
          scase->tourcoll.ntourinfo = 0;

          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &scase->tourcoll.ntourinfo);
          scase->tourcoll.ntourinfo++;
          if(scase->tourcoll.ntourinfo > 0){
            if(NewMemory((void **)&scase->tourcoll.tourinfo, scase->tourcoll.ntourinfo*sizeof(tourdata)) == 0)return 2;
            for(i = 0; i < scase->tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = scase->tourcoll.tourinfo + i;
              touri->path_times = NULL;
              touri->display = 0;
            }
          }
          ReallocTourMemory(&scase->tourcoll);
          InitCircularTour(scase->tourcoll.tourinfo,ncircletournodes,INIT);
          {
            keyframe *thisframe, *addedframe;
            tourdata *touri;
            int glui_avatar_index_local;

            for(i = 1; i < scase->tourcoll.ntourinfo; i++){
              int j;
              float dummy;
              int idummy;

              touri = scase->tourcoll.tourinfo + i;
              InitTour(&scase->tourcoll, touri);
              fgets(buffer, 255, stream);
              TrimBack(buffer);
              strcpy(touri->label, TrimFront(buffer));

              fgets(buffer, 255, stream);
              glui_avatar_index_local = 0;
              sscanf(buffer, "%i %i %f %i %i",
                &nkeyframes, &idummy, &dummy, &glui_avatar_index_local, &touri->display2);
              glui_avatar_index_local = CLAMP(glui_avatar_index_local, 0, scase->objectscoll.navatar_types - 1);
              touri->glui_avatar_index = glui_avatar_index_local;
              if(touri->display2 != 1)touri->display2 = 0;
              touri->nkeyframes = nkeyframes;

              if(NewMemory((void **)&touri->keyframe_times, nkeyframes*sizeof(float)) == 0)return 2;
              if(NewMemory((void **)&touri->path_times, scase->tourcoll.tour_ntimes*sizeof(float)) == 0)return 2;
              thisframe = &touri->first_frame;
              for(j = 0; j < nkeyframes; j++){
                key_view[0] = 0.0;
                key_view[1] = 0.0;
                key_view[2] = 0.0;
                key_az_path = 0.0;
                viewtype = 0;
                zzoom = 1.0;
                uselocalspeed = 0;
                fgets(buffer, 255, stream);

                sscanf(buffer, "%f %f %f %f %i",
                  &key_time,
                  key_xyz, key_xyz + 1, key_xyz + 2,
                  &viewtype);
                key_pause_time = 0.0;

                if(viewtype == 0){
                  float dummy3[3];

                  sscanf(buffer, "%f %f %f %f %i %f %f %f %f %f %f %f %i",
                    &key_time,
                    key_xyz, key_xyz + 1, key_xyz + 2,
                    &viewtype, &key_az_path, &dummy, &dummy,
                    dummy3, dummy3 + 1, dummy3 + 2,
                    &zzoom, &uselocalspeed);
                }
                else{
                  float dummy3[3];

                  sscanf(buffer, "%f %f %f %f %i %f %f %f %f %f %f %f %i",
                    &key_time,
                    key_xyz, key_xyz + 1, key_xyz + 2,
                    &viewtype, key_view, key_view + 1, key_view + 2,
                    dummy3, dummy3 + 1, dummy3 + 2,
                    &zzoom, &uselocalspeed);
                }
                if(zzoom<0.25)zzoom = 0.25;
                if(zzoom>4.00)zzoom = 4.0;
                addedframe = AddFrame(thisframe, key_time, key_pause_time, key_xyz, key_view);
                thisframe = addedframe;
                touri->keyframe_times[j] = key_time;
              }
            }
          }
          if(tours_flag == 1){
            for(i = 0; i < scase->tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = scase->tourcoll.tourinfo + i;
              touri->first_frame.next->prev = &touri->first_frame;
              touri->last_frame.prev->next = &touri->last_frame;
            }
            UpdateTourMenuLabels();
            CreateTourPaths();
            UpdateTimes();
            plotstate = GetPlotState(DYNAMIC_PLOTS);
            selectedtour_index = TOURINDEX_MANUAL;
            selected_frame = NULL;
            selected_tour = NULL;
            if(viewalltours == 1)TourMenu(MENU_TOUR_SHOWALL);
          }
          else{
            scase->tourcoll.ntourinfo = 0;
          }
          strcpy(buffer, "1.00000 1.00000 2.0000 0");
          TrimMZeros(buffer);
          continue;
        }
        if(MatchINI(buffer, "TOURINDEX")){
          if(fgets(buffer, 255, stream) == NULL)break;
          sscanf(buffer, "%i", &selectedtour_index_ini);
          if(selectedtour_index_ini < 0)selectedtour_index_ini = -1;
          update_selectedtour_index = 1;
        }
      }
  }
  fclose(stream);
  return 0;
}

/* ------------------ ReadBinIni ------------------------ */

int ReadBinIni(smv_case *scase){
  char *smokeviewini = GetSystemIniPath();
  //*** read in config files if they exist
  // smokeview.ini ini in install directory
  int returnval = 0;
  if(smokeviewini!=NULL){
    returnval = ReadIni2(scase, smokeviewini, 0);
  }
  FREEMEMORY(smokeviewini);
  return returnval;
}

/* ------------------ ReadIni ------------------------ */

int ReadIni(smv_case *scase, char *inifile){
  // There are 7 places to retrieve configuration file from:
  //
  //   1. A file within the same directory as the smokeview executable named
  //      "smokeview.ini".
  //   2. A file in the user's config directory named "smokeview.ini".
  //   3. A file in the current directory named "smokeview.ini".
  //   4. A file in the current directory named "${fdsprefix}.ini".
  //   5. A file in the scratch directory named "${fdsprefix}.ini".
  //   6. A file pointed to by SMOKEVIEW_CONFIG_PATH.
  //   7. A file pointed to be envar SMOKEVIEW_CONFIG.
  //
  // Last definition wins.

  scase->ntickinfo=scase->ntickinfo_smv;

  // Read "smokeview.ini" from bin dir
  char *global_ini = GetSystemIniPath();
  if(global_ini!=NULL){
    int returnval;

    returnval = ReadIni2(scase, global_ini, 0);
    if(returnval==2)return 2;
    if(returnval == 0 && readini_output==1){
      if(verbose_output==1)PRINTF("- complete\n");
    }
    UpdateTerrainOptions();
  }
  FREEMEMORY(global_ini);

  // Read "${fdsprefix}.ini" from the current directory
  if(scase->paths.caseini_filename!=NULL){
    int returnval;
    char localdir[10];

    returnval = ReadIni2(scase, scase->paths.caseini_filename, 1);

    // if directory is not writable then look for another ini file in the scratch directory
    strcpy(localdir, ".");
    if(Writable(localdir)==0){
      // Read "${fdsprefix}.ini" from the scratch directory
      char *scratch_ini_filename = GetUserConfigSubPath(scase->paths.caseini_filename);
      returnval = ReadIni2(scase, scratch_ini_filename, 1);
      FREEMEMORY(scratch_ini_filename);
    }
    if(returnval==2)return 2;
    if(returnval == 0 && readini_output==1){
      if(verbose_output==1)PRINTF("- complete\n");
    }
  }

  // Read file specified in the SSF script
  if(inifile!=NULL){
    int return_code;

    return_code = ReadIni2(scase, inifile,1);
    if(return_code == 0 && readini_output==1){
      if(verbose_output==1)PRINTF("- complete\n");
    }

    if(return_code==1||return_code==2){
      if(inifile==NULL){
        fprintf(stderr,"*** Error: Unable to read .ini file\n");
      }
      else{
        fprintf(stderr,"*** Error: Unable to read %s\n",inifile);
      }
    }
    if(return_code==2)return 2;

    UpdateRGBColors(colorbar_select_index);
  }

  if(use_graphics==1){
    if(showall_textures==1)TextureShowMenu(MENU_TEXTURE_SHOWALL);
  }
  if(colorbars.ncolorbars<=colorbars.ndefaultcolorbars){
    InitDefaultColorbars(&colorbars, 0, show_extreme_mindata, rgb_below_min,
                         show_extreme_maxdata, rgb_above_max, colorbarcopyinfo);
    UpdateColorbarDialogs();
    UpdateCurrentColorbar(colorbars.colorbarinfo + colorbartype);
    update_colorbar_dialog = 0;
  }
  updatezoommenu=1;
  GetSliceParams2();
  return 0;
}

/* ------------------ OutputViewpoints ------------------------ */

void OutputViewpoints(FILE *fileout){
  float *eye, *az_elev, *mat;
  cameradata *ca;

  for(ca = camera_list_first.next; ca->next != NULL; ca = ca->next){
    if(strcmp(ca->name, "external") == 0)continue;
    if(ca->view_id<=1)continue;

    if(ca->quat_defined == 1){
      fprintf(fileout, "VIEWPOINT6\n");
    }
    else{
      fprintf(fileout, "VIEWPOINT5\n");
    }
    eye = ca->eye;
    az_elev = ca->az_elev;
    mat = modelview_identity;

    fprintf(fileout, " %i %i %i\n", ca->rotation_type, ca->rotation_index, ca->view_id);
    fprintf(fileout, " %f %f %f %f %i\n", eye[0], eye[1], eye[2], zoom, zoomindex);
    fprintf(fileout, " %f %f %f %i\n", ca->view_angle, ca->azimuth, ca->elevation, ca->projection_type);
    fprintf(fileout, " %f %f %f\n", ca->xcen, ca->ycen, ca->zcen);

    fprintf(fileout, " %f %f\n", az_elev[0], az_elev[1]);
    if(ca->quat_defined == 1){
      fprintf(fileout, " 1 %f %f %f %f\n", ca->quaternion[0], ca->quaternion[1], ca->quaternion[2], ca->quaternion[3]);
    }
    else{
      fprintf(fileout, " %f %f %f %f\n", mat[0], mat[1], mat[2], mat[3]);
      fprintf(fileout, " %f %f %f %f\n", mat[4], mat[5], mat[6], mat[7]);
      fprintf(fileout, " %f %f %f %f\n", mat[8], mat[9], mat[10], mat[11]);
      fprintf(fileout, " %f %f %f %f\n", mat[12], mat[13], mat[14], mat[15]);
    }
    fprintf(fileout, " %i %i %i %i %i %i %i\n",
      ca->clip_mode,
      ca->clip_xmin, ca->clip_ymin, ca->clip_zmin,
      ca->clip_xmax, ca->clip_ymax, ca->clip_zmax);
    fprintf(fileout, " %f %f %f %f %f %f\n",
      ca->xmin, ca->ymin, ca->zmin,
      ca->xmax, ca->ymax, ca->zmax);
    fprintf(fileout, " %s\n", ca->name);
  }
}

  /* ------------------ WriteIniLocal ------------------------ */


void WriteIniLocal(smv_case *scase, FILE *fileout){
  int i;
  int ndevice_vis = 0;
  sv_object *obj_typei;
  labeldata *thislabel;
  int startup_count;
  scriptfiledata *scriptfile;

  fprintf(fileout, "\n ------------ local ini settings ------------\n\n");

  fprintf(fileout, "BOUNDARYOFFSET\n");
  fprintf(fileout, " %f \n", boundaryoffset);
  fprintf(fileout, "DEVICEVECTORDIMENSIONS\n");
  fprintf(fileout, " %f %f %f %f\n", vector_baselength, vector_basediameter, vector_headlength, vector_headdiameter);
  fprintf(fileout, "DEVICEBOUNDS\n");
  fprintf(fileout, " %f %f\n", device_valmin, device_valmax);
  fprintf(fileout, "DEVICEORIENTATION\n");
  fprintf(fileout, " %i %f\n", show_device_orientation, orientation_scale);
  fprintf(fileout, "GRIDPARMS\n");
  fprintf(fileout, " %i %i %i\n", visx_all, visy_all, visz_all);
  fprintf(fileout, " %i %i %i\n", iplotx_all, iploty_all, iplotz_all);
  fprintf(fileout, "GSLICEPARMS\n");
  fprintf(fileout, " %i %i %i %i\n", vis_gslice_data, show_gslice_triangles, show_gslice_triangulation, show_gslice_normal);
  fprintf(fileout, " %f %f %f\n", gslice_xyz[0], gslice_xyz[1], gslice_xyz[2]);
  fprintf(fileout, " %f %f\n", gslice_normal_azelev[0], gslice_normal_azelev[1]);
  for(thislabel = scase->labelscoll.label_first_ptr->next; thislabel->next != NULL; thislabel = thislabel->next){
    labeldata *labeli;
    float *xyz, *rgbtemp, *tstart_stop;
    int *useforegroundcolor, *show_always;
    float *xyztick, *xyztickdir;
    int *showtick;

    labeli = thislabel;
    if(labeli->labeltype == TYPE_SMV)continue;
    xyz = labeli->xyz;
    rgbtemp = labeli->frgb;
    tstart_stop = labeli->tstart_stop;
    useforegroundcolor = &labeli->useforegroundcolor;
    show_always = &labeli->show_always;
    xyztick = labeli->tick_begin;
    xyztickdir = labeli->tick_direction;
    showtick = &labeli->show_tick;

    fprintf(fileout, "TICKLABEL\n");
    fprintf(fileout, " %f %f %f %f %f %f %f %f %i %i\n",
      xyz[0], xyz[1], xyz[2],
      rgbtemp[0], rgbtemp[1], rgbtemp[2],
      tstart_stop[0], tstart_stop[1],
      *useforegroundcolor, *show_always);
    fprintf(fileout, " %f %f %f %f %f %f %i\n",
      xyztick[0], xyztick[1], xyztick[2],
      xyztickdir[0], xyztickdir[1], xyztickdir[2],
      *showtick);
    fprintf(fileout, " %s\n", labeli->name);
  }
  fprintf(fileout, "LOADFILESATSTARTUP\n");
  fprintf(fileout, " %i\n", loadfiles_at_startup);
  fprintf(fileout, "MSCALE\n");
  fprintf(fileout, " %f %f %f\n", mscale[0], mscale[1], mscale[2]);
  PutStartupSmoke3D(fileout);
  if(npart5prop > 0){
    fprintf(fileout, "PART5PROPDISP\n");
    for(i = 0; i < npart5prop; i++){
      partpropdata *propi;
      int j;

      propi = part5propinfo + i;
      fprintf(fileout, " ");
      for(j = 0; j < scase->npartclassinfo; j++){
        fprintf(fileout, " %i ", (int)propi->class_vis[j]);
      }
      fprintf(fileout, "\n");
    }
    fprintf(fileout, "PART5COLOR\n");
    for(i = 0; i < npart5prop; i++){
      partpropdata *propi;

      propi = part5propinfo + i;
      if(propi->display == 1){
        fprintf(fileout, " %i\n", i);
        break;
      }
    }

  }

  if(scase->npartclassinfo > 0){
    int j;

    fprintf(fileout, "partclassdataVIS\n");
    fprintf(fileout, " %i\n", scase->npartclassinfo);
    for(j = 0; j<scase->npartclassinfo; j++){
      partclassdata *partclassj;

      partclassj = scase->partclassinfo + j;
      fprintf(fileout, " %i\n", partclassj->vis_type);
    }
  }
  fprintf(fileout, "PARTSKIP\n");
  fprintf(fileout, " %i\n", partdrawskip);

  if(scase->propcoll.npropinfo>0){
    fprintf(fileout, "PROPINDEX\n");
    fprintf(fileout, " %i\n", scase->propcoll.npropinfo);
    for(i = 0; i < scase->propcoll.npropinfo; i++){
      propdata *propi;
      int offset;
      int jj;

      propi = scase->propcoll.propinfo + i;
      offset = -1;
      for(jj = 0; jj < propi->nsmokeview_ids; jj++){
        if(strcmp(propi->smokeview_id, propi->smokeview_ids[jj]) == 0){
          offset = jj;
          break;
        }
      }
      fprintf(fileout, " %i %i\n", i, offset);
    }
  }

  for(scriptfile = first_scriptfile.next; scriptfile->next != NULL; scriptfile = scriptfile->next){
    char *file;

    file = scriptfile->file;
    if(file != NULL){
      fprintf(fileout, "SCRIPTFILE\n");
      fprintf(fileout, " %s\n", file);
    }
  }
  fprintf(fileout, "SHOOTER\n");
  fprintf(fileout, " %f %f %f\n", shooter_xyz[0], shooter_xyz[1], shooter_xyz[2]);
  fprintf(fileout, " %f %f %f\n", shooter_dxyz[0], shooter_dxyz[1], shooter_dxyz[2]);
  fprintf(fileout, " %f %f %f\n", shooter_uvw[0], shooter_uvw[1], shooter_uvw[2]);
  fprintf(fileout, " %f %f %f\n", shooter_velmag, shooter_veldir, shooterpointsize);
  fprintf(fileout, " %i %i %i %i %i\n", shooter_fps, shooter_vel_type, shooter_nparts, visShooter, shooter_cont_update);
  fprintf(fileout, " %f %f\n", shooter_duration, shooter_v_inf);

  for(i = 0; i < scase->objectscoll.nobject_defs; i++){
    obj_typei = scase->objectscoll.object_defs[i];
    if(obj_typei->used == 1 && obj_typei->visible == 1){
      ndevice_vis++;
    }
  }
  fprintf(fileout, "SHOWDEVICES\n");
  fprintf(fileout, " %i %i %i\n", ndevice_vis, object_outlines, object_box);
  for(i = 0; i < scase->objectscoll.nobject_defs; i++){
    obj_typei = scase->objectscoll.object_defs[i];
    if(obj_typei->used == 1 && obj_typei->visible == 1){
      fprintf(fileout, " %s\n", obj_typei->label);
    }
  }
  fprintf(fileout, "SHOWDEVICEPLOTS\n");
  fprintf(fileout, " %i %i %i %f %f %f %f %f %f %f\n",
          vis_device_plot, show_plot2d_xlabels, show_plot2d_ylabels, plot2d_size_factor, plot2d_line_width, plot2d_point_size,
          plot2d_xyz_offset[0], plot2d_xyz_offset[1], plot2d_xyz_offset[2], plot2d_font_spacing
  );
  fprintf(fileout, "SHOWGENPLOTXLABEL\n");
  fprintf(fileout, " %i, %f\n", plot2d_show_xaxis_labels, plot2d_xaxis_position);
  fprintf(fileout, "%s\n", plot2d_xaxis_label);

  fprintf(fileout, "SHOWGENPLOTS\n");
  fprintf(fileout, " %i\n", nplot2dinfo);
  fprintf(fileout, " %i %i %i %i %i %i %f %i %i\n",
         plot2d_show_plot_title, plot2d_show_curve_labels, plot2d_show_curve_values,
         plot2d_show_xaxis_bounds, plot2d_show_yaxis_bounds, idevice_add, plot2d_time_average,
         plot2d_show_yaxis_units, plot2d_show_plots);
  for(i=0; i<nplot2dinfo; i++){
    plot2ddata *plot2di;
    int j;

    plot2di = plot2dinfo + i;
    fprintf(fileout, " %s\n", plot2di->plot_label);
    fprintf(fileout, " %f %f %f %i %i %i\n", plot2di->xyz[0], plot2di->xyz[1], plot2di->xyz[2], plot2di->show, plot2di->ncurves, plot2di->mult_devc);
    fprintf(fileout, " %f %i %f %i %f %i %f %i\n",
            plot2di->valmin[0], plot2di->use_valmin[0], plot2di->valmax[0], plot2di->use_valmax[0],
            plot2di->valmin[1], plot2di->use_valmin[1], plot2di->valmax[1], plot2di->use_valmax[1]
            );
    for(j = 0; j < plot2di->ncurves; j++){
      int *color;
      float linewidth1;
      int file_index, col_index;
      curvedata *curve;
      float factor;
      int apply_factor;
      int use_foreground_color;

      file_index        = plot2di->curve[j].csv_file_index;
      col_index         = plot2di->curve[j].csv_col_index;
      curve             = plot2di->curve+j;
      color             = curve->color;
      linewidth1        = curve->linewidth;
      factor            = curve->curve_factor;
      apply_factor      = curve->apply_curve_factor;
      use_foreground_color  = curve->use_foreground_color;
      fprintf(fileout, " %i %i %i %i %i %f %f %i %i\n", file_index, col_index, color[0], color[1], color[2], linewidth1, factor, apply_factor, use_foreground_color);
    };
  }
  fprintf(fileout, "GENPLOTLABELS\n");
  fprintf(fileout, " %i\n", nplot2dinfo);
  for(i = 0; i<nplot2dinfo; i++){
    plot2ddata *plot2di;
    int j;

    plot2di = plot2dinfo+i;
    fprintf(fileout, " %i\n", plot2di->ncurves);
    for(j = 0; j<plot2di->ncurves; j++){
      curvedata *curve;

      curve = plot2di->curve+j;
      fprintf(fileout, " %i %i\n", i, j);
      fprintf(fileout, " %s\n", curve->scaled_label);
      fprintf(fileout, " %s\n", curve->scaled_unit);
    }
  }

  fprintf(fileout, "SHOWDEVICEVALS\n");
  fprintf(fileout, " %i %i %i %i %i %i %i %i %i\n", showdevice_val, showvdevice_val, devicetypes_index, colordevice_val, vectortype, viswindrose, showdevice_type,showdevice_unit,showdevice_id);
  fprintf(fileout, "SHOWHRRPLOT\n");
  fprintf(fileout, " %i %i %f %f %i\n", glui_hrr, hoc_hrr, scase->fuel_hoc, plot2d_size_factor, vis_hrr_plot);
  fprintf(fileout, "SHOWMISSINGOBJECTS\n");
  fprintf(fileout, " %i\n", show_missing_objects);
  fprintf(fileout, "SHOWSLICEPLOT\n");
  fprintf(fileout, " %f %f %f %f %i %i %f %f %f %i %i\n",
                    slice_xyz[0], slice_xyz[1], slice_xyz[2],
                    plot2d_size_factor, vis_slice_plot, slice_plot_bound_option,
                    slice_dxyz[0], slice_dxyz[1], slice_dxyz[2], average_plot2d_slice_region, show_plot2d_slice_position
                    );
  fprintf(fileout, "SMOKE3DCUTOFFS\n");
  fprintf(fileout, " %f %f\n", load_3dsmoke_cutoff, scase->load_hrrpuv_cutoff);
  for(i = scase->ntickinfo_smv; i < scase->ntickinfo; i++){
    float *begt;
    float *endt;
    float *rgbtemp;
    tickdata *ticki;

    ticki = scase->tickinfo + i;
    begt = ticki->begin;
    endt = ticki->end;
    rgbtemp = ticki->rgb;

    fprintf(fileout, "TICKS\n");
    fprintf(fileout, " %f %f %f %f %f %f %i\n", begt[0], begt[1], begt[2], endt[0], endt[1], endt[2], ticki->nbars);
    fprintf(fileout, " %f %i %f %f %f %f\n", ticki->dlength, ticki->dir, rgbtemp[0], rgbtemp[1], rgbtemp[2], ticki->width);
  }
  fprintf(fileout, "SHOWGEOMTERRAIN\n");
  fprintf(fileout, "%i %i %i %i %i\n",
    scase->terrain_texture_coll.nterrain_textures, terrain_show_geometry_surface, terrain_show_geometry_outline, terrain_show_geometry_points, terrain_showonly_top);
  for(i = 0; i<scase->terrain_texture_coll.nterrain_textures; i++){
    texturedata *texti;

    texti = scase->terrain_texture_coll.terrain_textures+i;
    fprintf(fileout, "%i\n", texti->display);
  }

  fprintf(fileout, "TOURCIRCLE\n");
  fprintf(fileout, "%f %f %f %f %f %f %f %f\n",
    tour_circular_center[0],
    tour_circular_center[1], tour_circular_center[2],
    tour_circular_view[0], tour_circular_view[1], tour_circular_view[2],
    tour_circular_radius, tour_circular_angle0);
  fprintf(fileout, "TOURINDEX\n");
  fprintf(fileout, " %i\n", selectedtour_index);
  startup_count = 0;
  for(i = 0; i < scase->tourcoll.ntourinfo; i++){
    tourdata *touri;

    touri = scase->tourcoll.tourinfo + i;
    if(touri->startup == 1)startup_count++;
  }
  if(startup_count < scase->tourcoll.ntourinfo){
    //TOUR7
    // index
    //   tourlabel
    //   nkeyframes avatar_index display
    //      time pause_time x y z
    //      vx vy vz
    fprintf(fileout, "TOUR7\n");
    fprintf(fileout, " %i\n", scase->tourcoll.ntourinfo - startup_count);
    for(i = 0; i < scase->tourcoll.ntourinfo; i++){
      tourdata *touri;
      keyframe *framei;
      int j;

      touri =scase->tourcoll.tourinfo + i;
      if(touri->startup == 1)continue;

      TrimBack(touri->label);
      fprintf(fileout, "  %s\n", touri->label);
      fprintf(fileout, "  %i %i %i\n", touri->nkeyframes, touri->glui_avatar_index, touri->display);

      framei = &touri->first_frame;
      for(j = 0; j<touri->nkeyframes; j++){
        float xyz_smv[3], view_smv[3];

        framei = framei->next;
        SMV2FDS_XYZ(xyz_smv, framei->xyz_smv);
        fprintf(fileout, "    %f %f %f %f %f\n", framei->time, framei->pause_time, xyz_smv[0], xyz_smv[1], xyz_smv[2]);

        SMV2FDS_XYZ(view_smv, framei->view_smv);
        fprintf(fileout, "    %f %f %f\n", view_smv[0], view_smv[1], view_smv[2]);
      }
    }
  }
  fprintf(fileout, "USERTICKS\n");
  fprintf(fileout, " %i %i %i %i %i %i %f %i\n", visUSERticks, auto_user_tick_placement, user_tick_sub,
    user_tick_show_x, user_tick_show_y, user_tick_show_z, user_tick_direction, ntick_decimals);
  fprintf(fileout, " %f %f %f\n", user_tick_origin[0], user_tick_origin[1], user_tick_origin[2]);
  fprintf(fileout, " %f %f %f\n", user_tick_min[0], user_tick_min[1], user_tick_min[2]);
  fprintf(fileout, " %f %f %f\n", user_tick_max[0], user_tick_max[1], user_tick_max[2]);
  fprintf(fileout, " %f %f %f\n", user_tick_step[0], user_tick_step[1], user_tick_step[2]);
  fprintf(fileout, " %i %i %i\n", user_tick_show_x, user_tick_show_y, user_tick_show_z);
  fprintf(fileout, "XYZCLIP\n");
  fprintf(fileout, " %i\n", clip_mode);
  fprintf(fileout, " %i %f %i %f\n", clipinfo.clip_xmin, clipinfo.xmin, clipinfo.clip_xmax, clipinfo.xmax);
  fprintf(fileout, " %i %f %i %f\n", clipinfo.clip_ymin, clipinfo.ymin, clipinfo.clip_ymax, clipinfo.ymax);
  fprintf(fileout, " %i %f %i %f\n", clipinfo.clip_zmin, clipinfo.zmin, clipinfo.clip_zmax, clipinfo.zmax);

  fprintf(fileout, "\n *** TIME/DATA BOUNDS ***\n");
  fprintf(fileout, "  (0/1 min max skip (1=set, 0=unset)\n\n");

  for(i = 0; i<npatchbounds_cpp; i++){
    cpp_boundsdata *boundi;

    boundi = patchbounds_cpp+i;
    fprintf(fileout, "C_BOUNDARY\n");
    fprintf(fileout, " %i %f %i %f %s\n", boundi->set_chopmin, boundi->chopmin, boundi->set_chopmax, boundi->chopmax, boundi->label);
  }
  if(niso_bounds > 0){
    for(i = 0; i < niso_bounds; i++){
      fprintf(fileout, "C_ISO\n");
      fprintf(fileout, " %i %f %i %f %s\n",
        isobounds[i].setchopmin, isobounds[i].chopmin,
        isobounds[i].setchopmax, isobounds[i].chopmax,
        isobounds[i].label->shortlabel
        );
    }
  }
#ifdef pp_PARTBOUND_MULTI
  THREADcontrol(partbound_threads, THREAD_JOIN);
#endif
  for(i = 0; i<npartbounds_cpp; i++){
    cpp_boundsdata *boundi;

    boundi = partbounds_cpp+i;
    fprintf(fileout, "C_PARTICLES\n");
    fprintf(fileout, " %i %f %i %f %s\n", boundi->set_chopmin, boundi->chopmin, boundi->set_chopmax, boundi->chopmax, boundi->label);
  }

  fprintf(fileout, "C_PLOT3D\n");
  fprintf(fileout, " %i\n", nplot3dbounds_cpp);
  for(i = 0; i<nplot3dbounds_cpp; i++){
    cpp_boundsdata *boundi;

    boundi = plot3dbounds_cpp+i;
    fprintf(fileout, " %i %i %f %i %f\n", i+1, boundi->set_chopmin, boundi->chopmin, boundi->set_chopmax, boundi->chopmax);
  }

  if(nslicebounds_cpp>0){
    for(i = 0; i<nslicebounds_cpp; i++){
      cpp_boundsdata *boundi;

      boundi = slicebounds_cpp+i;
      fprintf(fileout, "C_SLICE\n");
      fprintf(fileout, " %i %f %i %f %s\n", boundi->set_chopmin, boundi->chopmin, boundi->set_chopmax, boundi->chopmax, boundi->label);
      fprintf(fileout, "C_SLICE_HIDE\n");
      fprintf(fileout, " %i %s\n", boundi->chop_hide, boundi->label);
    }
  }
  fprintf(fileout, "CACHE_DATA\n");
  fprintf(fileout, " %i %i %i %i \n", cache_boundary_data, cache_part_data, cache_plot3d_data, cache_slice_data);
  fprintf(fileout, "LOADMESH\n");
  fprintf(fileout, " %i %i\n", show_intersection_box, show_intersected_meshes);
  fprintf(fileout, " %i %f %i %f\n", use_meshclip[0], meshclip[0], use_meshclip[1], meshclip[1]);
  fprintf(fileout, " %i %f %i %f\n", use_meshclip[2], meshclip[2], use_meshclip[3], meshclip[3]);
  fprintf(fileout, " %i %f %i %f\n", use_meshclip[4], meshclip[4], use_meshclip[5], meshclip[5]);
  fprintf(fileout, "PATCHDATAOUT\n");
  fprintf(fileout, " %i %f %f %f %f %f %f %f %f\n", output_patchdata,
    patchout_tmin, patchout_tmax,
    patchout_xmin, patchout_xmax,
    patchout_ymin, patchout_ymax,
    patchout_zmin, patchout_zmax
    );
  fprintf(fileout, "TIMEOFFSET\n");
  fprintf(fileout, " %f\n", timeoffset);
  fprintf(fileout, "TLOAD\n");
  fprintf(fileout, " %i %f %i %f %i %i\n", use_tload_begin, scase->tload_begin, use_tload_end, scase->tload_end, use_tload_skip, tload_skip);
  for(i = 0; i < scase->npatchinfo; i++){
    patchdata *patchi;

    patchi = scase->patchinfo + i;
    if(patchi->firstshort_patch == 1){
      int set_valmin=0, set_valmax=0;
      float valmin=1.0, valmax=0.0;
      char *label;

      label = patchi->label.shortlabel;

      GLUIGetOnlyMinMax(BOUND_PATCH, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, "V2_BOUNDARY\n");
      fprintf(fileout, " %i %f %i %f %s\n", set_valmin, valmin, set_valmax, valmax, label);
    }
  }
  if(niso_bounds > 0){
    for(i = 0; i < niso_bounds; i++){
      fprintf(fileout, "V_ISO\n");
      fprintf(fileout, " %i %f %i %f %s\n",
        isobounds[i].dlg_setvalmin, isobounds[i].dlg_valmin,
        isobounds[i].dlg_setvalmax, isobounds[i].dlg_valmax,
        isobounds[i].label->shortlabel
        );
    }
  }
  if(npart5prop > 0){
    for(i = 0; i < npart5prop; i++){
      partpropdata *propi;

      propi = part5propinfo + i;
      fprintf(fileout, "V2_PARTICLES\n");
      int set_valmin=0, set_valmax=0;
      float valmin=1.0, valmax=0.0;
      char *label;

      label = propi->label->shortlabel;

      GLUIGetOnlyMinMax(BOUND_PART, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %f %i %f %s\n", set_valmin, valmin, set_valmax, valmax, label);
    }
  }
  {
    int n3d;

    n3d = MAXPLOT3DVARS;
    if(n3d<numplot3dvars)n3d = numplot3dvars;
    if(n3d>MAXPLOT3DVARS)n3d = MAXPLOT3DVARS;
    if(scase->plot3dinfo!=NULL){
      fprintf(fileout, "V2_PLOT3D\n");
      fprintf(fileout, " %i\n", n3d);
    }
    for(i = 0; i < n3d; i++){
    if(scase->plot3dinfo!=NULL){
      int set_valmin=0, set_valmax=0;
      float valmin=1.0, valmax=0.0;
      char *label;

      label = scase->plot3dinfo[0].label[i].shortlabel;
      GLUIGetOnlyMinMax(BOUND_PLOT3D, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %i %f %i %f %s\n", i+1, set_valmin, valmin, set_valmax, valmax, label);
    }
    }
  }
  if(nhvacductbounds > 0){
    for(i = 0; i < nhvacductbounds; i++){
      fprintf(fileout, "V2_HVACDUCT\n");
      int set_valmin = 0, set_valmax = 0;
      float valmin = 1.0, valmax = 0.0;
      char *label;

      label = hvacductbounds[i].label->shortlabel;
      GLUIGetOnlyMinMax(BOUND_HVACDUCT, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %f %i %f %s\n", set_valmin, valmin, set_valmax, valmax, label);
    }
  }
  if(nhvacnodebounds > 0){
    for(i = 0; i < nhvacnodebounds; i++){
      fprintf(fileout, "V2_HVACNODE\n");
      int set_valmin = 0, set_valmax = 0;
      float valmin = 1.0, valmax = 0.0;
      char *label;

      label = hvacnodebounds[i].label->shortlabel;
      GLUIGetOnlyMinMax(BOUND_HVACNODE, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %f %i %f %s\n", set_valmin, valmin, set_valmax, valmax, label);
    }
  }
  if(nslicebounds > 0){
    for(i = 0; i < nslicebounds; i++){
      fprintf(fileout, "V2_SLICE\n");
      int set_valmin=0, set_valmax=0;
      float valmin=1.0, valmax=0.0;
      char *label;

      label = slicebounds[i].label->shortlabel;
      GLUIGetOnlyMinMax(BOUND_SLICE, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %f %i %f %s : %f %f %i\n", set_valmin, valmin, set_valmax, valmax, label,
        slicebounds[i].line_contour_min, slicebounds[i].line_contour_max, slicebounds[i].line_contour_num
        );
    }
  }
  fprintf(fileout, "V_TARGET\n");
  fprintf(fileout, " %i %f %i %f\n", settargetmin, targetmin, settargetmax, targetmax);
  if(scase->nzoneinfo > 0){
    fprintf(fileout, "V_ZONE\n");
    fprintf(fileout, " %i %f %i %f\n", setzonemin, zoneusermin, setzonemax, zoneusermax);
    fprintf(fileout, "ZONEVIEW\n");
    fprintf(fileout, " %f\n", zone_hvac_diam);
  }
}

  /* ------------------ WriteIni ------------------------ */

void WriteIni(smv_case *scase, int flag,char *filename){
  FILE *fileout=NULL;
  int i;
  char *outfilename=NULL, *outfiledir=NULL;
  char *smokeviewini_filename = GetSystemIniPath();
  char *smokeview_scratchdir = GetUserConfigDir();

  switch(flag){
  case GLOBAL_INI:
    if(smokeviewini_filename!=NULL)fileout=fopen(smokeviewini_filename,"w");
    outfilename= smokeviewini_filename;
    break;
  case STDOUT_INI:
    fileout=stdout;
    break;
  case SCRIPT_INI:
    fileout=fopen(filename,"w");
    outfilename=filename;
    break;
  case LOCAL_INI:
    fileout=fopen(scase->paths.caseini_filename,"w");
    if(fileout==NULL&&smokeview_scratchdir!=NULL){
      fileout = fopen_indir(smokeview_scratchdir, scase->paths.caseini_filename, "w");
      outfiledir = smokeview_scratchdir;
     }
    outfilename=scase->paths.caseini_filename;
    break;
  default:
    assert(FFALSE);
    break;
  }
  if(flag==SCRIPT_INI)flag=LOCAL_INI;
  if(fileout==NULL){
    if(outfilename!=NULL){
      fprintf(stderr,"*** Error: unable to open %s for writing ",outfilename);
      return;
    }
    else{
      fprintf(stderr,"*** Error: unable to open ini file for output ");
    }
    if(outfiledir==NULL){
      printf("in current directory\n");
    }
    else{
      printf("in directory %s\n", outfiledir);
    }
  }
  FREEMEMORY(smokeviewini_filename);

  fprintf(fileout,"# NIST Smokeview configuration file, Release %s\n\n",__DATE__);
  fprintf(fileout, "\n ------------ global ini settings ------------\n\n");

  fprintf(fileout,"   *** COLOR/LIGHTING ***\n\n");

  fprintf(fileout, "BACKGROUNDCOLOR\n");
  fprintf(fileout, " %f %f %f\n", backgroundbasecolor[0], backgroundbasecolor[1], backgroundbasecolor[2]);
  fprintf(fileout, "BLOCKCOLOR\n");
  fprintf(fileout, " %f %f %f\n", block_ambient2[0], block_ambient2[1], block_ambient2[2]);
  fprintf(fileout, "BLOCKSHININESS\n");
  fprintf(fileout, " %f\n", block_shininess);
  fprintf(fileout, "BLOCKSPECULAR\n");
  fprintf(fileout, " %f %f %f\n", block_specular2[0], block_specular2[1], block_specular2[2]);
  fprintf(fileout, "BOUNDCOLOR\n");
  fprintf(fileout, " %f %f %f\n", boundcolor[0], boundcolor[1], boundcolor[2]);
  fprintf(fileout, "COLORBAR\n");
  {
    int usetexturebar = 1; //for older smokeviews
    fprintf(fileout, " %i %i %i %i\n", scase->nrgb, usetexturebar, colorbar_select_index, colorbar_selection_width);
  }
  for(i=0;i<scase->nrgb;i++){
    fprintf(fileout," %f %f %f\n",scase->rgb[i][0],scase->rgb[i][1],scase->rgb[i][2]);
  }
  fprintf(fileout,"COLOR2BAR\n");
  fprintf(fileout," %i\n",8);
  fprintf(fileout," %f %f %f :white  \n",rgb2[0][0],rgb2[0][1],rgb2[0][2]);
  fprintf(fileout," %f %f %f :yellow \n",rgb2[1][0],rgb2[1][1],rgb2[1][2]);
  fprintf(fileout," %f %f %f :blue   \n",rgb2[2][0],rgb2[2][1],rgb2[2][2]);
  fprintf(fileout," %f %f %f :red    \n",rgb2[3][0],rgb2[3][1],rgb2[3][2]);
  fprintf(fileout," %f %f %f :green  \n",rgb2[4][0],rgb2[4][1],rgb2[4][2]);
  fprintf(fileout," %f %f %f :magenta\n",rgb2[5][0],rgb2[5][1],rgb2[5][2]);
  fprintf(fileout," %f %f %f :cyan   \n",rgb2[6][0],rgb2[6][1],rgb2[6][2]);
  fprintf(fileout," %f %f %f :black  \n",rgb2[7][0],rgb2[7][1],rgb2[7][2]);
  fprintf(fileout, "COLORBAR_FLIP\n");
  fprintf(fileout, " %i %i\n", colorbar_flip,colorbar_autoflip);
  fprintf(fileout, "COLORBAR_SPLIT\n");
  fprintf(fileout, " %i %i %i %i %i %i\n", colorsplit[0], colorsplit[1], colorsplit[2], colorsplit[3], colorsplit[4], colorsplit[5]);
  fprintf(fileout, " %i %i %i %i %i %i\n", colorsplit[6], colorsplit[7], colorsplit[8], colorsplit[9], colorsplit[10], colorsplit[11]);
  fprintf(fileout, " %f %f %f\n", splitvals[0], splitvals[1], splitvals[2]);
  fprintf(fileout,"CO2COLOR\n");
  fprintf(fileout," %i %i %i\n", co2_color_int255[0],co2_color_int255[1],co2_color_int255[2]);
  fprintf(fileout, "DIRECTIONCOLOR\n");
  fprintf(fileout, " %f %f %f\n", direction_color[0], direction_color[1], direction_color[2]);
  fprintf(fileout, "FLIP\n");
  fprintf(fileout, " %i\n", background_flip);
  fprintf(fileout, "FOREGROUNDCOLOR\n");
  fprintf(fileout, " %f %f %f\n", foregroundbasecolor[0], foregroundbasecolor[1], foregroundbasecolor[2]);
  fprintf(fileout, "GEOMSELECTCOLOR\n");
  fprintf(fileout, " %u %u %u\n",  geom_vertex1_rgb[0],  geom_vertex1_rgb[1],  geom_vertex1_rgb[2]);
  fprintf(fileout, " %u %u %u\n",  geom_vertex2_rgb[0],  geom_vertex2_rgb[1],  geom_vertex2_rgb[2]);
  fprintf(fileout, " %u %u %u\n", geom_triangle_rgb[0], geom_triangle_rgb[1], geom_triangle_rgb[2]);
  fprintf(fileout, "HEATOFFCOLOR\n");
  fprintf(fileout, " %f %f %f\n", heatoffcolor[0], heatoffcolor[1], heatoffcolor[2]);
  fprintf(fileout, "HEATONCOLOR\n");
  fprintf(fileout, " %f %f %f\n", heatoncolor[0], heatoncolor[1], heatoncolor[2]);
  fprintf(fileout, "ISOCOLORS\n");
  fprintf(fileout," %f %f %i %i : shininess, default opaqueness\n",iso_shininess, iso_transparency, iso_transparency_option, iso_opacity_change);
  fprintf(fileout," %f %f %f : specular\n",iso_specular[0],iso_specular[1],iso_specular[2]);
  fprintf(fileout," %i : number of levels\n",MAX_ISO_COLORS);
  for(i=0;i<MAX_ISO_COLORS;i++){
    fprintf(fileout, " %f %f %f %f", iso_colors[4*i], iso_colors[4*i+1], iso_colors[4*i+2], iso_colors[4*i+3]);
    if(i==0)fprintf(fileout, " : red, green, blue, alpha (opaqueness)");
    fprintf(fileout, "\n");
  }
  if(ncolortableinfo>0){
    char percen[2];

    strcpy(percen,"%");
    fprintf(fileout, "COLORTABLE\n");
    fprintf(fileout, " %i \n", ncolortableinfo);

    for(i = 0; i<ncolortableinfo; i++){
      colortabledata *rgbi;

      rgbi = colortableinfo+i;
      fprintf(fileout, " %i %i %i %i %s %s\n",
        rgbi->color[0], rgbi->color[1], rgbi->color[2], rgbi->color[3], percen, rgbi->label);
    }
  }
  fprintf(fileout, "LIGHTING\n");
  fprintf(fileout, " %i\n", use_lighting);
  fprintf(fileout, "LIGHTFACES\n");
  fprintf(fileout, " %i\n", light_faces);
  fprintf(fileout, "LIGHTANGLES0\n");
  fprintf(fileout, " %f %f %i\n", glui_light_az0, glui_light_elev0, use_light0);
  fprintf(fileout, "LIGHTANGLES1\n");
  fprintf(fileout, " %f %f %i\n", glui_light_az1, glui_light_elev1, use_light1);
  fprintf(fileout, "LIGHTPROP\n");
  fprintf(fileout, " %f %f %f\n", ambientlight[0], ambientlight[1], ambientlight[2]);
  fprintf(fileout, " %f %f %f\n", diffuselight[0], diffuselight[1], diffuselight[2]);
  fprintf(fileout, "OUTLINECOLOR\n");
  fprintf(fileout, " %i %i %i\n", glui_outlinecolor[0], glui_outlinecolor[1], glui_outlinecolor[2]);
  fprintf(fileout, "SENSORCOLOR\n");
  fprintf(fileout, " %f %f %f\n", sensorcolor[0], sensorcolor[1], sensorcolor[2]);
  fprintf(fileout, "SENSORNORMCOLOR\n");
  fprintf(fileout, " %f %f %f\n", sensornormcolor[0], sensornormcolor[1], sensornormcolor[2]);
  fprintf(fileout, "SETBW\n");
  fprintf(fileout, " %i %i\n", setbw,setbwdata);
  fprintf(fileout, "SPRINKOFFCOLOR\n");
  fprintf(fileout, " %f %f %f\n", sprinkoffcolor[0], sprinkoffcolor[1], sprinkoffcolor[2]);
  fprintf(fileout, "SPRINKONCOLOR\n");
  fprintf(fileout, " %f %f %f\n", sprinkoncolor[0], sprinkoncolor[1], sprinkoncolor[2]);
  fprintf(fileout, "STATICPARTCOLOR\n");
  fprintf(fileout, " %f %f %f\n", static_color[0], static_color[1], static_color[2]);
  {
    int scount;

    scount = 0;
    for(i = 0; i<scase->surfcoll.nsurfinfo; i++){
      surfdata *surfi;

      surfi = scase->surfcoll.surfinfo+scase->surfcoll.sorted_surfidlist[i];
      if(surfi->used_by_geom==1)scount++;
    }
    if(scount>0){
      fprintf(fileout, "SURFCOLORS\n");
      fprintf(fileout, " %i %i\n", scount, use_surf_color);
      for(i = 0; i<scase->surfcoll.nsurfinfo; i++){
        surfdata *surfi;

        surfi = scase->surfcoll.surfinfo+scase->surfcoll.sorted_surfidlist[i];
        if(surfi->used_by_geom==1){
          int *ini_surf_color;

          ini_surf_color = surfi->geom_surf_color;
          fprintf(fileout, " %i %i %i : %s\n", ini_surf_color[0], ini_surf_color[1], ini_surf_color[2], surfi->surfacelabel);
        }
      }
    }
  }
  {
    int scount;

    scount = 0;
    for(i = 0; i<scase->surfcoll.nsurfinfo; i++){
      surfdata *surfi;

      surfi = scase->surfcoll.surfinfo+scase->surfcoll.sorted_surfidlist[i];
      if(surfi->in_color_dialog==1&&surfi->color!=surfi->color_orig)scount++;
    }
    if(scount>0){
      fprintf(fileout, "OBSTSURFCOLORS\n");
      fprintf(fileout, " %i %i\n", scount, use_surf_color);
      for(i = 0; i<scase->surfcoll.nsurfinfo; i++){
        surfdata *surfi;

        surfi = scase->surfcoll.surfinfo+scase->surfcoll.sorted_surfidlist[i];
        if(surfi->in_color_dialog==1&&surfi->color!=surfi->color_orig){
          float *color;

          color = surfi->color;
          fprintf(fileout, " %f %f %f %f: %s\n", color[0], color[1], color[2], surfi->transparent_level, surfi->surfacelabel);
        }
      }
    }
  }
  fprintf(fileout, "TIMEBARCOLOR\n");
  fprintf(fileout, " %f %f %f\n", timebarcolor[0], timebarcolor[1], timebarcolor[2]);
  fprintf(fileout, "VENTCOLOR\n");
  fprintf(fileout," %f %f %f\n",ventcolor[0],ventcolor[1],ventcolor[2]);

  fprintf(fileout, "\n   *** SIZES/OFFSETS ***\n\n");

  fprintf(fileout, "GEOMSAXIS\n");
  fprintf(fileout, " %f %f\n",  glui_surf_axis_length, glui_surf_axis_width);
  fprintf(fileout, "GRIDLINEWIDTH\n");
  fprintf(fileout, " %f\n", gridlinewidth);
  fprintf(fileout, "ISOLINEWIDTH\n");
  fprintf(fileout, " %f\n", isolinewidth);
  fprintf(fileout, "ISOPOINTSIZE\n");
  fprintf(fileout, " %f\n", isopointsize);
  fprintf(fileout, "LINEWIDTH\n");
  fprintf(fileout, " %f\n", scase->linewidth);
  fprintf(fileout, "PARTPOINTSIZE\n");
  fprintf(fileout, " %f\n", partpointsize);
  fprintf(fileout, "PLOT3DLINEWIDTH\n");
  fprintf(fileout, " %f\n", plot3dlinewidth);
  fprintf(fileout, "PLOT3DPOINTSIZE\n");
  fprintf(fileout, " %f\n", plot3dpointsize);
  fprintf(fileout, "SENSORABSSIZE\n");
  fprintf(fileout, " %f\n", sensorabssize);
  fprintf(fileout, "SENSORRELSIZE\n");
  fprintf(fileout, " %f\n", sensorrelsize);
  fprintf(fileout, "SLICEOFFSET\n");
  fprintf(fileout, " %f %f %i\n", sliceoffset_factor,slice_dz, agl_offset_actual);
  fprintf(fileout, "SMOOTHLINES\n");
  fprintf(fileout, " %i\n", antialiasflag);
  fprintf(fileout, "SPHERESEGS\n");
  fprintf(fileout, " %i\n", device_sphere_segments);
  fprintf(fileout, "SORTSLICES\n");
  fprintf(fileout, " %i\n", sortslices);
  fprintf(fileout, "SPRINKLERABSSIZE\n");
  fprintf(fileout, " %f\n", sprinklerabssize);
  fprintf(fileout, "STREAKLINEWIDTH\n");
  fprintf(fileout, " %f\n", streaklinewidth);
  fprintf(fileout, "TICKLINEWIDTH\n");
  fprintf(fileout, " %f\n", ticklinewidth);
  fprintf(fileout, "NEWDRAWFACE\n");
  fprintf(fileout, " %i\n", blockage_draw_option);
  fprintf(fileout, "VECCONTOURS\n");
  fprintf(fileout, " %i %i\n", show_node_slices_and_vectors,show_cell_slices_and_vectors);
  fprintf(fileout, "VECLENGTH\n");
  fprintf(fileout, " %i %f %f %i %i %i\n", 4, vecfactor, 1.0, vec_uniform_length, vec_uniform_spacing, color_vector_black);
  fprintf(fileout, "VECTORLINEWIDTH\n");
  fprintf(fileout, " %f %f\n", vectorlinewidth, slice_line_contour_width);
  fprintf(fileout, "VECTORPOINTSIZE\n");
  fprintf(fileout," %f\n",vectorpointsize);
  fprintf(fileout, "VENTLINEWIDTH\n");
  fprintf(fileout, " %f\n", scase->ventlinewidth);
  fprintf(fileout, "VENTOFFSET\n");
  fprintf(fileout, " %f\n", ventoffset_factor);
  fprintf(fileout, "WINDOWOFFSET\n");
  fprintf(fileout, " %i\n", titlesafe_offsetBASE);
  if(use_graphics == 1 &&
     (screenWidth == GLUTGetScreenWidth()||screenHeight == GLUTGetScreenHeight())
    ){
    fprintf(fileout,"WINDOWWIDTH\n");
    fprintf(fileout," %i\n",-1);
    fprintf(fileout,"WINDOWHEIGHT\n");
    fprintf(fileout," %i\n",-1);
  }
  else{

#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      fprintf(fileout,"WINDOWWIDTH\n");
      fprintf(fileout," %i\n",screenWidth/2);
      fprintf(fileout,"WINDOWHEIGHT\n");
      fprintf(fileout," %i\n",screenHeight/2);
    }
    else{
      fprintf(fileout,"WINDOWWIDTH\n");
      fprintf(fileout," %i\n",screenWidth);
      fprintf(fileout,"WINDOWHEIGHT\n");
      fprintf(fileout," %i\n",screenHeight);
    }
#else
    fprintf(fileout,"WINDOWWIDTH\n");
    fprintf(fileout," %i\n",screenWidth);
    fprintf(fileout,"WINDOWHEIGHT\n");
    fprintf(fileout," %i\n",screenHeight);
#endif
  }

  fprintf(fileout, "\n *** DATA LOADING ***\n\n");

  fprintf(fileout, "CSV\n");
  fprintf(fileout, " %i\n", csv_loaded);
#ifdef pp_FRAME
  fprintf(fileout, "FRAMETHREADS\n");
  fprintf(fileout, " %i\n", nframe_threads);
#endif
  fprintf(fileout, "LOADINC\n");
  fprintf(fileout, " %i\n", load_incremental);
  fprintf(fileout, "NOPART\n");
  fprintf(fileout, " %i\n", nopart);
  fprintf(fileout, "PARTFAST\n");
  fprintf(fileout, " %i %i %i\n", partfast, use_partload_threads, n_partload_threads);
  fprintf(fileout, "RESEARCHMODE\n");
  fprintf(fileout, " %i %i %f %i %i %i %i %i %i %i\n", research_mode, 1, colorbar_shift, ncolorlabel_digits, force_fixedpoint, ngridloc_digits, sliceval_ndigits, force_exponential, force_decimal, force_zero_pad);
  fprintf(fileout, "SLICEAVERAGE\n");
  fprintf(fileout, " %i %f %i\n", slice_average_flag, slice_average_interval, vis_slice_average);
  fprintf(fileout, "SLICEDATAOUT\n");
  fprintf(fileout, " %i \n", output_slicedata);
  fprintf(fileout, "USER_ROTATE\n");
  fprintf(fileout, " %i %i %f %f %f\n", glui_rotation_index, show_rotation_center, xcenCUSTOM, ycenCUSTOM, zcenCUSTOM);
  fprintf(fileout, "ZIPSTEP\n");
  fprintf(fileout, " %i\n", tload_zipstep);

  fprintf(fileout,"\n *** VIEW PARAMETERS ***\n\n");

  GLUIGetGeomDialogState();
  fprintf(fileout, "APERTURE\n");
  fprintf(fileout, " %i\n", apertureindex);
  fprintf(fileout, "BLOCKLOCATION\n");
  fprintf(fileout, " %i\n", blocklocation);
  fprintf(fileout, "BEAM\n");
  fprintf(fileout, " %i %f %i %i %i %i\n", showbeam_as_line,beam_line_width,use_beamcolor,beam_color[0], beam_color[1], beam_color[2]);
  fprintf(fileout, "BLENDMODE\n");
  fprintf(fileout, " %i %i %i\n", slices3d_max_blending, hrrpuv_max_blending,showall_3dslices);
  fprintf(fileout, "BOUNDARYTWOSIDE\n");
  fprintf(fileout, " %i\n", showpatch_both);
  fprintf(fileout, "CLIP\n");
  fprintf(fileout, " %f %f\n", nearclip, farclip);
  fprintf(fileout, "CONTOURTYPE\n");
  fprintf(fileout, " %i %i\n", contour_type, colorbar_linewidth);
  fprintf(fileout, "CULLFACES\n");
  fprintf(fileout, " %i\n", cullfaces);
  fprintf(fileout, "ENABLETEXTURELIGHTING\n");
  fprintf(fileout, " %i\n", enable_texture_lighting);
  fprintf(fileout, "EYEVIEW\n");
  fprintf(fileout, " %i\n", rotation_type);
  fprintf(fileout, "FONTSIZE\n");
  fprintf(fileout, " %i\n", fontindex);
  fprintf(fileout, "FIREPARAMS\n");
  fprintf(fileout, "FRAMERATEVALUE\n");
  fprintf(fileout, " %i\n", frameratevalue);
  fprintf(fileout, "FREEZEVOLSMOKE\n");
  fprintf(fileout, " %i %i\n", freeze_volsmoke, autofreeze_volsmoke);
  fprintf(fileout, "GEOMBOUNDARYPROPS\n");
  fprintf(fileout, " %i %i %i %f %f %i\n",show_boundary_shaded, show_boundary_outline, show_boundary_points, geomboundary_linewidth, geomboundary_pointsize, boundary_edgetype);
  if(hvaccoll.nhvacinfo > 0){
    fprintf(fileout, "HVACVIEW\n");
    fprintf(fileout, " %i %i %i %i %f %i\n", hvaccoll.nhvacinfo, hvac_metro_view, 1, 0, 0.0, hvac_cell_view);
    for(i = 0; i < hvaccoll.nhvacinfo; i++){
      hvacdata *hvaci;
      int *dc, *nc;

      hvaci = hvaccoll.hvacinfo + i;
      dc = hvaci->duct_color;
      nc = hvaci->node_color;
      fprintf(fileout, " %i %i %i %i %i %f %f %f %f %f\n",
        hvaci->display, hvaci->show_node_labels, hvaci->show_duct_labels, hvaci->show_component,
        hvaci->show_filters, hvaci->duct_width, hvaci->node_size, hvaci->component_size, hvaci->filter_size, hvaci->cell_node_size);
      fprintf(fileout, " %i %i %i %i %i %i\n", dc[0], dc[1], dc[2], nc[0], nc[1], nc[2]);
    }
  }
  fprintf(fileout, "SHOWSLICEVALS\n");
  fprintf(fileout, " %i %i %i\n", show_slice_values[0], show_slice_values[1], show_slice_values[2]);
  fprintf(fileout, "GEOMCELLPROPS\n");
  fprintf(fileout, " %i\n",
    slice_celltype);
  fprintf(fileout, " %i %i %i\n",
    slice_edgetypes[0], slice_edgetypes[1], slice_edgetypes[2]);
  fprintf(fileout, " %i %i %i %f %i %i %i\n",
    show_slice_shaded[0], show_slice_shaded[1], show_slice_shaded[2], 5.0,
    show_vector_slice[0], show_vector_slice[1], show_vector_slice[2]);
  fprintf(fileout, " %i %i %i\n",
    show_slice_outlines[0], show_slice_outlines[1], show_slice_outlines[2]);
  fprintf(fileout, " %i %i %i\n",
    show_slice_points[0], show_slice_points[1], show_slice_points[2]);
  fprintf(fileout, "GEOMDOMAIN\n");
  fprintf(fileout, " %i %i\n", showgeom_inside_domain, showgeom_outside_domain);
  fprintf(fileout, "GEOMSHOW\n");
  fprintf(fileout, " %i %i %i %i %i %i %f %f %i %i %f %f %f\n",
     0, 1, show_faces_shaded, show_faces_outline, smooth_geom_normal,
     geom_force_transparent, geom_transparency, geom_linewidth, use_geom_factors, show_cface_normals, geom_pointsize, geom_dz_offset, geom_norm_offset);
  fprintf(fileout, " %i %i %i %i\n", 0, 0, 0, 0);
  fprintf(fileout, " %f %f %i %i %i %i\n", geom_vert_exag, 30.0, 0, 0, show_geom_boundingbox, show_geom_bndf);

  fprintf(fileout, "GVERSION\n");
  fprintf(fileout, " %i\n", vis_title_gversion);
  fprintf(fileout, "GVECDOWN\n");
  fprintf(fileout, " %i\n", gvec_down);
  fprintf(fileout, "ISOTRAN2\n");
  fprintf(fileout, " %i\n", transparent_state);
  for(i = 0; i < scase->meshescoll.nmeshes; i++){
    meshdata *meshi;

    meshi = scase->meshescoll.meshinfo + i;
    if(meshi->mesh_offset_ptr != NULL){
      fprintf(fileout, "MESHOFFSET\n");
      fprintf(fileout, " %i\n", i);
    }
  }
  if(scase->meshescoll.nmeshes>1){
    fprintf(fileout,"MESHVIS\n");
    fprintf(fileout," %i\n",scase->meshescoll.nmeshes);

    for(i=0;i<scase->meshescoll.nmeshes;i++){
      meshdata *meshi;

      meshi = scase->meshescoll.meshinfo + i;
      fprintf(fileout," %i\n",meshi->blockvis);
    }
  }
  fprintf(fileout, "NORTHANGLE\n");
  fprintf(fileout, " %i\n", vis_northangle);
  fprintf(fileout, " %f %f %f\n", northangle_position[0], northangle_position[1], northangle_position[2]);
  fprintf(fileout, "OFFSETSLICE\n");
  fprintf(fileout, " %i\n", offset_slice);
  fprintf(fileout, "OUTLINEMODE\n");
  fprintf(fileout, " %i %i\n", highlight_flag, outline_color_flag);
  fprintf(fileout, "P3DSURFACETYPE\n");
  fprintf(fileout, " %i\n", p3dsurfacetype);
  fprintf(fileout, "P3DSURFACESMOOTH\n");
  fprintf(fileout, " %i\n", p3dsurfacesmooth);
  fprintf(fileout, "PLOT2DHRRBOUNDS\n");
  fprintf(fileout, " %i %f %i %f\n", 0, plot2d_hrr_min, 0, plot2d_hrr_max);
  fprintf(fileout, "PROJECTION\n");
  fprintf(fileout, " %i\n", projection_type);
  fprintf(fileout, "SCALEDFONT\n");
  fprintf(fileout, " %i %f %i\n", scaled_font2d_height, scaled_font2d_height2width, scaled_font2d_thickness);
  fprintf(fileout, " %i %f %i\n", scaled_font3d_height, scaled_font3d_height2width, scaled_font3d_thickness);
  fprintf(fileout, "SHOWALLTEXTURES\n");
  fprintf(fileout, " %i\n", showall_textures);
  fprintf(fileout, "SHOWAXISLABELS\n");
  fprintf(fileout, " %i\n", visaxislabels);
  fprintf(fileout, "SHOWBLOCKLABEL\n");
  fprintf(fileout, " %i\n", visMeshlabel);
  fprintf(fileout, "SHOWBLOCKS\n");
  fprintf(fileout, " %i\n", visBlocks);
  fprintf(fileout, "SHOWBOUNDS\n");
  fprintf(fileout, " %i %i\n", bounds_each_mesh, show_bound_diffs);
  fprintf(fileout, "SHOWCADOPAQUE\n");
  fprintf(fileout, " %i\n", viscadopaque);
  fprintf(fileout, "SHOWCEILING\n");
  fprintf(fileout, " %i\n", visCeiling);
  fprintf(fileout, "SHOWCHID\n");
  fprintf(fileout, " %i\n", vis_title_CHID);
  fprintf(fileout, "SHOWCOLORBARS\n");
  fprintf(fileout, " %i %i\n", visColorbarVertical,visColorbarHorizontal);
  fprintf(fileout, "SHOWCVENTS\n");
  fprintf(fileout, " %i %i\n", visCircularVents, circle_outline);
  fprintf(fileout, "SHOWDUMMYVENTS\n");
  fprintf(fileout, " %i\n", visDummyVents);
  fprintf(fileout, "SHOWFIRECUTOFF\n");
  fprintf(fileout, " %i\n", show_firecutoff);
  fprintf(fileout, "SHOWFLOOR\n");
  fprintf(fileout, " %i\n", visFloor);
  fprintf(fileout, "SHOWFRAME\n");
  fprintf(fileout, " %i\n", visFrame);
  fprintf(fileout, "SHOWFRAMELABEL\n");
  fprintf(fileout, " %i\n", visFramelabel);
  fprintf(fileout, "SHOWFRAMETIMELABEL\n");
  fprintf(fileout, " %i\n", visFrameTimelabel);
  fprintf(fileout, "SHOWFRAMERATE\n");
  fprintf(fileout, " %i\n", visFramerate);
  fprintf(fileout, "SHOWGRID\n");
  fprintf(fileout, " %i\n", visGrid);
  fprintf(fileout, "SHOWGRIDLOC\n");
  fprintf(fileout, " %i\n", visgridloc);
  fprintf(fileout, "SHOWHMSTIMELABEL\n");
  fprintf(fileout, " %i\n", vishmsTimelabel);
  fprintf(fileout, "SHOWHRRLABEL\n");
  fprintf(fileout, " %i\n", vis_hrr_label);
  fprintf(fileout, "SHOWISO\n");
  fprintf(fileout, " %i\n", visAIso);
  fprintf(fileout, "SHOWISONORMALS\n");
  fprintf(fileout, " %i\n", show_iso_normal);
  fprintf(fileout, "SHOWLABELS\n");
  fprintf(fileout, " %i\n", visLabels);
#ifdef pp_memstatus
  fprintf(fileout, "SHOWMEMLOAD\n");
  fprintf(fileout, " %i\n", visAvailmemory);
#endif
  fprintf(fileout, "SHOWPARTTAG\n");
  fprintf(fileout, " %i\n", select_part);
  fprintf(fileout, "SHOWOPENVENTS\n");
  fprintf(fileout, " %i %i\n", visOpenVents, visOpenVentsAsOutline);
  fprintf(fileout, "SHOWOTHERVENTS\n");
  fprintf(fileout, " %i\n", scase->visOtherVents);
  fprintf(fileout, "SHOWROOMS\n");
  fprintf(fileout, " %i\n", visCompartments);
  fprintf(fileout, "SHOWSENSORS\n");
  fprintf(fileout, " %i %i\n", visSensor, visSensorNorm);
  fprintf(fileout, "SHOWSLICEINOBST\n");
  fprintf(fileout, " %i\n", scase->show_slice_in_obst);
  fprintf(fileout, "SHOWSMOKEPART\n");
  fprintf(fileout, " %i\n", visSmokePart);
#ifdef pp_SKY
  fprintf(fileout, "SHOWSKY\n");
  fprintf(fileout, " %i\n", visSky);
#endif
  fprintf(fileout, "SHOWSPRINKPART\n");
  fprintf(fileout, " %i\n", visSprinkPart);
  fprintf(fileout, "SHOWSTREAK\n");
  fprintf(fileout, " %i %i %i %i\n", streak5show, streak5step, showstreakhead, streak_index);
  fprintf(fileout, "SHOWTERRAIN\n");
  fprintf(fileout, " %i %i\n", scase->visTerrainType, terrain_slice_overlap);
  fprintf(fileout, "SHOWTHRESHOLD\n");
  fprintf(fileout, " %i %i %f\n", vis_threshold, vis_onlythreshold, temp_threshold);
  fprintf(fileout, "SHOWTICKS\n");
  fprintf(fileout, " %i\n", visFDSticks);
  fprintf(fileout, "SHOWTIMEBAR\n");
  fprintf(fileout, " %i %i\n", visTimebar,timebar_overlap);
  fprintf(fileout, "SHOWTIMELABEL\n");
  fprintf(fileout, " %i\n", visTimelabel);
  fprintf(fileout, "SHOWTITLE\n");
  fprintf(fileout, " %i %i %i\n", vis_title_smv_version, 0, vis_title_fds);
  fprintf(fileout, "SHOWTRACERSALWAYS\n");
  fprintf(fileout, " %i\n", show_tracers_always);
  fprintf(fileout, "SHOWTRANSPARENT\n");
  fprintf(fileout, " %i\n", visTransparentBlockage);
  fprintf(fileout, "SHOWTRIANGLES\n");
  fprintf(fileout, " %i %i %i %i 1 %i %i\n", show_iso_shaded, show_iso_outline, show_iso_points, show_iso_normal, smooth_iso_normal, sort_iso_triangles);
  fprintf(fileout, "SHOWTRANSPARENTVENTS\n");
  fprintf(fileout, " %i\n", show_transparent_vents);
  fprintf(fileout, "SHOWTRIANGLECOUNT\n");
  fprintf(fileout, " %i\n", show_triangle_count);
  fprintf(fileout, "SHOWVENTFLOW\n");
  fprintf(fileout, " %i %i %i %i %i\n", visVentHFlow, visventslab, visventprofile, visVentVFlow, visVentMFlow);
  fprintf(fileout, "SHOWTARGETS\n");
  fprintf(fileout, " %i \n", vis_target_data);
  fprintf(fileout, "SHOWVENTS\n");
  fprintf(fileout, " %i\n", visVents);
  fprintf(fileout, "SHOWWALLS\n");
  fprintf(fileout, " %i %i\n", visWalls,vis_wall_data);
  fprintf(fileout, "SKIPEMBEDSLICE\n");
  fprintf(fileout, " %i\n", skip_slice_in_embedded_mesh);
  fprintf(fileout, "SLICEDUP\n");
  fprintf(fileout, " %i %i %i\n", slicedup_option, vectorslicedup_option, boundaryslicedup_option);
  fprintf(fileout, "SMOKESENSORS\n");
  fprintf(fileout, " %i %i\n", show_smokesensors, test_smokesensors);
  fprintf(fileout, "STARTUPLANG\n");
  fprintf(fileout, " %s\n", startup_lang_code);
  fprintf(fileout, "STEREO\n");
  fprintf(fileout, " %i\n", stereotype);
  fprintf(fileout, "SURFINC\n");
  fprintf(fileout, " %i\n", surfincrement);
  fprintf(fileout, "TERRAINPARMS\n");
  fprintf(fileout, " %i %i %i\n", terrain_rgba_zmin[0], terrain_rgba_zmin[1], terrain_rgba_zmin[2]);
  fprintf(fileout, " %i %i %i\n", terrain_rgba_zmax[0], terrain_rgba_zmax[1], terrain_rgba_zmax[2]);
  fprintf(fileout, " %f\n", vertical_factor);
  fprintf(fileout, "TITLESAFE\n");
  fprintf(fileout, " %i\n", titlesafe_offset);
  if(trainer_mode == 1){
    fprintf(fileout, "TRAINERMODE\n");
    fprintf(fileout, " %i\n", trainer_mode);
  }
  fprintf(fileout, "TRAINERVIEW\n");
  fprintf(fileout, " %i\n", trainerview);
  fprintf(fileout, "TRANSPARENT\n");
  fprintf(fileout, " %i %f %i %i\n", use_transparency_data, transparent_level, iso_transparency_option, iso_opacity_change);
  fprintf(fileout, "TREEPARMS\n");
  fprintf(fileout, " %i %i %i %i\n", mintreesize,vis_xtree,vis_ytree,vis_ztree);
  fprintf(fileout, "TWOSIDEDVENTS\n");
  fprintf(fileout, " %i %i\n", show_bothsides_int, show_bothsides_ext);
  fprintf(fileout, "VECTORSKIP\n");
  fprintf(fileout, " %i\n", vectorskip);
  fprintf(fileout, "VISBOUNDARYTYPE\n");
  for(i = 0; i<7; i++){
    fprintf(fileout, " %i ", vis_boundary_type[i]);
  }
  fprintf(fileout, " %i %i \n", show_mirror_boundary, show_mirror_boundary);
  fprintf(fileout, "WINDROSEDEVICE\n");
  fprintf(fileout, " %i %i %i %i %i %i %i %i %i\n",
    viswindrose, showref_windrose, windrose_xy_vis, windrose_xz_vis, windrose_yz_vis, windstate_windrose, showlabels_windrose,
    windrose_first,windrose_next);
  fprintf(fileout, " %i %i %i %f %i %i\n", nr_windrose, ntheta_windrose, scale_windrose, radius_windrose, scale_increment_windrose, scale_max_windrose);
  {
    if(nwindrosez_showhide > 0){
      int ii;

      GLUIUpdateWindRoseDevices(UPDATE_WINDROSE_SHOWHIDE);
      fprintf(fileout, "WINDROSESHOWHIDE\n");
      fprintf(fileout, " %i\n", nwindrosez_showhide);
      for(ii = 0; ii < nwindrosez_showhide; ii++){
        fprintf(fileout, " %i", windrosez_showhide[ii]);
        if((ii+1)%WINDROSE_PER_ROW==0)fprintf(fileout, "\n");
      }
      fprintf(fileout, "\n");
    }
  }
  {
    float *xyzt;

    xyzt = windrose_merge_dxyzt;
    fprintf(fileout, "WINDROSEMERGE\n");
    fprintf(fileout, " %i %f %f %f %f\n",windrose_merge_type,xyzt[0],xyzt[1],xyzt[2],xyzt[3]);
  }
  fprintf(fileout, "ZOOM\n");
  fprintf(fileout, " %i %f\n", zoomindex, zoom);

  fprintf(fileout,"\n *** MISC ***\n\n");

  fprintf(fileout, "SHOWSLICEVALS\n");
  fprintf(fileout, " %i\n", show_slice_values_all_regions);
  if(scase->paths.fds_filein != NULL&&strlen(scase->paths.fds_filein) > 0){
    fprintf(fileout, "INPUT_FILE\n");
    fprintf(fileout, " %s\n", scase->paths.fds_filein);
  }
  fprintf(fileout, "LABELSTARTUPVIEW\n");
  fprintf(fileout, " %s\n", viewpoint_label_startup);
  fprintf(fileout, "RENDERCLIP\n");
  fprintf(fileout, " %i %i %i %i %i\n",
    clip_rendered_scene, render_clip_left, render_clip_right, render_clip_bottom, render_clip_top);
  fprintf(fileout, "RENDERFILELABEL\n");
  fprintf(fileout, " %i\n", render_label_type);
  fprintf(fileout, "RENDERFILETYPE\n");
  fprintf(fileout," %i %i %i\n",render_filetype, movie_filetype, render_resolution);
  fprintf(fileout, "MOVIEFILETYPE\n");
  {
    int quicktime_dummy=1;

    fprintf(fileout, "MOVIEFILETYPE\n");
    fprintf(fileout," %i %i %i %i %i\n",movie_filetype,movie_framerate,movie_bitrate,quicktime_dummy,movie_crf);
  }
  fprintf(fileout, "MOVIEPARMS\n");
  fprintf(fileout, " %i %i %i\n", movie_queue_index, movie_nprocs, movie_slice_index);
  if(nskyboxinfo>0){
    int iskybox;
    skyboxdata *skyi;
    char *filei;
    char *nullfile="NULL";

    for(iskybox=0;iskybox<nskyboxinfo;iskybox++){
      skyi = skyboxinfo + iskybox;
      fprintf(fileout,"SKYBOX\n");
      for(i=0;i<6;i++){
        filei = skyi->face[i].file;
        if(filei==NULL)filei=nullfile;
        if(strcmp(filei,"NULL")==0){
          fprintf(fileout,"NULL\n");
        }
        else{
          fprintf(fileout," %s\n",filei);
        }
      }
    }
  }
  fprintf(fileout, "RENDEROPTION\n");
  fprintf(fileout, " %i %i %i\n", render_window_size, resolution_multiplier, nheight360);
  fprintf(fileout, "UNITCLASSES\n");
  fprintf(fileout, " %i\n", nunitclasses);
  for(i = 0; i<nunitclasses; i++){
    fprintf(fileout, " %i\n", unitclasses[i].unit_index);
  }
  fprintf(fileout, "SHOWGRAVVECTOR\n");
  fprintf(fileout, " %i\n", showgravity_vector);
  if(zaxis_custom == 1){
    fprintf(fileout, "ZAXISANGLES\n");
    fprintf(fileout, " %f %f %f\n", zaxis_angles[0], zaxis_angles[1], zaxis_angles[2]);
  }

  fprintf(fileout,"\n *** 3D SMOKE INFO ***\n\n");

  {
    colorbardata *cb;
    char percen[2];

    cb = colorbars.colorbarinfo + colorbartype;
    strcpy(percen, "%");
    fprintf(fileout, "COLORBARTYPE\n");
    fprintf(fileout, " %i %s %s \n", colorbartype, percen, cb->menu_label);
  }
  if(colorbars.co2_colorbar_index >= 0 && colorbars.co2_colorbar_index < colorbars.ncolorbars){
    fprintf(fileout, "COLORMAP\n");
    fprintf(fileout, " CO2 %i %s\n", co2_colormap_type, colorbars.colorbarinfo[colorbars.co2_colorbar_index].menu_label);
  }
  {
    int mmin[3], mmax[3];
    for(i = 0; i < 3; i++){
      mmin[i] = rgb_below_min[i];
      mmax[i] = rgb_above_max[i];
    }
    fprintf(fileout, "EXTREMECOLORS\n");
    fprintf(fileout, " %i %i %i %i %i %i\n",
      mmin[0], mmin[1], mmin[2],
      mmax[0], mmax[1], mmax[2]);
  }
  fprintf(fileout, "FIRECOLOR\n");
  fprintf(fileout, " %i %i %i\n", fire_color_int255[0], fire_color_int255[1], fire_color_int255[2]);
  if(colorbars.fire_colorbar_index >= 0 && colorbars.fire_colorbar_index < colorbars.ncolorbars){
    fprintf(fileout, "FIRECOLORMAP\n");
    fprintf(fileout, " FIRE %i %s\n", fire_colormap_type, colorbars.colorbarinfo[colorbars.fire_colorbar_index].menu_label);
  }
  fprintf(fileout, "FIREDEPTH\n");
  fprintf(fileout, " %f %f %f %i %i\n", fire_halfdepth, co2_halfdepth, emission_factor, use_fire_alpha, force_alpha_opaque);
  if(colorbars.ncolorbars > colorbars.ndefaultcolorbars){
    colorbardata *cbi;
    unsigned char *rrgb;
    int n;

    fprintf(fileout, "GCOLORBAR\n");
    fprintf(fileout, " %i\n", colorbars.ncolorbars - colorbars.ndefaultcolorbars);
    for(n = colorbars.ndefaultcolorbars; n < colorbars.ncolorbars; n++){
      cbi = colorbars.colorbarinfo + n;
      fprintf(fileout, " %s\n", cbi->menu_label);
      fprintf(fileout, " %i %i\n", cbi->nnodes, cbi->nodehilight);
      for(i = 0; i < cbi->nnodes; i++){
        rrgb = cbi->node_rgb + 3 * i;
        fprintf(fileout, " %i %i %i %i\n", (int)cbi->node_index[i], (int)rrgb[0], (int)rrgb[1], (int)rrgb[2]);
      }
    }
  }
  fprintf(fileout, "HRRPUVCUTOFF\n");
  fprintf(fileout, " %f\n", scase->global_hrrpuv_cutoff);
  fprintf(fileout, "SHOWEXTREMEDATA\n");
  {
    int show_extremedata = 0;

    if(show_extreme_mindata == 1 || show_extreme_maxdata == 1)show_extremedata = 1;
    fprintf(fileout, " %i %i %i\n", show_extremedata, show_extreme_mindata, show_extreme_maxdata);
  }
  fprintf(fileout, "SLICESKIP\n");
  fprintf(fileout, " %i %i %i %i\n", slice_skip, slice_skipx, slice_skipy, slice_skipz);
  fprintf(fileout, "SMOKECOLOR\n");
  fprintf(fileout, " %i %i %i %i\n", smoke_color_int255[0], smoke_color_int255[1], smoke_color_int255[2], force_gray_smoke);
  fprintf(fileout, "SMOKECULL\n");
  fprintf(fileout," %i\n",smokecullflag);
  if(ABS(scase->smoke_albedo - scase->smoke_albedo_base) > 0.001){
    fprintf(fileout, "SMOKEALBEDO\n");
    fprintf(fileout, " %f\n", scase->smoke_albedo);
  }
  if((have_fire == NO_FIRE && have_smoke == NO_SMOKE)||(have_fire != NO_FIRE && have_smoke != NO_SMOKE)){
    fprintf(fileout, "SMOKEFIREPROP\n");
    fprintf(fileout, " %i %i\n", use_opacity_depth, use_opacity_multiplier);
  }
  fprintf(fileout, "SMOKEPROP\n");
  fprintf(fileout, "%f\n", glui_smoke3d_extinct);
  glui_smoke3d_extinct_default = glui_smoke3d_extinct;
  fprintf(fileout, "SMOKESKIP\n");
  fprintf(fileout," %i %i %i %i %i\n",smokeskipm1,smoke3d_skip, smoke3d_skipx, smoke3d_skipy, smoke3d_skipz);
#ifdef pp_GPU
  fprintf(fileout, "USEGPU\n");
  fprintf(fileout, " %i\n", usegpu);
#endif
  fprintf(fileout, "VOLSMOKE\n");
  fprintf(fileout, " %i %i %i %i %i\n",
    glui_compress_volsmoke, use_multi_threading, load_at_rendertimes, volbw, show_volsmoke_moving);
  fprintf(fileout, " %f %f %f %f %f %f %f\n",
    global_temp_min, global_temp_cutoff, global_temp_max, fire_opacity_factor, mass_extinct, gpu_vol_factor, nongpu_vol_factor);

  fprintf(fileout, "\n *** ZONE FIRE PARAMETRES ***\n\n");

  fprintf(fileout, "SHOWHAZARDCOLORS\n");
  fprintf(fileout, " %i\n", zonecolortype);
  fprintf(fileout, "SHOWZONEPLANE\n");
  fprintf(fileout, " %i %i\n", visZonePlane, show_zonelower);
  fprintf(fileout, "SHOWSZONE\n");
  fprintf(fileout, " %i\n", visSZone);
  fprintf(fileout, "SHOWZONEFIRE\n");
  fprintf(fileout, " %i\n", viszonefire);

  fprintf(fileout,"\n *** TOUR INFO ***\n\n");

  fprintf(fileout, "SHOWAVATAR\n");
  fprintf(fileout, " %i\n", show_avatar);
  fprintf(fileout, "SHOWPATHNODES\n");
  fprintf(fileout, " %i\n", show_path_knots);
  fprintf(fileout, "SHOWTOURROUTE\n");
  fprintf(fileout, " %i\n", edittour);
  {
    float *col;

    fprintf(fileout, "TOURCOLORS\n");
    col = tourcol_selectedpathline;
    fprintf(fileout, " %f %f %f   :selected path line\n", col[0], col[1], col[2]);
    col = tourcol_selectedpathlineknots;
    fprintf(fileout, " %f %f %f   :selected path line knots\n", col[0], col[1], col[2]);
    col = tourcol_selectedknot;
    fprintf(fileout, " %f %f %f   :selected knot\n", col[0], col[1], col[2]);
    col = tourcol_pathline;
    fprintf(fileout, " %f %f %f   :path line\n", col[0], col[1], col[2]);
    col = tourcol_pathknots;
    fprintf(fileout, " %f %f %f   :path knots\n", col[0], col[1], col[2]);
    col = tourcol_text;
    fprintf(fileout, " %f %f %f   :text\n", col[0], col[1], col[2]);
    col = tourcol_avatar;
    fprintf(fileout, " %f %f %f   :avatar\n", col[0], col[1], col[2]);


  }
  fprintf(fileout, "VIEWALLTOURS\n");
  fprintf(fileout, " %i\n", viewalltours);
  fprintf(fileout, "VIEWTIMES\n");
  fprintf(fileout, " %f %f %i\n", scase->tourcoll.tour_tstart, scase->tourcoll.tour_tstop, scase->tourcoll.tour_ntimes);
  fprintf(fileout, "VIEWTOURFROMPATH\n");
  fprintf(fileout, " %i %i %f\n", viewtourfrompath, tour_snap, tour_snap_time);


  if(flag == LOCAL_INI)WriteIniLocal(scase, fileout);
  if(
    ((INI_fds_filein != NULL&&scase->paths.fds_filein != NULL&&strcmp(INI_fds_filein, scase->paths.fds_filein) == 0) ||
    flag == LOCAL_INI))OutputViewpoints(fileout);

  {
    char githash[256];
    char gitdate[256];

    GetGitInfo(githash,gitdate);    // get githash
    fprintf(fileout,"\n\n");
    fprintf(fileout,"# FDS/Smokeview Environment\n");
    fprintf(fileout,"# -------------------------\n\n");
    fprintf(fileout,"# Smokeview Build: %s\n",githash);
    fprintf(fileout,"# Smokeview Build Date: %s\n",__DATE__);
    if(scase->fds_version!=NULL){
      fprintf(fileout,"# FDS Version: %s\n",scase->fds_version);
    }
    if(scase->fds_githash!=NULL){
      fprintf(fileout, "# FDS Build: %s\n", scase->fds_githash);
    }
    fprintf(fileout,"# Platform: WIN64\n");
#ifdef pp_OSX
    PRINTF("Platform: OSX64\n");
#endif
#ifdef pp_LINUX
    fprintf(fileout,"# Platform: LINUX64\n");
#endif

    if(use_graphics==1){
      GLint nred, ngreen, nblue, ndepth, nalpha;
      int max_texture_size;

      glGetIntegerv(GL_RED_BITS,&nred);
      glGetIntegerv(GL_GREEN_BITS,&ngreen);
      glGetIntegerv(GL_BLUE_BITS,&nblue);
      glGetIntegerv(GL_DEPTH_BITS,&ndepth);
      glGetIntegerv(GL_ALPHA_BITS,&nalpha);
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
      fprintf(fileout, "\n\n");
      fprintf(fileout,"# Graphics Environment\n");
      fprintf(fileout,"# --------------------\n\n");
      if(use_graphics == 1){
        char version_label[256];
        char *glversion = NULL;
        char *glshadeversion = NULL;

        glversion = (char *)glGetString(GL_VERSION);
        if(glversion != NULL){
          strcpy(version_label, "OpenGL Version: ");
          strcat(version_label, glversion);
          fprintf(fileout, "# %s\n", version_label);
        }
        glshadeversion=(char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if(glshadeversion != NULL){
          strcpy(version_label, "  GLSL Version: ");
          strcat(version_label, glshadeversion);
          fprintf(fileout, "# %s\n", version_label);
        }
      }
      fprintf(fileout,"#         Red bits:%i\n", nred);
      fprintf(fileout,"#       Green bits:%i\n",ngreen);
      fprintf(fileout,"#        Blue bits:%i\n",nblue);
      fprintf(fileout,"#       Alpha bits:%i\n",nalpha);
      fprintf(fileout,"#       Depth bits:%i\n",ndepth);
      fprintf(fileout,"# max texture size:%i\n\n",max_texture_size);
    }
  }

  if(fileout!=stdout){
    fclose(fileout);
  }
  FREEMEMORY(smokeview_scratchdir);
}
