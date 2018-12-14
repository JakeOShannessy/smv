#define XYZ_EXTRA 7

#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include GLUT_H
#include <math.h>

#include "smv_endian.h"
#include "update.h"
#include "smokeviewvars.h"
#include "histogram.h"
#include "compress.h"
#include "IOobjects.h"

#define READPASS 1
#define READFAIL 0

#define FORTPART5READ(var,size) \
returncode=READPASS;\
FSEEK(PART5FILE,4,SEEK_CUR);if(ferror(PART5FILE)==1||feof(PART5FILE)==1)returncode=READFAIL;\
if(returncode==READPASS){\
  fread(var,4,size,PART5FILE);\
  if(ferror(PART5FILE)==1||feof(PART5FILE)==1)returncode=READFAIL;\
}\
if(returncode==READPASS){\
  if(endianswitch==1)EndianSwitch(var,size);\
  FSEEK(PART5FILE,4,SEEK_CUR);\
  if(ferror(PART5FILE)==1||feof(PART5FILE)==1)returncode=READFAIL;\
}

/* ------------------ GetEvacPartColor ------------------------ */

int GetEvacPartColor(float **color_handle, part5data *datacopy, int show_default, int j, int itype){
  int is_human_color;
  float *colorptr;
  unsigned char *color;
  int showcolor;

  showcolor = 1;
  is_human_color = 0;
  if(current_property != NULL&&strcmp(current_property->label->longlabel, "HUMAN_COLOR") == 0 && navatar_colors > 0){
    is_human_color = 1;
  }
  if(show_default == 1){
    colorptr = datacopy->partclassbase->rgb;
  }
  else{
    color = datacopy->irvals + itype*datacopy->npoints;
    if(is_human_color == 1){
      colorptr = avatar_colors + 3 * color[j];
    }
    else{
      colorptr = rgb_full[color[j]];
    }
    if(current_property != NULL && (color[j] > current_property->imax || color[j] < current_property->imin))showcolor = 0;
  }
  *color_handle = colorptr;
  return showcolor;
}

/* ------------------ GetPartPropS ------------------------ */

partpropdata *GetPartPropS(char *label){
  int i;

  for(i = 0;i < npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->shortlabel, label) == 0)return propi;
  }
  return NULL;
}

/* ------------------ CopyDepVals ------------------------ */

void CopyDepVals(partclassdata *partclassi, part5data *datacopy, float *colorptr, propdata *prop, int j){
  int ii;
  int ndep_vals;
  float *dep_vals;

  if(prop == NULL)return;
  dep_vals = partclassi->fvars_dep;
  ndep_vals = partclassi->nvars_dep;
  for(ii = 0; ii < partclassi->nvars_dep - 3; ii++){

    unsigned char *var_type;
    unsigned char color_index;
    partpropdata *varprop;
    float valmin, valmax;
    char *shortlabel;
    flowlabels *label;

    shortlabel = NULL;
    varprop = NULL;
    label = datacopy->partclassbase->labels + ii + 2;
    if(label != NULL)shortlabel = label->shortlabel;
    if(shortlabel != NULL)varprop = GetPartPropS(shortlabel);
    if(varprop != NULL){
      var_type = datacopy->irvals + ii*datacopy->npoints;
      color_index = var_type[j];
      valmin = varprop->valmin;
      valmax = varprop->valmax;
      dep_vals[ii] = valmin + color_index*(valmax - valmin) / 255.0;
    }
    else{
      dep_vals[ii] = 1.0;
    }
  }

  dep_vals[ndep_vals - 3] = colorptr[0] * 255;
  dep_vals[ndep_vals - 2] = colorptr[1] * 255;
  dep_vals[ndep_vals - 1] = colorptr[2] * 255;
  prop->nvars_dep = partclassi->nvars_dep;
  prop->smv_object->visible = 1;
  for(ii = 0; ii < prop->nvars_dep; ii++){
    prop->fvars_dep[ii] = partclassi->fvars_dep[ii];
  }
  prop->nvars_dep = partclassi->nvars_dep;
  for(ii = 0; ii < partclassi->nvars_dep; ii++){
    prop->vars_dep_index[ii] = partclassi->vars_dep_index[ii];
  }
  prop->tag_number = datacopy->tags[j];
}

/* ------------------ CompareTags ------------------------ */

int CompareTags(const void *arg1, const void *arg2){
  int i, j;

  i = *(int *)arg1;
  j = *(int *)arg2;
  if(i < j)return -1;
  if(i > j)return 1;
  return 0;

}

/* ------------------ GetTagIndex ------------------------ */

int GetTagIndex(const partdata *partin, part5data **datain, int tagval){
  int *returnval;
  part5data *data;
  int i;

  for(i = -1; i < npartinfo; i++){
    const partdata *parti;

    if(i == -1){
      parti = partin;
      data = *datain;
    }
    else{
      parti = partinfo + i;
      if(parti == partin)continue;
      if(parti->loaded == 0 || parti->display == 0)continue;
      data = parti->data5 + (*datain - partin->data5);
    }
    if(parti->loaded == 0 || parti->display == 0)continue;

    if(data->npoints == 0)continue;
    ASSERT(data->npoints>0);
    ASSERT(data->sort_tags != NULL);
    returnval = bsearch(&tagval, data->sort_tags, data->npoints, 2 * sizeof(int), CompareTags);
    if(returnval == NULL)continue;
    *datain = data;
    return *(returnval + 1);
  }
  return -1;
}

/* ------------------ DrawPart ------------------------ */

