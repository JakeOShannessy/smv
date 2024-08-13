#define INMAIN
#include "options.h"
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "MALLOCC.h"
#include "smokeviewvars.h"
#include "string_util.h"

#include <json-c/json_object.h>

#ifndef _WIN32
#include <libgen.h>
#endif

/// @brief Given a file path, get the filename excluding the final extension.
/// This allocates a new copy which can be deallocated with free().
/// @param input_file a file path
/// @return an allocated string containing the basename or NULL on failure.
char *GetBaseName(const char *input_file) {
  if (input_file == NULL) return NULL;
#ifdef _WIN32
  char *result = malloc(_MAX_FNAME + 1);
  errno_t err =
      _splitpath_s(input_file, NULL, 0, NULL, 0, result, _MAX_FNAME, NULL, 0);
  if (err) return NULL;
#else
  // POSIX basename can modify it's contents, so we'll make some copies.
  char *input_file_temp = strdup(input_file);
  // Get the filename (final component of the path, including any extensions).
  char *bname = basename(input_file_temp);
  // If a '.' exists, set it to '\0' to trim the extension.
  char *dot = strrchr(bname, '.');
  if (dot) *dot = '\0';
  char *result = strdup(bname);
  free(input_file_temp);
#endif
  return result;
}

int SetGlobalFilenames(const char *fdsprefix) {
  int len_casename = strlen(fdsprefix);
  strcpy(movie_name, fdsprefix);
  strcpy(render_file_base, fdsprefix);
  strcpy(html_file_base, fdsprefix);

  FREEMEMORY(log_filename);
  NewMemory((void **)&log_filename, len_casename + strlen(".smvlog") + 1);
  STRCPY(log_filename, fdsprefix);
  STRCAT(log_filename, ".smvlog");

  FREEMEMORY(caseini_filename);
  NewMemory((void **)&caseini_filename, len_casename + strlen(".ini") + 1);
  STRCPY(caseini_filename, fdsprefix);
  STRCAT(caseini_filename, ".ini");

  FREEMEMORY(expcsv_filename);
  NewMemory((void **)&expcsv_filename, len_casename + strlen("_exp.csv") + 1);
  STRCPY(expcsv_filename, fdsprefix);
  STRCAT(expcsv_filename, "_exp.csv");

  FREEMEMORY(dEcsv_filename);
  NewMemory((void **)&dEcsv_filename, len_casename + strlen("_dE.csv") + 1);
  STRCPY(dEcsv_filename, fdsprefix);
  STRCAT(dEcsv_filename, "_dE.csv");

  FREEMEMORY(html_filename);
  NewMemory((void **)&html_filename, len_casename + strlen(".html") + 1);
  STRCPY(html_filename, fdsprefix);
  STRCAT(html_filename, ".html");

  FREEMEMORY(smv_orig_filename);
  NewMemory((void **)&smv_orig_filename, len_casename + strlen(".smo") + 1);
  STRCPY(smv_orig_filename, fdsprefix);
  STRCAT(smv_orig_filename, ".smo");

  FREEMEMORY(hrr_filename);
  NewMemory((void **)&hrr_filename, len_casename + strlen("_hrr.csv") + 1);
  STRCPY(hrr_filename, fdsprefix);
  STRCAT(hrr_filename, "_hrr.csv");

  FREEMEMORY(htmlvr_filename);
  NewMemory((void **)&htmlvr_filename, len_casename + strlen("_vr.html") + 1);
  STRCPY(htmlvr_filename, fdsprefix);
  STRCAT(htmlvr_filename, "_vr.html");

  FREEMEMORY(htmlobst_filename);
  NewMemory((void **)&htmlobst_filename,
            len_casename + strlen("_obst.json") + 1);
  STRCPY(htmlobst_filename, fdsprefix);
  STRCAT(htmlobst_filename, "_obst.json");

  FREEMEMORY(htmlslicenode_filename);
  NewMemory((void **)&htmlslicenode_filename,
            len_casename + strlen("_slicenode.json") + 1);
  STRCPY(htmlslicenode_filename, fdsprefix);
  STRCAT(htmlslicenode_filename, "_slicenode.json");

  FREEMEMORY(htmlslicecell_filename);
  NewMemory((void **)&htmlslicecell_filename,
            len_casename + strlen("_slicecell.json") + 1);
  STRCPY(htmlslicecell_filename, fdsprefix);
  STRCAT(htmlslicecell_filename, "_slicecell.json");

  FREEMEMORY(event_filename);
  NewMemory((void **)&event_filename, len_casename + strlen("_events.csv") + 1);
  STRCPY(event_filename, fdsprefix);
  STRCAT(event_filename, "_events.csv");

  if (ffmpeg_command_filename == NULL) {
    NewMemory((void **)&ffmpeg_command_filename,
              (unsigned int)(len_casename + 12));
    STRCPY(ffmpeg_command_filename, fdsprefix);
    STRCAT(ffmpeg_command_filename, "_ffmpeg");
#ifdef WIN32
    STRCAT(ffmpeg_command_filename, ".bat");
#else
    STRCAT(ffmpeg_command_filename, ".sh");
#endif
  }
#ifdef pp_FED
  if (fed_filename == NULL) {
    STRCPY(fed_filename_base, fdsprefix);
    STRCAT(fed_filename_base, ".fed_smv");
    fed_filename =
        GetFileName(smokeview_scratchdir, fed_filename_base, NOT_FORCE_IN_DIR);
  }
#endif
  if (stop_filename == NULL) {
    NewMemory((void **)&stop_filename,
              (unsigned int)(len_casename + strlen(".stop") + 1));
    STRCPY(stop_filename, fdsprefix);
    STRCAT(stop_filename, ".stop");
  }
  if (smvzip_filename == NULL) {
    NewMemory((void **)&smvzip_filename,
              (unsigned int)(len_casename + strlen(".smvzip") + 1));
    STRCPY(smvzip_filename, fdsprefix);
    STRCAT(smvzip_filename, ".smvzip");
  }
  if (sliceinfo_filename == NULL) {
    NewMemory((void **)&sliceinfo_filename,
              strlen(fdsprefix) + strlen(".sinfo") + 1);
    STRCPY(sliceinfo_filename, fdsprefix);
    STRCAT(sliceinfo_filename, ".sinfo");
  }
  if (deviceinfo_filename == NULL) {
    NewMemory((void **)&deviceinfo_filename,
              strlen(fdsprefix) + strlen("_device.info") + 1);
    STRCPY(deviceinfo_filename, fdsprefix);
    STRCAT(deviceinfo_filename, "_device.info");
  }

  // if smokezip created part2iso files then concatenate .smv entries found in
  // the .isosmv file to the end of the .smv file creating a new .smv file. Then
  // read in that .smv file.

  {
    FILE *stream_iso = NULL;

    NewMemory((void **)&iso_filename, len_casename + strlen(".isosmv") + 1);
    STRCPY(iso_filename, fdsprefix);
    STRCAT(iso_filename, ".isosmv");
    stream_iso = fopen(iso_filename, "r");
    if (stream_iso != NULL) {
      fclose(stream_iso);
    }
    else {
      FREEMEMORY(iso_filename);
    }
  }

  if (trainer_filename == NULL) {
    NewMemory((void **)&trainer_filename, (unsigned int)(len_casename + 6));
    STRCPY(trainer_filename, fdsprefix);
    STRCAT(trainer_filename, ".svd");
  }
  if (test_filename == NULL) {
    NewMemory((void **)&test_filename, (unsigned int)(len_casename + 6));
    STRCPY(test_filename, fdsprefix);
    STRCAT(test_filename, ".svd");
  }
  return 0;
}