void DrawPart(const partdata *parti){
  int ipframe;
  part5data *datacopy, *datapast;
  int nclasses;
  int i, j;
  int offset_terrain;
  propdata *prop;

  if(parti->times[0] > global_times[itimes])return;
  if(nterraininfo > 0 && ABS(vertical_factor - 1.0) > 0.01){
    offset_terrain = 1;
  }
  else{
    offset_terrain = 0;
  }

  if(current_property == NULL)return;
  ipframe = parti->itime;
  if(ipframe < 0){
    ipframe = 0;
  } //xxx need to check this - why is ipframe < 0 ???
  nclasses = parti->nclasses;
  datacopy = parti->data5 + nclasses*ipframe;
  CheckMemory;
  if(part5show == 1){
    if(streak5show == 0 || (streak5show == 1 && showstreakhead == 1)){
      for(i = 0;i < parti->nclasses;i++){
        short *sx, *sy, *sz;
        float *angle, *width, *depth, *height;
        unsigned char *vis, *color;
        partclassdata *partclassi;
        int partclass_index, itype, vistype, class_vis;
        int show_default;

        partclassi = parti->partclassptr[i];
        partclass_index = partclassi - partclassinfo;

        vistype = current_property->class_present[partclass_index];
        class_vis = current_property->class_vis[partclass_index];


        if(vistype == 0 || datacopy->npoints <= 0 || (vistype == 1 && class_vis == 0)){
          if(show_tracers_always == 0 || partclassi->ntypes > 2){
            datacopy++;
            continue;
          }
        }
        itype = current_property->class_types[partclass_index];

        show_default = 0;
        if(itype == -1 || (show_tracers_always == 1 && partclassi->ntypes <= 2)){
          show_default = 1;
        }

        sx = datacopy->sx;
        sy = datacopy->sy;
        sz = datacopy->sz;
        vis = datacopy->vis_part;
        if(parti->evac == 1){
          int avatar_type = 0;

          angle = datacopy->avatar_angle;
          width = datacopy->avatar_width;
          depth = datacopy->avatar_depth;
          height = datacopy->avatar_height;
          CheckMemory;

          avatar_type = 0;
          prop = datacopy->partclassbase->prop;
          if(prop == NULL)prop = prop_evacdefault;
          if(iavatar_evac != -1)avatar_type = iavatar_evac;
          for(j = 0;j < datacopy->npoints;j++){
            float az_angle;
            float *colorptr;

            if(vis[j] == 1){
              int save_use_displaylist;

              glPushMatrix();
              glTranslatef(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]] - SCALE2SMV(parti->zoffset));
              if(select_avatar == 1 && selected_avatar_tag > 0 && selected_avatar_tag == datacopy->tags[j]){
                selected_avatar_pos[0] = xplts[sx[j]];
                selected_avatar_pos[1] = yplts[sy[j]];
                selected_avatar_pos[2] = zplts[sz[j]];
                selected_avatar_angle = datacopy->avatar_angle[j];
              }
              glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));

              az_angle = angle[j];
              glRotatef(az_angle, 0.0, 0.0, 1.0);

              GetEvacPartColor(&colorptr, datacopy, show_default, j, itype);

              //  :W :D :H1 :SX :SY :SZ :R :G :B :HX :HY :HZ
              //  class color: rgbobject[0], rgbobject[1], rgbobject[2]

              if(prop != NULL){
                int n;
                sv_object_frame *obj_frame;
                tokendata **evac_tokens, *evac_token;

                obj_frame = prop->smv_object->obj_frames[0];
                evac_tokens = obj_frame->evac_tokens;
                obj_frame->nevac_tokens = 12;

                n = 0;

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = width[j]; //:W

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = depth[j]; //:D

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = 1.0;//:H1

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = 1.0;//:SX

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = 1.0;//:SY

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = height[j];  //:SZ

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = 255 * colorptr[0]; //:R

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = 255 * colorptr[1];//:G

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = 255 * colorptr[2];//:B

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = 0.0;//:HX

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = 0.0;//:HY

                evac_token = evac_tokens[n++];
                if(evac_token != NULL)evac_token->evac_var = height[j] / 2.0; //:HZ
                prop->draw_evac = 1;
              }

              save_use_displaylist = avatar_types[avatar_type]->use_displaylist;
              if(select_avatar == 1 && show_mode == SELECTOBJECT){
                int tagval;

                avatar_types[avatar_type]->select_mode = 1;
                select_device_color_ptr = select_device_color;
                tagval = datacopy->tags[j];
                select_device_color[0] = tagval >> (ngreenbits + nbluebits);
                select_device_color[1] = tagval >> nbluebits;
                select_device_color[2] = tagval&rgbmask[nbluebits - 1];
                avatar_types[avatar_type]->use_displaylist = 0;
              }
              else{
                if(selected_avatar_tag > 0 && select_avatar == 1 && datacopy->tags[j] == selected_avatar_tag){
                  select_device_color_ptr = select_device_color;
                  select_device_color[0] = 255;
                  select_device_color[1] = 0;
                  select_device_color[2] = 0;
                  avatar_types[avatar_type]->use_displaylist = 0;
                }
                else{
                  select_device_color_ptr = NULL;
                  avatar_types[avatar_type]->select_mode = 0;
                }
              }
              CopyDepVals(partclassi, datacopy, colorptr, prop, j);
              DrawSmvObject(avatar_types[avatar_type], 0, prop, 0, NULL, 0);
              select_device_color_ptr = NULL;
              avatar_types[avatar_type]->use_displaylist = save_use_displaylist;
              glPopMatrix();
            }
          }
          SNIFF_ERRORS("after draw in Evac");
        }
        else{
          glPointSize(partpointsize);
          if(offset_terrain == 0){

            // *** draw particles as points

            if(datacopy->partclassbase->vis_type == PART_POINTS){
              glBegin(GL_POINTS);
              if(show_default == 1){
                glColor4fv(datacopy->partclassbase->rgb);
                for(j = 0;j < datacopy->npoints;j++){
                  if(vis[j] == 1){
                    glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
                  }
                }
              }
              else{
                color = datacopy->irvals + itype*datacopy->npoints;
                for(j = 0;j < datacopy->npoints;j++){
                  if(vis[j] == 1){
                    if(current_property != NULL && (color[j] > current_property->imax || color[j] < current_property->imin))continue;
                    glColor4fv(rgb_full[color[j]]);
                    glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
                  }
                }
              }
              glEnd();
            }

            // *** draw particles using smokeview object

            if(datacopy->partclassbase->vis_type == PART_SMV_DEVICE){
              for(j = 0;j < datacopy->npoints;j++){
                float *colorptr;

                if(vis[j] != 1)continue;

                glPushMatrix();
                glTranslatef(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);

                glRotatef(-datacopy->partclassbase->elevation, 0.0, 1.0, 0.0);
                glRotatef(datacopy->partclassbase->azimuth, 0.0, 0.0, 1.0);

                //  0->2   color
                //  3      diameter
                //  4      length

                if(show_default == 1){
                  colorptr = datacopy->partclassbase->rgb;
                }
                else{
                  color = datacopy->irvals + itype*datacopy->npoints;
                  colorptr = rgb_full[color[j]];
                }

                prop = datacopy->partclassbase->prop;
                CopyDepVals(partclassi, datacopy, colorptr, prop, j);
                glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));

                partfacedir[0] = xbar0 + SCALE2SMV(fds_eyepos[0]) - xplts[sx[j]];
                partfacedir[1] = ybar0 + SCALE2SMV(fds_eyepos[1]) - yplts[sy[j]];
                partfacedir[2] = zbar0 + SCALE2SMV(fds_eyepos[2]) - zplts[sz[j]];

                DrawSmvObject(prop->smv_object, 0, prop, 0, NULL, 0);
                glPopMatrix();
              }
            }

            // *** draw particle as lines

            if(datacopy->partclassbase->vis_type == PART_LINES
              && ((datacopy->dsx != NULL&&datacopy->dsy != NULL&&datacopy->dsz != NULL) || datacopy->partclassbase->device_name != NULL)
              ){
              float *dxv, *dyv, *dzv;
              float dx, dy, dz;
              int flag = 0;

              if(datacopy->dsx != NULL&&datacopy->dsy != NULL&&datacopy->dsz != NULL){
                flag = 1;
                dxv = datacopy->dsx;
                dyv = datacopy->dsy;
                dzv = datacopy->dsz;
              }
              else{
                dx = datacopy->partclassbase->dx;
                dy = datacopy->partclassbase->dy;
                dz = datacopy->partclassbase->dz;
              }
              glBegin(GL_LINES);
              if(show_default == 1){
                glColor4fv(datacopy->partclassbase->rgb);
                for(j = 0;j < datacopy->npoints;j++){
                  if(vis[j] == 1){
                    if(flag == 1){
                      dx = dxv[j];
                      dy = dyv[j];
                      dz = dzv[j];
                    }
                    glVertex3f(xplts[sx[j]] - dx, yplts[sy[j]] - dy, zplts[sz[j]] - dz);
                    glVertex3f(xplts[sx[j]] + dx, yplts[sy[j]] + dy, zplts[sz[j]] + dz);
                  }
                }
              }
              else{
                color = datacopy->irvals + itype*datacopy->npoints;
                for(j = 0;j < datacopy->npoints;j++){
                  if(vis[j] == 1){
                    glColor4fv(rgb_full[color[j]]);
                    if(flag == 1){
                      dx = dxv[j];
                      dy = dyv[j];
                      dz = dzv[j];
                    }
                    glVertex3f(xplts[sx[j]] - dx, yplts[sy[j]] - dy, zplts[sz[j]] - dz);
                    glVertex3f(xplts[sx[j]] + dx, yplts[sy[j]] + dy, zplts[sz[j]] + dz);
                  }
                }
              }
              glEnd();
            }
          }
          else{
            glBegin(GL_POINTS);
            if(show_default == 1){
              glColor4fv(datacopy->partclassbase->rgb);
              for(j = 0;j < datacopy->npoints;j++){
                float zoffset;
                float xx, yy, zz;
                int loc;

                xx = xplts[sx[j]];
                yy = yplts[sy[j]];
                zz = zplts[sz[j]];

                zoffset = GetZCellValOffset(meshinfo, xx, yy, &loc);
                if(vis[j] == 1)glVertex3f(xx, yy, zz + zoffset);
              }
            }
            else{
              color = datacopy->irvals + itype*datacopy->npoints;
              for(j = 0;j < datacopy->npoints;j++){
                if(vis[j] == 1){
                  glColor4fv(rgb_full[color[j]]);
                  glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
                }
              }
            }
            glEnd();
          }
        }

        datacopy++;
      }
    }
  }

  // draw streak lines

  datacopy = parti->data5 + nclasses*ipframe;

  if(streak5show == 1){
    for(i = 0;i < parti->nclasses;i++){
      short *sx, *sy, *sz;
      short *sxx, *syy, *szz;
      unsigned char *vis;
      int k;
      int show_default;
      float *colorptr;

      partclassdata *partclassi;
      int partclass_index, itype, vistype, class_vis;

      partclassi = parti->partclassptr[i];
      partclass_index = partclassi - partclassinfo;

      vistype = current_property->class_present[partclass_index];
      class_vis = current_property->class_vis[partclass_index];

      if(vistype == 0 || datacopy->npoints <= 0 || (vistype == 1 && class_vis == 0)){
        if(show_tracers_always == 0 || partclassi->ntypes > 2){
          datacopy++;
          continue;
        }
      }
      itype = current_property->class_types[partclass_index];

      show_default = 0;
      if(itype == -1 || (show_tracers_always == 1 && partclassi->ntypes <= 2)){
        show_default = 1;
      }

      sx = datacopy->sx;
      sy = datacopy->sy;
      sz = datacopy->sz;
      vis = datacopy->vis_part;

      if(show_default == 1){

        // draw the streak line

        GetEvacPartColor(&colorptr, datacopy, show_default, 0, itype);
        glColor4fv(colorptr);

        glLineWidth(streaklinewidth);
        for(j = 0;j < datacopy->npoints;j++){
          int tagval;

          tagval = datacopy->tags[j];
          if(vis[j] == 0)continue;
          glBegin(GL_LINE_STRIP);
          glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
          for(k = 1;k < streak5step;k++){
            int jj;

            if(ipframe - k < 0)break;
            datapast = parti->data5 + nclasses*(ipframe - k) + i;
            jj = GetTagIndex(parti, &datapast, tagval);
            if(jj < 0)break;
            sxx = datapast->sx;
            syy = datapast->sy;
            szz = datapast->sz;
            glVertex3f(xplts[sxx[jj]], yplts[syy[jj]], zplts[szz[jj]]);
          }
          glEnd();
        }

        // draw the dot at the end of the streak line
      }
      else{

        // draw the streak line

        for(j = 0;j < datacopy->npoints;j++){
          int tagval;

          tagval = datacopy->tags[j];
          if(vis[j] == 0)continue;
          if(GetEvacPartColor(&colorptr, datacopy, show_default, j, itype) == 0)continue;

          glBegin(GL_LINE_STRIP);
          glColor4fv(colorptr);
          glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
          for(k = 1;k < streak5step;k++){
            int jj;

            if(ipframe - k < 0)break;
            datapast = parti->data5 + nclasses*(ipframe - k) + i;
            jj = GetTagIndex(parti, &datapast, tagval);
            if(jj < 0 || datapast->irvals == NULL)break;
            sxx = datapast->sx;
            syy = datapast->sy;
            szz = datapast->sz;

            GetEvacPartColor(&colorptr, datacopy, show_default, jj, itype);
            glColor4fv(colorptr);
            glVertex3f(xplts[sxx[jj]], yplts[syy[jj]], zplts[szz[jj]]);
          }
          glEnd();
        }
      }

      datacopy++;
    }
  }

}

/* ------------------ DrawPartFrame ------------------------ */

void DrawPartFrame(void){
  partdata *parti;
  int i;

  for(i=0;i<npartinfo;i++){
    parti = partinfo + i;
    if(parti->loaded==0||parti->display==0)continue;
    if(parti->evac==1){
      DrawEvac(parti);
      SNIFF_ERRORS("after DrawEvac");
    }
    else{
      DrawPart(parti);
      SNIFF_ERRORS("after DrawPart");
    }
  }
}

/* ------------------ DrawEvacFrame ------------------------ */

void DrawEvacFrame(void){
  int i;

  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(parti->loaded==0||parti->display==0||parti->evac==0)continue;
    DrawEvac(parti);
  }
  SNIFF_ERRORS("after DrawEvac 2");
}

/* ------------------ FreePart5Data ------------------------ */

void FreePart5Data(part5data *datacopy){
  FREEMEMORY(datacopy->cvals);
  FREEMEMORY(datacopy->sx);
  FREEMEMORY(datacopy->sy);
  FREEMEMORY(datacopy->sz);
  FREEMEMORY(datacopy->dsx);
  FREEMEMORY(datacopy->dsy);
  FREEMEMORY(datacopy->dsz);
  FREEMEMORY(datacopy->avatar_angle);
  FREEMEMORY(datacopy->avatar_width);
  FREEMEMORY(datacopy->avatar_height);
  FREEMEMORY(datacopy->avatar_depth);
  FREEMEMORY(datacopy->tags);
  FREEMEMORY(datacopy->sort_tags);
  FREEMEMORY(datacopy->vis_part);
  FREEMEMORY(datacopy->rvals);
  FREEMEMORY(datacopy->irvals);
}

/* ------------------ FreeAllPart5Data ------------------------ */

void FreeAllPart5Data(partdata *parti){
  int i;
  part5data *datacopy;

  if(parti->data5==NULL)return;
  datacopy = parti->data5;
  for(i=0;i<parti->ntimes*parti->nclasses;i++){
    FreePart5Data(datacopy);
    datacopy++;
  }
  FREEMEMORY(parti->data5);
}

/* ------------------ InitPart5Data ------------------------ */

void InitPart5Data(part5data *datacopy, partclassdata *partclassi){
  datacopy->cvals=NULL;
  datacopy->partclassbase=partclassi;
  datacopy->sx=NULL;
  datacopy->sy=NULL;
  datacopy->sz=NULL;
  datacopy->dsx=NULL;
  datacopy->dsy=NULL;
  datacopy->dsz=NULL;
  datacopy->avatar_angle=NULL;
  datacopy->avatar_width=NULL;
  datacopy->avatar_height=NULL;
  datacopy->avatar_depth=NULL;
  datacopy->tags=NULL;
  datacopy->vis_part=NULL;
  datacopy->sort_tags=NULL;
  datacopy->rvals=NULL;
  datacopy->irvals=NULL;
}

/* ------------------ ComparePart ------------------------ */

int ComparePart(const void *arg1, const void *arg2){
  partdata *parti, *partj;
  int i, j;

  i = *(int *)arg1;
  j = *(int *)arg2;

  parti = partinfo + i;
  partj = partinfo + j;

  if(parti->blocknumber<partj->blocknumber)return -1;
  if(parti->blocknumber>partj->blocknumber)return 1;
  return 0;
}

/* ------------------ UpdatePartVis ------------------------ */

void UpdatePartVis(int first_frame, partdata *parti, part5data *datacopy, int nclasses){
  int nparts;
  unsigned char *vis_part;

  nparts = datacopy->npoints;
  vis_part = datacopy->vis_part;

  if(first_frame == 1){
    int ii;

    for(ii = 0; ii < nparts; ii++){
      vis_part[ii] = 1;
    }
  }
  else{
    int ii;
    part5data *datalast;
    int nvis = 0, nleft;

    for(ii = 0; ii < nparts; ii++){
      int tag_index;

      datalast = datacopy - nclasses;
      tag_index = GetTagIndex(parti, &datalast, datacopy->tags[ii]);
      if(tag_index != -1 && datalast->vis_part[tag_index] == 1){
        datacopy->vis_part[ii] = 1;
        nvis++;
      }
      else{
        datacopy->vis_part[ii] = 0;
      }
    }

    nleft = nparts - nvis;
    if(nleft > 0){
      for(ii = 0; ii<nparts; ii++){
        if(datacopy->vis_part[ii] == 1)continue;
        if(nleft>0){
          datacopy->vis_part[ii] = 1;
          nleft--;
        }
      }
    }
  }
}

/* ------------------ UpdateAllPartVis ------------------------ */

void UpdateAllPartVis(partdata *parti){
  part5data *datacopy;
  int i, j;
  int firstframe = 1;

  datacopy = parti->data5;
  for(i = 0; i < parti->ntimes; i++){
    for(j = 0; j < parti->nclasses; j++){
      UpdatePartVis(firstframe, parti, datacopy, parti->nclasses);
      datacopy++;
    }
    if(firstframe == 1)firstframe = 0;
  }
}

/* ------------------ GetHistFileStatus ------------------------ */