// TODO: remove this definition of Zoom2Aperture
float Zoom2Aperture(float zoom0){
  float ap;
  // note tan(46*PI/360)~(W/2)/D  where W=17==monitor width
  //                                D=20==distance from eye to monitor
  // (rounded off to 45 degrees)

  ap = 2.0*RAD2DEG*atan(tan(45.0*DEG2RAD/2.0)/zoom0);
  return ap;
}

// TODO: remove this definition of InitVars
void InitVars(void){
  int i;
  char *queue_list = NULL, *queue=NULL, *htmldir=NULL, *email=NULL;

#ifdef pp_OSX_HIGHRES
  double_scale = 1;
#endif
  curdir_writable = Writable(".");
  windrose_circ.ncirc=0;
  InitCircle(180, &windrose_circ);

  object_circ.ncirc=0;
  cvent_circ.ncirc=0;

//*** define slurm queues

  queue_list = getenv("SMV_QUEUES");
#ifdef pp_MOVIE_BATCH_DEBUG
  if(queue_list==NULL)queue_list = "batch"; // placeholder for debugging slurm queues on the PC
#endif

#define MAX_QUEUS 100
  if(queue_list!=NULL){
    strcpy(movie_queue_list, queue_list);
    queue = strtok(movie_queue_list, ":");
  }
  if(queue!=NULL){
    NewMemory((void **)&movie_queues, MAX_QUEUS*sizeof(char *));
    movie_queues[nmovie_queues++]=TrimFrontBack(queue);
    for(;;){
      queue = strtok(NULL, ":");
      if(queue==NULL||nmovie_queues>=MAX_QUEUS)break;
      movie_queues[nmovie_queues++]=TrimFrontBack(queue);
    }
    ResizeMemory((void **)&movie_queues, nmovie_queues*sizeof(char *));
    have_slurm = 1;
  }

//*** define weburl
  {
    char *hostname = NULL, *username = NULL;

    hostname = getenv("HOSTNAME");
    username = getenv("USER");

    if(hostname!=NULL&&username!=NULL){
      strcpy(movie_url, "http://");
      strcat(movie_url, hostname);
      strcat(movie_url, "/");
      strcat(movie_url, username);
    }
    else{
      strcpy(movie_url, "");
    }
  }

//*** define html directory

  htmldir = getenv("SMV_HTMLDIR");
  if(htmldir!=NULL&&strlen(htmldir)>0){
    strcpy(movie_htmldir, htmldir);
  }
  else{
    strcpy(movie_htmldir, "");
  }

//*** define email address

  email = getenv("SMV_EMAIL");
  if(email!=NULL&&strlen(email)>0){
    strcpy(movie_email, email);
  }
  else{
    strcpy(movie_email, "");
  }

#ifdef pp_RENDER360_DEBUG
  NewMemory((void **)&screenvis, nscreeninfo * sizeof(int));
  for(i = 0; i < nscreeninfo; i++){
    screenvis[i] = 1;
  }
#endif

  beam_color[0] = 255 * foregroundcolor[0];
  beam_color[1] = 255 * foregroundcolor[1];
  beam_color[2] = 255 * foregroundcolor[2];

  if(movie_filetype==WMV){
    strcpy(movie_ext, ".wmv");
  }
  else if(movie_filetype==MP4){
    strcpy(movie_ext, ".mp4");
  }
  else{
    strcpy(movie_ext, ".avi");
  }
  for(i=0;i<200;i++){
    face_id[i]=1;
  }
  for(i=0;i<10;i++){
    face_vis[i]=1;
    face_vis_old[i]=1;
  }
  for(i=0;i<7;i++){
    b_state[i]=-1;
  }
  strcpy((char *)degC,"C");
  strcpy((char *)degF,"F");

  labelscoll.label_first_ptr = &labelscoll.label_first;
  labelscoll.label_last_ptr = &labelscoll.label_last;

  labelscoll.label_first_ptr->prev = NULL;
  labelscoll.label_first_ptr->next = labelscoll.label_last_ptr;
  strcpy(labelscoll.label_first_ptr->name,"first");

  labelscoll.label_last_ptr->prev = labelscoll.label_first_ptr;
  labelscoll.label_last_ptr->next = NULL;
  strcpy(labelscoll.label_last_ptr->name,"last");

  {
    labeldata *gl;

    gl=&LABEL_default;
    gl->rgb[0]=0;
    gl->rgb[1]=0;
    gl->rgb[2]=0;
    gl->rgb[3]=255;
    gl->frgb[0]=0.0;
    gl->frgb[1]=0.0;
    gl->frgb[2]=0.0;
    gl->frgb[3]=1.0;
    gl->tstart_stop[0]=0.0;
    gl->tstart_stop[1]=1.0;
    gl->useforegroundcolor=1;
    gl->show_always=1;
    strcpy(gl->name,"new");
    gl->xyz[0]=0.0;
    gl->xyz[1]=0.0;
    gl->xyz[2]=0.0;
    gl->tick_begin[0] = 0.0;
    gl->tick_begin[1] = 0.0;
    gl->tick_begin[2] = 0.0;
    gl->tick_direction[0] = 1.0;
    gl->tick_direction[1] = 0.0;
    gl->tick_direction[2] = 0.0;
    gl->show_tick = 0;
    gl->labeltype = TYPE_INI;
    memcpy(&LABEL_local,&LABEL_default,sizeof(labeldata));
  }

  strcpy(startup_lang_code,"en");
  mat_specular_orig[0]=0.5f;
  mat_specular_orig[1]=0.5f;
  mat_specular_orig[2]=0.2f;
  mat_specular_orig[3]=1.0f;
  mat_specular2=GetColorPtr(colorcoll, mat_specular_orig);

  mat_ambient_orig[0] = 0.5f;
  mat_ambient_orig[1] = 0.5f;
  mat_ambient_orig[2] = 0.2f;
  mat_ambient_orig[3] = 1.0f;
  mat_ambient2=GetColorPtr(colorcoll, mat_ambient_orig);

  ventcolor_orig[0]=1.0;
  ventcolor_orig[1]=0.0;
  ventcolor_orig[2]=1.0;
  ventcolor_orig[3]=1.0;
  ventcolor=GetColorPtr(colorcoll, ventcolor_orig);

  block_ambient_orig[0] = 1.0;
  block_ambient_orig[1] = 0.8;
  block_ambient_orig[2] = 0.4;
  block_ambient_orig[3] = 1.0;
  block_ambient2=GetColorPtr(colorcoll, block_ambient_orig);

  block_specular_orig[0] = 0.0;
  block_specular_orig[1] = 0.0;
  block_specular_orig[2] = 0.0;
  block_specular_orig[3] = 1.0;
  block_specular2=GetColorPtr(colorcoll, block_specular_orig);

  for(i=0;i<256;i++){
    boundarylevels256[i]=(float)i/255.0;
  }

  first_scriptfile.id=-1;
  first_scriptfile.prev=NULL;
  first_scriptfile.next=&last_scriptfile;

  last_scriptfile.id=-1;
  last_scriptfile.prev=&first_scriptfile;
  last_scriptfile.next=NULL;

  first_inifile.id=-1;
  first_inifile.prev=NULL;
  first_inifile.next=&last_inifile;

  last_inifile.id=-1;
  last_inifile.prev=&first_inifile;
  last_inifile.next=NULL;
  // TODO: why is thei GLUI init here?
  // FontMenu(fontindex);

  direction_color[0]=39.0/255.0;
  direction_color[1]=64.0/255.0;
  direction_color[2]=139.0/255.0;
  direction_color[3]=1.0;
  direction_color_ptr=GetColorPtr(colorcoll, direction_color);

  GetGitInfo(smv_githash,smv_gitdate);

  rgb_terrain[0][0]=1.0;
  rgb_terrain[0][1]=0.0;
  rgb_terrain[0][2]=0.0;
  rgb_terrain[0][3]=1.0;

  rgb_terrain[1][0]=0.5;
  rgb_terrain[1][1]=0.5;
  rgb_terrain[1][2]=0.0;
  rgb_terrain[1][3]=1.0;

  rgb_terrain[2][0]=0.0;
  rgb_terrain[2][1]=1.0;
  rgb_terrain[2][2]=0.0;
  rgb_terrain[2][3]=1.0;

  rgb_terrain[3][0]=0.0;
  rgb_terrain[3][1]=0.5;
  rgb_terrain[3][2]=0.0;
  rgb_terrain[3][3]=1.0;

  rgb_terrain[4][0]=0.0;
  rgb_terrain[4][1]=0.5;
  rgb_terrain[4][2]=0.5;
  rgb_terrain[4][3]=1.0;

  rgb_terrain[5][0]=0.0;
  rgb_terrain[5][1]=0.0;
  rgb_terrain[5][2]=1.0;
  rgb_terrain[5][3]=1.0;

  rgb_terrain[6][0]=0.5;
  rgb_terrain[6][1]=0.0;
  rgb_terrain[6][2]=0.5;
  rgb_terrain[6][3]=1.0;

  rgb_terrain[7][0]=1.0;
  rgb_terrain[7][1]=0.5;
  rgb_terrain[7][2]=0.0;
  rgb_terrain[7][3]=1.0;

  rgb_terrain[8][0]=1.0;
  rgb_terrain[8][1]=0.5;
  rgb_terrain[8][2]=0.5;
  rgb_terrain[8][3]=1.0;

  rgb_terrain[9][0]=1.0;
  rgb_terrain[9][1]=0.25;
  rgb_terrain[9][2]=0.5;
  rgb_terrain[9][3]=1.0;

  strcpy(script_inifile_suffix,"");
  strcpy(script_renderdir,"");
  strcpy(script_renderfilesuffix,"");
  strcpy(script_renderfile,"");
  setpartmin_old=setpartmin;
  setpartmax_old=setpartmax;
  // UpdateCurrentColorbar(colorbarinfo);
  visBlocks=visBLOCKAsInput;
  blocklocation=BLOCKlocation_grid;
  render_window_size=RenderWindow;
  // RenderMenu(render_window_size);
  solidlinewidth=linewidth;
  setbwSAVE=setbw;

  glui_backgroundbasecolor[0] = 255 * backgroundbasecolor[0];
  glui_backgroundbasecolor[1] = 255 * backgroundbasecolor[1];
  glui_backgroundbasecolor[2] = 255 * backgroundbasecolor[2];
  glui_backgroundbasecolor[3] = 255 * backgroundbasecolor[3];

  glui_foregroundbasecolor[0] = 255 * foregroundbasecolor[0];
  glui_foregroundbasecolor[1] = 255 * foregroundbasecolor[1];
  glui_foregroundbasecolor[2] = 255 * foregroundbasecolor[2];
  glui_foregroundbasecolor[3] = 255 * foregroundbasecolor[3];

  strcpy(emptylabel,"");
  font_ptr          = GLUT_BITMAP_HELVETICA_12;
  colorbar_font_ptr = GLUT_BITMAP_HELVETICA_10;
#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      font_ptr = (void *)GLUT_BITMAP_HELVETICA_24;
      colorbar_font_ptr = (void *)GLUT_BITMAP_HELVETICA_20;
    }