int GetHistFileStatus(partdata *parti){

  // return -1 if history file cannot be created (corresponding particle file does not exist)
  // return  0 if history file does not need to be created
  // return  1 if history file needs to be created (doesn't exist or is older than corresponding particle file)

  STRUCTSTAT stat_histfile_buffer, stat_regfile_buffer;
  int stat_histfile, stat_regfile;

  stat_histfile = STAT(parti->hist_file, &stat_histfile_buffer);
  stat_regfile = STAT(parti->reg_file, &stat_regfile_buffer);

  if(stat_regfile != 0)return HIST_ERR;                    // particle filei does not exist

  if(stat_regfile_buffer.st_size > parti->reg_file_size){  // particle file has grown
    parti->reg_file_size = stat_regfile_buffer.st_size;
    return HIST_OLD;
  }

  if(stat_histfile != 0) return HIST_OLD;                  // history file does not exist
  if(stat_regfile_buffer.st_mtime > stat_histfile_buffer.st_mtime)return HIST_OLD; // size file is older than particle file
  return HIST_OK;
}

/* ------------------ GetSizeFileStatus ------------------------ */

int GetSizeFileStatus(partdata *parti){

  // return -1 if size file cannot be created (corresponding particle file does not exist)
  // return  0 if size file does not need to be created
  // return  1 if size file needs to be created (doesn't exist or is older than corresponding particle file)

  STRUCTSTAT stat_sizefile_buffer, stat_regfile_buffer;
  int stat_sizefile, stat_regfile;

  stat_sizefile = STAT(parti->size_file, &stat_sizefile_buffer);
  stat_regfile = STAT(parti->reg_file, &stat_regfile_buffer);

  if(stat_regfile != 0)return -1;                         // particle file does not exist
  if(stat_sizefile != 0) return 1;                        // size file does not exist
  if(stat_regfile_buffer.st_size > parti->reg_file_size){ // particle file has grown
    parti->reg_file_size = stat_regfile_buffer.st_size;
    return 1;
  }
  if(stat_regfile_buffer.st_mtime > stat_sizefile_buffer.st_mtime)return 1; // size file is older than particle file
  return 0;
}

/* ------------------ ReadPartHistogram ------------------------ */

void ReadPartHistogram(partdata *parti){
  FILE *STREAM_HIST = NULL;
  int i, *buckets = NULL, nbucketsmax = 0;
  float valminmax[2];
  unsigned char *compressed_buckets = NULL;
  int ncompressed_bucketsMAX = 0;
  uLongf ncompressed_buckets, nbuckets, nbuffer;

  STREAM_HIST = fopen(parti->hist_file, "rb");
  if(STREAM_HIST == NULL)return;

  if(parti->histograms == NULL){
    NewMemory((void **)&parti->histograms, parti->nclasses * sizeof(histogramdata *));
    for(i = 0; i < npart5prop; i++){
      NewMemory((void **)&parti->histograms[i], parti->nclasses * sizeof(histogramdata));
    }
  }

  for(i = 0; i < npart5prop; i++){
    histogramdata *histi;

    fread(valminmax, sizeof(float), 2, STREAM_HIST);
    fread(&nbuckets, sizeof(int), 1, STREAM_HIST);
    if(nbuckets > nbucketsmax){
      nbucketsmax = nbuckets;
      FREEMEMORY(buckets);
      NewMemory((void **)&buckets, (1.02*nbucketsmax + 600) * sizeof(int));
    }
    fread(&ncompressed_buckets, sizeof(uLongf), 1, STREAM_HIST);
    if(ncompressed_buckets > ncompressed_bucketsMAX){
      ncompressed_bucketsMAX = ncompressed_buckets;
      FREEMEMORY(compressed_buckets);
      NewMemory((void **)&compressed_buckets, 1.02*ncompressed_bucketsMAX + 600);
    }
    fread(compressed_buckets, sizeof(unsigned char), ncompressed_buckets, STREAM_HIST);

    nbuffer = (1.02*nbucketsmax + 600) * sizeof(int);
    UnCompressZLIB((unsigned char *)buckets, &nbuffer, compressed_buckets, ncompressed_buckets);
    nbuckets = nbuffer / 4;

    histi = parti->histograms[i];
    CopyBuckets2Histogram(buckets, nbuckets, valminmax[0], valminmax[1], histi);
  }
  FREEMEMORY(buckets);
  fclose(STREAM_HIST);
}

/* ------------------ WritePartHistogram ------------------------ */

void WritePartHistogram(partdata *parti){
  FILE *STREAM_HIST = NULL;
  int i;
  unsigned char *compressed_buckets = NULL;
  int ncompressed_bucketsMAX = 0;
  uLongf ncompressed_buckets;

  STREAM_HIST = fopen(parti->hist_file, "wb");
  if(STREAM_HIST == NULL)return;

  for(i = 0; i < npart5prop; i++){
    histogramdata *histi;
    float valminmax[2];

    histi = parti->histograms[i];
    valminmax[0] = histi->val_min;
    valminmax[1] = histi->val_max;

    fwrite(valminmax, sizeof(float), 2, STREAM_HIST);
    fwrite(&histi->nbuckets, sizeof(int), 1, STREAM_HIST);

    if(sizeof(int)*histi->nbuckets > ncompressed_bucketsMAX){
      ncompressed_bucketsMAX = sizeof(int)*histi->nbuckets;
      FREEMEMORY(compressed_buckets);
      NewMemory((void **)&compressed_buckets, 1.02*ncompressed_bucketsMAX + 600);
    }

    ncompressed_buckets = 1.02*ncompressed_bucketsMAX + 600;
    CompressZLIB(compressed_buckets, &ncompressed_buckets, (unsigned char *)histi->buckets, histi->nbuckets * sizeof(unsigned int));

    fwrite(&ncompressed_buckets, sizeof(uLongf), 1, STREAM_HIST);
    fwrite(compressed_buckets, sizeof(unsigned char), ncompressed_buckets, STREAM_HIST);
  }
  fclose(STREAM_HIST);
}

 /* ------------------ CreatePart5SizeFile ------------------------ */
void CreatePart5SizeFile(char *part5file, char *part5sizefile, int angle_flag, int *error){
  FILE *PART5FILE, *streamout;
  int returncode;
  int one, version, nclasses;
  int i;
  int *numtypes, *numpoints;
  int skip, numvals;
  int endianswitch = 0;

  PART5FILE = fopen(part5file, "rb");
  streamout = fopen(part5sizefile, "w");

  FSEEK(PART5FILE, 4, SEEK_CUR); fread(&one, 4, 1, PART5FILE); FSEEK(PART5FILE, 4, SEEK_CUR);
  if(one != 1)endianswitch = 1;
  FORTPART5READ(&version, 1);

  FORTPART5READ(&nclasses, 1);
  NewMemory((void **)&numtypes, 2*nclasses * sizeof(int));
  NewMemory((void **)&numpoints, nclasses * sizeof(int));
  for (i = 0; i < nclasses; i++){
    FORTPART5READ(numtypes+2*i, 2);
    numvals = numtypes[2 * i] + numtypes[2 * i + 1];
    skip = 2*numvals*(4 + 30 + 4);
    FSEEK(PART5FILE, skip, SEEK_CUR);
  }
  while (!feof(PART5FILE)){
    float time;

    FORTPART5READ(&time, 1);
    if(returncode == 0)break;
    for (i = 0; i < nclasses; i++){
      FORTPART5READ(numpoints + i, 1);
      if(angle_flag == 1){
        skip = 4 + 4 * 7 * numpoints[i] + 4;
      }
      else {
        skip = 4 + 4 * 3 * numpoints[i] + 4;
      }
      skip += 4 + 4 * numpoints[i] + 4;
      if(numtypes[2 * i] > 0)    skip += 4 + 4 * numpoints[i] * numtypes[2 * i] + 4;
      if(numtypes[2 * i + 1] > 0)skip += 4 + 4 * numpoints[i] * numtypes[2 * i + 1] + 4;
      FSEEK(PART5FILE, skip, SEEK_CUR);
    }
    fprintf(streamout, "%f\n", time);
    for (i = 0; i < nclasses; i++){
      fprintf(streamout, " %i\n", numpoints[i]);
    }
  }
  fclose(PART5FILE);
  fclose(streamout);
  FREEMEMORY(numtypes);
  FREEMEMORY(numpoints);
}

/* ------------------ GetPartHistogramFile ------------------------ */

void GetPartHistogramFile(partdata *parti){
  int i;
  part5data *datacopy;

  if(parti->histograms == NULL){
    NewMemory((void **)&parti->histograms, npart5prop*sizeof(histogramdata *));
    for(i = 0; i < npart5prop; i++){
      NewMemory((void **)&parti->histograms[i], sizeof(histogramdata));
      InitHistogram(parti->histograms[i], NHIST_BUCKETS, NULL, NULL);
    }
  }
  for(i = 0; i<npart5prop; i++){
    ResetHistogram(parti->histograms[i], NULL, NULL);
  }
  datacopy = parti->data5;
  if(datacopy != NULL){
    for(i = 0; i < parti->ntimes; i++){
      int j;

      for(j = 0; j < parti->nclasses; j++){
        partclassdata *partclassi;
        float *rvals;
        int k;

        partclassi = parti->partclassptr[j];
        rvals = datacopy->rvals;

        if(rvals!=NULL){
          for(k = 2; k<partclassi->ntypes; k++){
            partpropdata *prop_id;
            int partprop_index;

            prop_id = GetPartProp(partclassi->labels[k].longlabel);
            if(prop_id==NULL)continue;

            partprop_index = prop_id-part5propinfo;
            UpdateHistogram(rvals, NULL,datacopy->npoints, parti->histograms[partprop_index]);
            rvals += datacopy->npoints;
          }
        }
        datacopy++;
      }
    }
  }
}

/* ------------------ GetPartData ------------------------ */

void GetPartData(partdata *parti, int partframestep_local, int nf_all, FILE_SIZE *file_size){
  FILE *PART5FILE;
  int one;
  int endianswitch=0;
  int version;
  int nclasses;
  int i;
  int skip_local;
  size_t returncode;
  float time_local;
  int nparts;
  int *numtypes=NULL,*numtypescopy, *numpoints=NULL;
  int numtypes_temp[2];
  char *reg_file;
  part5data *datacopy;
  int count;
  int count2;
  int first_frame=1;

  reg_file=parti->reg_file;

  PART5FILE=fopen(reg_file,"rb");
  if(PART5FILE==NULL)return;

  if(file_size!=NULL)*file_size= GetFileSizeSMV(reg_file);
  FSEEK(PART5FILE,4,SEEK_CUR);fread(&one,4,1,PART5FILE);FSEEK(PART5FILE,4,SEEK_CUR);
  if(one!=1)endianswitch=1;

  FORTPART5READ(&version, 1);
  if(returncode==0)goto wrapup;

  FORTPART5READ(&nclasses,1);
  if(returncode==0)goto wrapup;

  NewMemory((void **)&numtypes,2*nclasses*sizeof(int));
  NewMemory((void **)&numpoints,nclasses*sizeof(int));
  numtypescopy=numtypes;
  numtypes_temp[0]=0;
  numtypes_temp[1]=0;
  CheckMemory;
  for(i=0;i<nclasses;i++){
    FORTPART5READ(numtypes_temp,2);
    if(returncode==0)goto wrapup;
    *numtypescopy++=numtypes_temp[0];
    *numtypescopy++=numtypes_temp[1];
    skip_local = 2*(numtypes_temp[0]+numtypes_temp[1])*(8 + 30);
    returncode=FSEEK(PART5FILE,skip_local,SEEK_CUR);
    if(returncode!=0)goto wrapup;
  }
  CheckMemory;

  datacopy = parti->data5;
  count=0;
  count2=-1;
  for(;;){
    int doit;

    CheckMemory;
    if(count>=nf_all)break;
    FORTPART5READ(&time_local,1);
    if(returncode == 0)break;

    if(count%partframestep_local!=0||(settmin_p==1&&time_local<tmin_p-TEPS)||(settmax_p==1&&time_local>tmax_p+TEPS)){
      doit=0;
    }
    else{
      count2++;
      doit=1;
    }
    count++;

    if(doit==1){
      parti->times[count2]=time_local;
    }
    for(i=0;i<nclasses;i++){
      partclassdata *partclassi;
      int factor=256*128-1;

      partclassi = parti->partclassptr[i];
      FORTPART5READ(&nparts,1);
      if(returncode==0)goto wrapup;
      numpoints[i] = nparts;
      skip_local=0;
      CheckMemory;
      if(doit==1){
        short *sx, *sy, *sz;
        float *xyz;
        float *angle, *width, *depth, *height;
        int j;

        if(parti->evac==1){
          FORTPART5READ(partclassi->xyz,XYZ_EXTRA*nparts);
        }
        else{
          FORTPART5READ(partclassi->xyz,3*nparts);
        }
        CheckMemory;
        if(nparts>0){
          if(returncode==0)goto wrapup;
          xyz = partclassi->xyz;
          sx = datacopy->sx;
          sy = datacopy->sy;
          sz = datacopy->sz;
          if(parti->evac==1){
            angle=datacopy->avatar_angle;
            width=datacopy->avatar_width;
            depth=datacopy->avatar_depth;
            height=datacopy->avatar_height;
          }
          for(j=0;j<nparts;j++){
            float xx, yy, zz;

            xx = NORMALIZE_X(xyz[         j])/xbar;
            yy = NORMALIZE_Y(xyz[  nparts+j])/ybar;
            zz = NORMALIZE_Z(xyz[2*nparts+j])/zbar;

            sx[j] = factor*xx;
            sy[j] = factor*yy;
            sz[j] = factor*zz;
            if(parti->evac==1){
              angle[j] =xyz[j+3*nparts];
              width[j] =xyz[j+4*nparts];
              depth[j] =xyz[j+5*nparts];
              height[j]=xyz[j+6*nparts];
            }
          }
          CheckMemory;
        }
      }
      else{
        if(parti->evac==1){
          skip_local += 4 + XYZ_EXTRA*nparts*sizeof(float) + 4;
        }
        else{
          skip_local += 4 + 3*nparts*sizeof(float) + 4;
        }
      }
      CheckMemory;
      if(doit==1){
        int *sort_tags;
        int j;

        sort_tags=datacopy->sort_tags;
        FORTPART5READ(datacopy->tags,nparts);
        CheckMemory;
        if(nparts>0){
          if(returncode==0)goto wrapup;
          for(j=0;j<nparts;j++){
            sort_tags[2*j]=datacopy->tags[j];
            sort_tags[2*j+1]=j;
          }
          qsort( sort_tags, (size_t)nparts, 2*sizeof(int), CompareTags);
        }
      }
      else{
        skip_local += 4 + nparts*sizeof(int) + 4;  // skip over tag
      }
      CheckMemory;
      if(doit==1){
        if(numtypes[2 * i] > 0){
#ifdef pp_PARTTEST
          int iii, jjj;
#endif

          FORTPART5READ(datacopy->rvals, nparts*numtypes[2 * i]);

#ifdef pp_PARTTEST
          for(jjj = 0; jjj < numtypes[2 * i]; jjj++){
            for(iii = 0; iii < nparts; iii++){
              datacopy->rvals[iii+jjj*nparts] = 1000.0*parti->seq_id + 200*jjj+ (float)RandInt(-1000, 1000) / 1000.0;
            }
          }
#endif
          if(returncode==0)goto wrapup;
        }
      }
      else{
        if(numtypes[2*i]>0){
          skip_local += 4 + nparts*numtypes[2*i]*sizeof(float) + 4;  // skip over vals for now
        }
        //if(numtypes[2*i+1]>0){
        //  skip_local += 4 + nparts*numtypes[2*i+1]*sizeof(float) + 4;
       // }
      }
      CheckMemory;


      returncode=0;
      if(skip_local>0){
        returncode=FSEEK(PART5FILE,skip_local,SEEK_CUR);
        if(returncode!=0)goto wrapup;
      }
      CheckMemory;
      datacopy++;
    }
    CheckMemory;
    if(first_frame==1)first_frame=0;
  }
wrapup:
  UpdateAllPartVis(parti);
  CheckMemory;
  FREEMEMORY(numtypes);
  FREEMEMORY(numpoints);
  fclose(PART5FILE);
}

  /* ------------------ GetHistFileData ------------------------ */