#endif

  aperture = Zoom2Aperture(zoom);
  aperture_glui = aperture;
  aperture_default = aperture;

  {
    int ii;
    rgbmask[0]=1;
    for(ii=1;ii<16;ii++){
      rgbmask[ii]=2*rgbmask[ii-1]+1;
    }
  }

  strcpy(ext_png,".png");
  strcpy(ext_jpg,".jpg");
  render_filetype=PNG;

  strcpy(surfacedefaultlabel,"");
  if(streak_index>=0)float_streak5value=streak_rvalue[streak_index];

  objectscoll = CreateObjectCollection();

  GetTitle("Smokeview ", release_title);
  GetTitle("Smokeview ", plot3d_title);

  strcpy(blank_global,"");

  NewMemory((void **)&iso_colors, 4 * MAX_ISO_COLORS*sizeof(float));
  NewMemory((void **)&iso_colorsbw, 4 * MAX_ISO_COLORS*sizeof(float));

#define N_ISO_COLORS 10
  iso_colors[0] = 0.96;
  iso_colors[1] = 0.00;
  iso_colors[2] = 0.96;

  iso_colors[4] = 0.75;
  iso_colors[5] = 0.80;
  iso_colors[6] = 0.80;

  iso_colors[8] = 0.00;
  iso_colors[9] = 0.96;
  iso_colors[10] = 0.28;

  iso_colors[12] = 0.00;
  iso_colors[13] = 0.00;
  iso_colors[14] = 1.00;

  iso_colors[16] = 0.00;
  iso_colors[17] = 0.718750;
  iso_colors[18] = 1.00;

  iso_colors[20] = 0.00;
  iso_colors[21] = 1.0;
  iso_colors[22] = 0.5625;

  iso_colors[24] = 0.17185;
  iso_colors[25] = 1.0;
  iso_colors[26] = 0.0;

  iso_colors[28] = 0.890625;
  iso_colors[29] = 1.0;
  iso_colors[30] = 0.0;

  iso_colors[32] = 1.0;
  iso_colors[33] = 0.380952;
  iso_colors[34] = 0.0;

  iso_colors[36] = 1.0;
  iso_colors[37] = 0.0;
  iso_colors[38] = 0.0;

  glui_iso_transparency = CLAMP(255 * iso_transparency+0.1, 1, 255);
  for(i = 0; i < N_ISO_COLORS; i++){
    iso_colors[4 * i + 3] = iso_transparency;
  }

  for(i = N_ISO_COLORS; i<MAX_ISO_COLORS; i++){
    int grey;

    grey=1.0-(float)(i-N_ISO_COLORS)/(float)(MAX_ISO_COLORS+1-N_ISO_COLORS);
    iso_colors[4*i+0]=grey;
    iso_colors[4*i+1]=grey;
    iso_colors[4*i+2]=grey;
    iso_colors[4 * i + 3] = iso_transparency;
  }

  for(i = 0; i < MAX_ISO_COLORS; i++){
    float graylevel;

    graylevel = TOBW(iso_colors+4*i);
    iso_colorsbw[4*i+0] = graylevel;
    iso_colorsbw[4*i+1] = graylevel;
    iso_colorsbw[4*i+2] = graylevel;
    iso_colorsbw[4*i+3] = 1.0;
  }
  CheckMemory;

  ncolortableinfo = 2;
  if(ncolortableinfo>0){
    colortabledata *cti;

    NewMemory((void **)&colortableinfo, ncolortableinfo*sizeof(colortabledata));

    cti = colortableinfo+0;
    cti->color[0] = 210;
    cti->color[1] = 180;
    cti->color[2] = 140;
    cti->color[3] = 255;
    strcpy(cti->label, "tan");

    cti = colortableinfo+1;
    cti->color[0] = 178;
    cti->color[1] = 34;
    cti->color[2] = 34;
    cti->color[3] = 255;
    strcpy(cti->label, "firebrick");
  }

  memcpy(rgb_base,rgb_baseBASE,MAXRGB*4*sizeof(float));
  memcpy(bw_base,bw_baseBASE,MAXRGB*4*sizeof(float));
  memcpy(rgb2,rgb2BASE,MAXRGB*3*sizeof(float));
  memcpy(bw_base,bw_baseBASE,MAXRGB*4*sizeof(float));

  strcpy(viewpoint_label_startup,"external");
  {
    int iii;

    for(iii=0;iii<7;iii++){
      vis_boundary_type[iii]=1;
    }
    vis_boundary_type[0]=1;
    for(iii=0;iii<MAXPLOT3DVARS;iii++){
      p3min_all[iii]    = 1.0f;
      p3chopmin[iii]    = 1.0f;
      p3max_all[iii]    = 1.0f;
      p3chopmax[iii]    = 0.0f;
    }
  }
}