void GetHistFileData(partdata *parti, int partframestep_local, int nf_all){
  FILE *PART5FILE;
  int one;
  int endianswitch = 0;
  int version;
  int nclasses;
  int i;
  int skip_local;
  size_t returncode;
  float time_local;
  int nparts;
  int *numtypes = NULL, *numtypescopy, *numpoints = NULL;
  int numtypes_temp[2];
  char *reg_file;
  int count;
  float *rvals;
  int nrvals;

  reg_file = parti->reg_file;

  nrvals = 100;
  NewMemory((void **)&rvals, nrvals*sizeof(float));

  PART5FILE = fopen(reg_file, "rb");
  if(PART5FILE == NULL)return;

  FSEEK(PART5FILE, 4, SEEK_CUR); fread(&one, 4, 1, PART5FILE); FSEEK(PART5FILE, 4, SEEK_CUR);
  if(one != 1)endianswitch = 1;

  FORTPART5READ(&version, 1); if(returncode == 0)goto wrapup;
  FORTPART5READ(&nclasses, 1); if(returncode == 0)goto wrapup;
  NewMemory((void **)&numtypes, 2 * nclasses*sizeof(int));
  NewMemory((void **)&numpoints, nclasses*sizeof(int));
  numtypescopy = numtypes;
  numtypes_temp[0] = 0;
  numtypes_temp[1] = 0;
  CheckMemory;
  for(i = 0; i < nclasses; i++){
    FORTPART5READ(numtypes_temp, 2);
    if(returncode == 0)goto wrapup;
    *numtypescopy++ = numtypes_temp[0];
    *numtypescopy++ = numtypes_temp[1];
    skip_local = 2 * (numtypes_temp[0] + numtypes_temp[1])*(8 + 30);
    returncode = FSEEK(PART5FILE, skip_local, SEEK_CUR);
    if(returncode != 0)goto wrapup;
  }
  CheckMemory;

  count=0;
  for(;;){
    int doit;

    CheckMemory;
    if(count >= nf_all)break;
    FORTPART5READ(&time_local, 1);
    if(returncode == 0)break;

    if(count%partframestep_local != 0 || (settmin_p == 1 && time_local<tmin_p - TEPS) || (settmax_p == 1 && time_local>tmax_p + TEPS)){
      doit = 0;
    }
    else{
      doit = 1;
    }
    count++;

    for(i = 0; i < nclasses; i++){
      FORTPART5READ(&nparts, 1);
      if(returncode == 0)goto wrapup;
      numpoints[i] = nparts;
      skip_local = 0;
      CheckMemory;
      if(parti->evac == 1){
        skip_local += 4 + XYZ_EXTRA * 4 * nparts + 4;
      }
      else{
        skip_local += 4 + 3 * 4 * nparts + 4;
      }
      skip_local += 4 + 4 * nparts + 4;  // skip over tag for now
      if(skip_local > 0){
        returncode = FSEEK(PART5FILE, skip_local, SEEK_CUR);
        if(returncode != 0)goto wrapup;
      }
      CheckMemory;

      skip_local = 0;
      if(doit == 1){
        if(numtypes[2 * i] > 0){
#ifdef pp_PARTTEST
          int iii, jjj;
#endif

          if(nparts*numtypes[2 * i] > nrvals){
            nrvals = nparts*numtypes[2 * i];
            NewMemory((void **)&rvals, nrvals*sizeof(float));
          }
          FORTPART5READ(rvals, nparts*numtypes[2 * i]);
          if(returncode == 0)goto wrapup;

#ifdef pp_PARTTEST
          for(jjj = 0; jjj < numtypes[2 * i]; jjj++){
            for(iii = 0; iii < nparts; iii++){
              rvals[iii + jjj*nparts] = 1000.0*parti->seq_id + 200 * jjj + (float)RandInt(-1000, 1000) / 1000.0;
            }
          }
#endif
        }
      }
      else{
        if(numtypes[2 * i]>0){
          skip_local += 4 + 4 * nparts*numtypes[2 * i] + 4;  // skip over vals for now
        }
      }
      CheckMemory;
      if(numtypes[2 * i + 1] > 0){
        skip_local += 4 + 4 * nparts*numtypes[2 * i + 1] + 4;
      }

      returncode = 0;
      if(skip_local > 0){
        returncode = FSEEK(PART5FILE, skip_local, SEEK_CUR);
        if(returncode != 0)goto wrapup;
      }
      CheckMemory;
    }
    CheckMemory;
  }
wrapup:
  CheckMemory;
  FREEMEMORY(numtypes);
  FREEMEMORY(numpoints);
  FREEMEMORY(rvals);
  fclose(PART5FILE);
}

/* ------------------ PrintPartProp ------------------------ */

#ifdef _DEBUG
void PrintPartProp(void){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(STRCMP(propi->label->longlabel, "Uniform color")==0){
      PRINTF("label=%s\n", propi->label->longlabel);
    }
    else{
      PRINTF("label=%s min=%f max=%f\n", propi->label->longlabel, propi->valmin, propi->valmax);
      PRINTF("   glbmin=%f glbmax=%f\n", propi->global_min, propi->global_max);
      PRINTF("   permin=%f permax=%f\n", propi->percentile_min, propi->percentile_max);
    }
    PRINTF("\n");
  }
}
#endif


/* ------------------ GetPartPropIndexS ------------------------ */

int GetPartPropIndexS(char *shortlabel){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->shortlabel,shortlabel)==0)return i;
  }
  return -1;
}

/* ------------------ GetPartPropIndex ------------------------ */

int GetPartPropIndex(char *label){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->longlabel,label)==0)return i;
  }
  return 0;
}

/* ------------------ GetPartProp ------------------------ */

partpropdata *GetPartProp(char *label){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->longlabel,label)==0)return propi;
  }
  return NULL;
}

/* ------------------ InitPartProp ------------------------ */

void InitPartProp(void){
  int i,j,k;

  // 0.  only needed if InitPartProp is called more than once
  // (and if so, need to also free memory of each component)

  FREEMEMORY(part5propinfo);
  npart5prop=0;

  // 1.  count max number of distinct variables

  for(i=0;i<npartclassinfo;i++){
    partclassdata *partclassi;

    partclassi = partclassinfo + i;
    npart5prop+=(partclassi->ntypes-1);
  }

  // 2. now count the exact amount and put labels into array just allocated

  if(npart5prop>0){
    NewMemory((void **)&part5propinfo,npart5prop*sizeof(partpropdata));
    npart5prop=0;

    for(i=0;i<npartclassinfo;i++){
      int ii;
      partclassdata *partclassi;

      partclassi = partclassinfo + i;
      for(j=1;j<partclassi->ntypes;j++){
        flowlabels *flowlabel;
        int define_it;

        define_it = 1;
        flowlabel = partclassi->labels + j;
        for(k=0;k<npart5prop;k++){
          partpropdata *propi;
          char *proplabel;

          propi = part5propinfo + k;
          proplabel = propi->label->longlabel;
          if(strcmp(proplabel,flowlabel->longlabel)==0){
            define_it=0;
            break;
          }
        }
        if(define_it==1){
          partpropdata *propi;

          propi = part5propinfo + npart5prop;

          propi->human_property=0;
          propi->particle_property=0;
          propi->label=flowlabel;

          propi->setvalmin=0;
          propi->setvalmax=0;
          propi->set_global_bounds=1;
          propi->global_min=100000000.0;
          propi->global_max=-propi->global_min;
          propi->valmin=1.0;
          propi->valmax=0.0;
          propi->percentile_min=1.0;
          propi->percentile_max=0.0;
          propi->user_min=1.0;
          propi->user_max=0.0;
          propi->display=0;


          propi->setchopmin=0;
          propi->setchopmax=0;
          propi->chopmin=1.0;
          propi->chopmax=0.0;

          propi->buckets=NULL;
          propi->partlabels=NULL;
          NewMemory((void **)&propi->partlabels,256*sizeof(char *));
          for(ii=0;ii<256;ii++){
            char *labeli;

            labeli=NULL;
            NewMemory((void **)&labeli,11);
            propi->partlabels[ii]=labeli;
          }
          NewMemory((void **)&propi->scale,256);
          InitHistogram(&propi->histogram, NHIST_BUCKETS, NULL, NULL);

          npart5prop++;
        }
      }

    }
  }
  for(i=0;i<npart5prop;i++){
    partpropdata *propi;
    int ii;

    propi = part5propinfo + i;

    propi->class_present=NULL;
    propi->class_vis=NULL;
    propi->class_types=NULL;
    NewMemory((void **)&propi->class_types,npartclassinfo*sizeof(unsigned int));
    NewMemory((void **)&propi->class_present,npartclassinfo*sizeof(unsigned char));
    NewMemory((void **)&propi->class_vis,npartclassinfo*sizeof(unsigned char));
    for(ii=0;ii<npartclassinfo;ii++){
      propi->class_vis[ii]=1;
      propi->class_present[ii]=0;
      propi->class_types[ii]=0;
    }
  }
  for(i=0;i<npartclassinfo;i++){
    partclassdata *partclassi;

    partclassi = partclassinfo + i;
    for(j=1;j<partclassi->ntypes;j++){
      flowlabels *flowlabel;
      partpropdata *classprop;

      flowlabel = partclassi->labels + j;
      classprop = GetPartProp(flowlabel->longlabel);
      if(classprop!=NULL){
        if(partclassi->kind==1){
          classprop->human_property=1;
        }
        else{
          classprop->particle_property=1;
        }
        classprop->class_present[i]=1;
        classprop->class_types[i]=j-2;
      }
    }
  }
}