void FreeVars(void) {
  FreeObjectCollection(objectscoll);
}

int RunBenchmark(char *input_file) {
  initMALLOC();
  InitVars();
  SetGlobalFilenames(fdsprefix);

  INIT_PRINT_TIMER(parse_time);
  fprintf(stderr, "reading:\t%s\n", input_file);
  {
    bufferstreamdata *smv_streaminfo = GetSMVBuffer(input_file);
    if (smv_streaminfo == NULL) {
      fprintf(stderr, "could not open %s\n", input_file);
      return 1;
    }
    INIT_PRINT_TIMER(ReadSMV_time);
    int return_code = ReadSMV(smv_streaminfo);
    STOP_TIMER(ReadSMV_time);
    fprintf(stderr, "ReadSMV:\t%8.3f ms\n", ReadSMV_time * 1000);
    if (smv_streaminfo != NULL) {
      FCLOSE(smv_streaminfo);
    }
    if (return_code) return return_code;
  }
  show_timings = 1;
  ReadSMVOrig();
  INIT_PRINT_TIMER(ReadSMVDynamic_time);
  // ReadSMVDynamic(input_file);
  STOP_TIMER(ReadSMVDynamic_time);
  fprintf(stderr, "ReadSMVDynamic:\t%8.3f ms\n", ReadSMVDynamic_time * 1000);
  STOP_TIMER(parse_time);
  fprintf(stderr, "Total Time:\t%8.3f ms\n", parse_time * 1000);
  struct json_object *jobj = json_object_new_object();
  json_object_object_add(jobj, "version", json_object_new_int(1));
  json_object_object_add(jobj, "chid", json_object_new_string(chidfilebase));
  if (fds_title != NULL) {
    json_object_object_add(jobj, "title", json_object_new_string(fds_title));
  }
  json_object_object_add(jobj, "fds_version",
                         json_object_new_string(fds_version));
  struct json_object *mesh_array = json_object_new_array();
  for (int i = 0; i < meshescoll.nmeshes; i++) {
    meshdata *mesh = &meshescoll.meshinfo[i];
    struct json_object *mesh_obj = json_object_new_object();
    json_object_object_add(mesh_obj, "index", json_object_new_int(i + 1));
    if (mesh->label != NULL) {
      json_object_object_add(mesh_obj, "id",
                             json_object_new_string(mesh->label));
    }
    struct json_object *mesh_coordinates = json_object_new_object();
    json_object_object_add(mesh_coordinates, "i",
                           json_object_new_int(mesh->ibar));
    json_object_object_add(mesh_coordinates, "j",
                           json_object_new_int(mesh->jbar));
    json_object_object_add(mesh_coordinates, "k",
                           json_object_new_int(mesh->kbar));
    json_object_object_add(mesh_obj, "coordinates", mesh_coordinates);
    struct json_object *mesh_dimensions = json_object_new_object();
    json_object_object_add(mesh_dimensions, "x_min",
                           json_object_new_double(mesh->x0));
    json_object_object_add(mesh_dimensions, "x_max",
                           json_object_new_double(mesh->x1));
    json_object_object_add(mesh_dimensions, "y_min",
                           json_object_new_double(mesh->y0));
    json_object_object_add(mesh_dimensions, "y_ax",
                           json_object_new_double(mesh->y1));
    json_object_object_add(mesh_dimensions, "z_min",
                           json_object_new_double(mesh->z0));
    json_object_object_add(mesh_dimensions, "z_max",
                           json_object_new_double(mesh->z1));
    json_object_object_add(mesh_obj, "dimensions", mesh_dimensions);
    json_object_array_add(mesh_array, mesh_obj);
  }
  json_object_object_add(jobj, "meshes", mesh_array);

  // TODO: the parse rejects CSV files that it doesn't find in it's own working
  // directory.
  struct json_object *csv_files = json_object_new_array();
  for (int i = 0; i < ncsvfileinfo; i++) {
    csvfiledata *csv_file = &csvfileinfo[i];
    struct json_object *csv_obj = json_object_new_object();
    json_object_object_add(csv_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(csv_obj, "filename",
                           json_object_new_string(csv_file->file));
    json_object_object_add(csv_obj, "type",
                           json_object_new_string(csv_file->c_type));
    json_object_array_add(csv_files, csv_obj);
  }
  json_object_object_add(jobj, "csv_files", csv_files);

  // Add devices to JSON
  struct json_object *devices = json_object_new_array();
  for (int i = 0; i < ndeviceinfo; i++) {
    devicedata *device = &deviceinfo[i];
    struct json_object *device_obj = json_object_new_object();
    json_object_object_add(device_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(device_obj, "id",
                           json_object_new_string(device->deviceID));
    json_object_object_add(device_obj, "csvlabel",
                           json_object_new_string(device->csvlabel));
    json_object_object_add(device_obj, "label",
                           json_object_new_string(device->labelptr));
    json_object_object_add(device_obj, "quantity",
                           json_object_new_string(device->quantity));
    if (device->have_xyz) {
      struct json_object *device_position = json_object_new_object();
      json_object_object_add(device_position, "x",
                             json_object_new_double(device->xyz[0]));
      json_object_object_add(device_position, "y",
                             json_object_new_double(device->xyz[1]));
      json_object_object_add(device_position, "z",
                             json_object_new_double(device->xyz[2]));
      json_object_object_add(device_obj, "position", device_position);
    }
    if (device->act_times != NULL) {
      struct json_object *state_changes = json_object_new_array();
      for (int j = 0; j < device->nstate_changes; j++) {
        struct json_object *state_change = json_object_new_object();
        json_object_object_add(state_change, "time",
                               json_object_new_double(device->act_times[j]));
        json_object_object_add(state_change, "value",
                               json_object_new_int(device->state_values[j]));
        json_object_array_add(state_changes, state_change);
      }
      json_object_object_add(device_obj, "state_changes", state_changes);
    }
    json_object_array_add(devices, device_obj);
  }
  json_object_object_add(jobj, "devices", devices);

  // Add slices to JSON
  struct json_object *slices = json_object_new_array();
  for (int i = 0; i < nsliceinfo; i++) {
    slicedata *slice = &sliceinfo[i];
    struct json_object *slice_obj = json_object_new_object();
    json_object_object_add(slice_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(slice_obj, "mesh",
                           json_object_new_int(slice->blocknumber));
    if (slice->label.longlabel != NULL) {
      json_object_object_add(slice_obj, "longlabel",
                             json_object_new_string(slice->label.longlabel));
    }
    if (slice->label.shortlabel) {
      json_object_object_add(slice_obj, "shortlabel",
                             json_object_new_string(slice->label.shortlabel));
    }
    if (slice->slicelabel) {
      json_object_object_add(slice_obj, "id",
                             json_object_new_string(slice->slicelabel));
    }
    if (slice->label.unit) {
      json_object_object_add(slice_obj, "unit",
                             json_object_new_string(slice->label.unit));
    }
    struct json_object *coordinates = json_object_new_object();
    json_object_object_add(coordinates, "i_min",
                           json_object_new_int(slice->ijk_min[0]));
    json_object_object_add(coordinates, "i_max",
                           json_object_new_int(slice->ijk_max[0]));
    json_object_object_add(coordinates, "j_min",
                           json_object_new_int(slice->ijk_min[1]));
    json_object_object_add(coordinates, "j_max",
                           json_object_new_int(slice->ijk_max[1]));
    json_object_object_add(coordinates, "k_min",
                           json_object_new_int(slice->ijk_min[2]));
    json_object_object_add(coordinates, "k_max",
                           json_object_new_int(slice->ijk_max[2]));
    json_object_object_add(slice_obj, "coordinates", coordinates);
    json_object_array_add(slices, slice_obj);
  }
  json_object_object_add(jobj, "slices", slices);

  // Add surfaces to JSON
  struct json_object *surfaces = json_object_new_array();
  for (int i = 0; i < nsurfinfo; i++) {
    surfdata *surf = &surfinfo[i];
    struct json_object *surf_obj = json_object_new_object();
    json_object_object_add(surf_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(surf_obj, "id",
                           json_object_new_string(surf->surfacelabel));
    json_object_array_add(surfaces, surf_obj);
  }
  json_object_object_add(jobj, "surfaces", surfaces);

  // Add materials to JSON
  struct json_object *materials = json_object_new_array();
  for (int i = 0; i < nsurfinfo; i++) {
    surfdata *surf = &surfinfo[i];
    struct json_object *surf_obj = json_object_new_object();
    json_object_object_add(surf_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(surf_obj, "id",
                           json_object_new_string(surf->surfacelabel));
    json_object_array_add(materials, surf_obj);
  }
  json_object_object_add(jobj, "surfaces", materials);

  const char *json_output =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  printf("%s\n", json_output);
  json_object_put(jobj);
  FreeVars();
  return 0;
}

int main(int argc, char **argv) {

  bool print_help = false;
  bool print_version = false;

  int c;

  opterr = 0;

  while ((c = getopt(argc, argv, "hV")) != -1)
    switch (c) {
    case 'h':
      print_help = true;
      break;
    case 'V':
      print_version = true;
      break;
    case '?':
      if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      return 1;
    default:
      abort();
    }
  if (print_help) {
    printf("smvq-%s\n", PROGVERSION);
    printf("\nUsage:  smvq [OPTIONS] <FILE>\n");
    printf("\nOptions:\n");
    printf("  -h Print help\n");
    printf("  -V Print version\n");
    return 0;
  }
  if (print_version) {
    printf("smvq - smv query processor (v%s)\n", PROGVERSION);
    return 0;
  }
  char *input_file = argv[optind];

  if (input_file == NULL) {
    fprintf(stderr, "No input file specified.\n");
    return 1;
  }
  fdsprefix = GetBaseName(input_file);
  int result = RunBenchmark(input_file);
  if (fdsprefix != NULL) free(fdsprefix);
  return result;
}