/* ------------------ GetNPartFrames ------------------------ */

int GetNPartFrames(partdata *parti){
  FILE *stream;
  char buffer[256];
  float time_local;
  char *reg_file, *size_file;
  int i;
  int doit = 0;
  int stat_sizefile, stat_regfile;
  STRUCTSTAT stat_sizefile_buffer, stat_regfile_buffer;
  int nframes_all;

  reg_file=parti->reg_file;
  size_file=parti->size_file;

  // if size file doesn't exist then generate it

  stat_sizefile=STAT(size_file,&stat_sizefile_buffer);
  stat_regfile=STAT(reg_file,&stat_regfile_buffer);
  if(stat_regfile!=0)return -1;

  // create a size file if 1) the size does not exist
  //                       2) base file is newer than the size file
  // create_part5sizefile(reg_file,size_file);

  if(stat_regfile_buffer.st_size > parti->reg_file_size){                   // particle file has grown
    parti->reg_file_size = stat_regfile_buffer.st_size;
    doit = 1;
  }
  if(doit==1||stat_sizefile != 0 || stat_regfile_buffer.st_mtime>stat_sizefile_buffer.st_mtime){
    int error;
    int angle_flag=0;

    TrimBack(reg_file);
    TrimBack(size_file);
    if(parti->evac==1){
      angle_flag=1;
      PRINTF("Sizing evac data: %s\n", reg_file);
      CreatePart5SizeFile(reg_file, size_file, angle_flag, &error);
    }
    else{
      angle_flag=0;
      PRINTF("Sizing particle data: %s\n", reg_file);
      CreatePart5SizeFile(reg_file, size_file, angle_flag, &error);
    }
  }

  stream=fopen(size_file,"r");
  if(stream==NULL)return -1;

  nframes_all=0;
  for(;;){
    int exitloop;

    if(fgets(buffer,255,stream)==NULL)break;
    sscanf(buffer,"%f",&time_local);
    exitloop=0;
    for(i=0;i<parti->nclasses;i++){
      if(fgets(buffer,255,stream)==NULL){
        exitloop=1;
        break;
      }
    }
    if(exitloop==1)break;
    nframes_all++;
  }
  fclose(stream);
  return nframes_all;
}

/* ------------------ GetMinPartFrames ------------------------ */

int GetMinPartFrames(int flag){
  int i;
  int min_frames=-1;

  for(i=0;i<npartinfo;i++){
    partdata *parti;
    int nframes;

    parti = partinfo + i;
    if(flag == PARTFILE_LOADALL ||
      (flag == PARTFILE_RELOADALL&&parti->loaded == 1) ||
      (flag >= 0 && i == flag)){

      nframes = GetNPartFrames(parti);
      if(nframes > 0){
        if(min_frames == -1){
          min_frames = nframes;
        }
        else{
          if(nframes != -1 && nframes < min_frames)min_frames = nframes;
        }
      }
    }
  }
  return min_frames;
}

#define FORCE 1
#define NOT_FORCE 0

/* ------------------ GetPartHeader ------------------------ */

void GetPartHeader(partdata *parti, int partframestep_local, int *nf_all, int option, int print_option){
  FILE *stream;
  char buffer[256];
  float time_local;
  int count;
  char *reg_file, *size_file;
  int i;
  int nframes_all;
  int sizefile_status;

  parti->ntimes=0;

  reg_file = parti->reg_file;
  size_file = parti->size_file;

  sizefile_status = GetSizeFileStatus(parti);
  if(sizefile_status == -1)return; // particle file does not exist so cannot be sized
  if(option==FORCE||sizefile_status == 1){        // size file is missing or older than particle file
    int error;
    int angle_flag = 0;

    TrimBack(reg_file);
    TrimBack(size_file);
    if(parti->evac == 1){
      angle_flag = 1;
      if(print_option==1)PRINTF("Sizing evac data: %s\n", reg_file);
      }
    else{
      angle_flag = 0;
      if(print_option==1)PRINTF("Sizing particle data: %s\n", reg_file);
    }
    CreatePart5SizeFile(reg_file, size_file, angle_flag, &error);
  }

  stream=fopen(size_file,"r");
  if(stream==NULL)return;

    // pass 1: count frames

  nframes_all=0;
  for(;;){
    int exitloop;

    if(fgets(buffer,255,stream)==NULL)break;
    sscanf(buffer,"%f",&time_local);
    exitloop=0;
    for(i=0;i<parti->nclasses;i++){
      if(fgets(buffer,255,stream)==NULL||(npartinfo>1&&npartframes_max!=-1&&nframes_all+1>npartframes_max)){
        exitloop=1;
        break;
      }
    }
    if(exitloop==1)break;
    nframes_all++;
    if((nframes_all-1)%partframestep_local!=0||
       (settmin_p!=0&&time_local<tmin_p-TEPS)||
       (settmax_p!=0&&time_local>tmax_p+TEPS)){
       continue;
    }
    (parti->ntimes)++;
  }
  rewind(stream);
  *nf_all = nframes_all;

  // allocate memory for number of time steps * number of classes

  NewMemory((void **)&parti->data5,parti->nclasses*parti->ntimes*sizeof(part5data));
  NewMemory((void **)&parti->times,parti->ntimes*sizeof(float));

  // free memory for x, y, z frame data

  for(i=0;i<parti->nclasses;i++){
    partclassdata *partclassi;

    partclassi = parti->partclassptr[i];
    FREEMEMORY(partclassi->xyz);
    partclassi->maxpoints=0;
  }

  // pass 2 - allocate memory for x, y, z frame data

  {
    part5data *datacopy;
    int fail;

    fail=0;
    count=-1;
    datacopy=parti->data5;
    for(i=0;i<nframes_all;i++){
      int j;

      count++;
      fail=0;
      if(fgets(buffer,255,stream)==NULL){
        fail=1;
        break;
      }
      sscanf(buffer,"%f",&time_local);
      if(count%partframestep_local!=0||
         (settmin_p!=0&&time_local<tmin_p-TEPS)||
         (settmax_p!=0&&time_local>tmax_p+TEPS)){
        for(j=0;j<parti->nclasses;j++){
          if(fgets(buffer,255,stream)==NULL){
            fail=1;
            break;
          }
        }
        if(fail==1)break;
        continue;
      }
      for(j=0;j<parti->nclasses;j++){
        int npoints ,ntypes;

        partclassdata *partclassj;

        datacopy->time = time_local;
        partclassj = parti->partclassptr[j];
        InitPart5Data(datacopy,partclassj);
        if(fgets(buffer,255,stream)==NULL){
          fail=1;
          break;
        }
        sscanf(buffer,"%i",&datacopy->npoints);
        npoints=datacopy->npoints;
        if(npoints>partclassj->maxpoints)partclassj->maxpoints=npoints;
        if(npoints>0){
          NewMemory((void **)&datacopy->tags,npoints*sizeof(int));
          NewMemory((void **)&datacopy->sort_tags,2*npoints*sizeof(int));
          NewMemory((void **)&datacopy->vis_part,npoints*sizeof(unsigned char));
          NewMemory((void **)&datacopy->sx,npoints*sizeof(short));
          NewMemory((void **)&datacopy->sy,npoints*sizeof(short));
          NewMemory((void **)&datacopy->sz,npoints*sizeof(short));
          NewMemory((void **)&datacopy->dsx,npoints*sizeof(float));
          NewMemory((void **)&datacopy->dsy,npoints*sizeof(float));
          NewMemory((void **)&datacopy->dsz,npoints*sizeof(float));
          if(parti->evac==1){
            NewMemory((void **)&datacopy->avatar_angle,npoints*sizeof(float));
            NewMemory((void **)&datacopy->avatar_width,npoints*sizeof(float));
            NewMemory((void **)&datacopy->avatar_depth,npoints*sizeof(float));
            NewMemory((void **)&datacopy->avatar_height,npoints*sizeof(float));
          }
          ntypes = datacopy->partclassbase->ntypes;
          if(ntypes>0){
            NewMemory((void **)&datacopy->rvals, ntypes*npoints*sizeof(float));
            NewMemory((void **)&datacopy->irvals,ntypes*npoints*sizeof(unsigned char));
          }
        }
        datacopy++;
      }
      if(fail==1)break;
    }
    if(fail==1)parti->ntimes=i;
    fclose(stream);
  }

  // allocate memory for x, y, z and tag for the maximum frame size
  //           don't need to allocate memory for all frames

  for(i=0;i<parti->nclasses;i++){
    partclassdata *partclassi;

    partclassi = parti->partclassptr[i];
    if(partclassi->maxpoints>0){
      if(parti->evac==1){
        NewMemory((void **)&partclassi->xyz,XYZ_EXTRA*partclassi->maxpoints*sizeof(float));
      }
      else{
        NewMemory((void **)&partclassi->xyz,3*partclassi->maxpoints*sizeof(float));
      }
    }
  }

}

/* ------------------ UpdatePartColorBounds ------------------------ */

void UpdatePartColorBounds(partdata *parti){
  int j;

  for (j = 0; j < npartinfo; j++) {
    partdata *partj;

    partj = partinfo + j;
    if(partj == parti || (parti->finalize==1&&partj->loaded==1)){
      AdjustPart5Bounds(partj);
    }
  }
  if(colorlabelpart!=NULL){
    NewMemory((void **)&colorlabelpart, MAXRGB*sizeof(char *));
    {
      int n;

      for(n = 0; n<MAXRGB; n++){
        colorlabelpart[n] = NULL;
      }
      for(n = 0; n<nrgb; n++){
        NewMemory((void **)&colorlabelpart[n], 11);
      }
    }
  }
  for(j = 0; j<npartinfo; j++){
    partdata *partj;

    partj = partinfo+j;
    if(partj->loaded==1&&partj->display==1){
      GetPart5Colors(partj, nrgb, PARTFILE_MAP); // make sure there is data
    }
  }
}

    /* -----  ------------- ReadPart ------------------------ */

FILE_SIZE ReadPart(char *file, int ifile, int loadflag, int *errorcode){
  size_t lenfile;
  int error=0;
  partdata *parti;
  int nf_all;
  FILE_SIZE file_size;
  int j;
  float load_time;

  START_TIMER(load_time);
  ASSERT(ifile>=0&&ifile<npartinfo);
  parti=partinfo+ifile;

  FreeAllPart5Data(parti);

  if(parti->loaded==0&&loadflag==UNLOAD)return 0.0;

  *errorcode=0;
  partfilenum=ifile;
  ReadPartFile = 0;
  ReadEvacFile = 0;
  for(j = 0; j<npartinfo; j++){
    partdata *partj;

    partj = partinfo + j;
    if(partj->loaded==1&&partj!=parti&&partj->evac==1){
      ReadEvacFile = 1;
      break;
    }
  }
  for(j = 0; j<npartinfo; j++){
    partdata *partj;

    partj = partinfo + j;
    if(partj->loaded==1&&partj!=parti&&partj->evac==0){
      ReadPartFile = 1;
      break;
    }
  }
  parti->loaded = 0;
  parti->display=0;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  updatemenu=1;

  FREEMEMORY(parti->times);

  if(loadflag==UNLOAD){
    if(parti->finalize == 1){
      UpdatePartColorBounds(parti);
      UpdateTimes();
      updatemenu = 1;
      UpdatePart5Extremes();
      PrintMemoryInfo;
    }
    return 0.0;
  }

  lenfile = strlen(file);
  if(lenfile==0){
    ReadPart("",ifile,UNLOAD,&error);
    UpdateTimes();
    return 0.0;
  }

  PRINTF("Loading %s", file);
  GetPartHeader(parti, partframestep, &nf_all, NOT_FORCE, 1);
  GetPartData(parti, partframestep, nf_all, &file_size);

  PrintMemoryInfo;

  // convert particle temperatures into integers pointing to an rgb color table
  parti->request_load = 1;
  if(parti->finalize == 1){
    for(j = 0;j<npartinfo;j++){
      partdata *partj;

      partj = partinfo+j;
      if(partj->request_load==1){
        partj->request_load = 0;
        partj->loaded = 1;
        partj->display = 1;
      }
    }
    for(j = 0; j<npart5prop; j++){
      partpropdata *propj;

      propj = part5propinfo + j;
      ResetHistogram(&propj->histogram,NULL,NULL);
    }
    for(j = 0; j < npartinfo; j++){
      partdata *partj;
      int i;

      partj = partinfo + j;
      if(partj->loaded == 0)continue;
      GetPartHistogramFile(partj);
      for(i = 0; i < npart5prop; i++){
        partpropdata *propi;

        propi = part5propinfo + i;
        MergeHistogram(&propi->histogram, partj->histograms[i], MERGE_BOUNDS);
      }
    }
    UpdatePartColorBounds(parti);
    UpdateGlui();
    if(parti->evac == 0){
      visParticles = 1;
      ReadPartFile = 1;
    }
    else{
      visEvac = 1;
      ReadEvacFile = 1;
    }

    parttype = 0;
    PartBoundCBInit();
    ParticlePropShowMenu(part5colorindex);
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    UpdateTimes();
    UpdatePart5Extremes();
    updatemenu = 1;
    IdleCB();
    glutPostRedisplay();
    STOP_TIMER(load_time);
  }
  if(file_size>1000000000){
    PRINTF(" - %.1f GB/%.1f s\n", (float)file_size/1000000000., load_time);
  }
  else if(file_size>1000000){
    PRINTF(" - %.1f MB/%.1f s\n", (float)file_size/1000000., load_time);
  }
  else{
    PRINTF(" - %.0f kB/%.1f s\n", (float)file_size/1000., load_time);
  }
  return file_size;
}

/* ----------------------- DrawSelectAvatars ----------------------------- */

void DrawSelectAvatars(void){
  int i;

  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(parti->loaded==0||parti->display==0)continue;
    if(parti->evac==1){
      DrawEvac(parti);
      SNIFF_ERRORS("after DrawEvac");
    }
  }
}

/* ------------------ DrawEvac ------------------------ */

void DrawEvac(const partdata *parti){
  DrawPart(parti);
}

/* ------------------ UpdatePartMenuLabels ------------------------ */

void UpdatePartMenuLabels(void){
  int i;
  partdata *parti;
  char label[128];
  int lenlabel;

  if(npartinfo>0){
    FREEMEMORY(partorderindex);
    NewMemory((void **)&partorderindex,sizeof(int)*npartinfo);
    for(i=0;i<npartinfo;i++){
      partorderindex[i]=i;
    }
    qsort( (int *)partorderindex, (size_t)npartinfo, sizeof(int), ComparePart);

    for(i=0;i<npartinfo;i++){
      parti = partinfo + i;
      STRCPY(parti->menulabel,"");
      if(parti->evac==1){
        STRCAT(parti->menulabel,"humans");
      }
      else{
        STRCAT(parti->menulabel,"particles");
      }
      lenlabel=strlen(parti->menulabel);
      if(nmeshes>1){
        meshdata *partmesh;

        partmesh = meshinfo + parti->blocknumber;
        sprintf(label,"%s",partmesh->label);
        if(lenlabel>0)STRCAT(parti->menulabel,", ");
        STRCAT(parti->menulabel,label);
      }
      if(showfiles==1||lenlabel==0){
        if(nmeshes>1||lenlabel>0)STRCAT(parti->menulabel,", ");
        STRCAT(parti->menulabel,parti->file);
      }
    }
  }
}
