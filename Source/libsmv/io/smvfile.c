#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// #include "stdio_buffer.h"
#include "datadefs.h"
#include "smokeviewdefs.h"
#include "smvfile.h"

#define BREAK break
#define BREAK2                                                                 \
  if ((stream == stream1 && stream2 == NULL) || stream == stream2) break;      \
  stream = stream2;                                                            \
  continue

#define COLOR_INVISIBLE -2

#define BLOCK_OUTLINE 2

#define DEVICE_DEVICE 0
#define DEVICE_HEAT 2
#define DEVICE_SPRK 3
#define DEVICE_SMOKE 4

#define ZVENT_1ROOM 1
#define ZVENT_2ROOM 2

#define MATCH 1
#define NOTMATCH 0

char *TrimFront(char *line) {

  //  returns first non-blank character at the begininn of line

  char *c;

  for (c = line; c <= line + strlen(line) - 1; c++) {
    if (!isspace((unsigned char)(*c))) return c;
  }
  return line;
}

void TrimBack(char *line) {

  //  removes trailing white space from the character string line

  char *c;
  size_t len;

  if (line == NULL) return;
  len = strlen(line);
  if (len == 0) return;
  for (c = line + len - 1; c >= line; c--) {
    if (isspace((unsigned char)(*c))) continue;
    *(c + 1) = '\0';
    return;
  }
  *line = '\0';
}

/* ------------------ TrimFrontBack ------------------------ */

char *TrimFrontBack(char *buffer) {

  //  removes trailing blanks from buffer and returns a pointer to the first
  //  non-blank character

  if (buffer == NULL) return NULL;
  TrimBack(buffer);
  return TrimFront(buffer);
}

int Match(char *buffer, const char *key) {
  size_t lenbuffer;
  size_t lenkey;

  lenkey = strlen(key);
  lenbuffer = strlen(buffer);
  if (lenbuffer < lenkey)
    return NOTMATCH; // buffer shorter than key so no match
  if (strncmp(buffer, key, lenkey) != 0)
    return NOTMATCH; // key doesn't match buffer so no match
  if (lenbuffer > lenkey && !isspace((unsigned char)buffer[lenkey]))
    return NOTMATCH;
  return MATCH;
}

char *GetStringPtr(char *buffer) {
  int len_buffer;
  int first = -1, last = -1, i;

  if (buffer == NULL) return NULL;
  if (strlen(buffer) == 0) return NULL;

  for (i = 0; i < strlen(buffer); i++) {
    if (buffer[i] != ' ') {
      first = i;
      break;
    }
  }
  if (first < 0) return NULL;

  for (i = strlen(buffer) - 1; i >= 0; i--) {
    if (buffer[i] != ' ') {
      last = i;
      break;
    }
  }
  if (last <= 0) return NULL;

  len_buffer = 1 + last - first;
  char *bufferptr = malloc(len_buffer + 1);
  for (i = 0; i < len_buffer; i++) {
    bufferptr[i] = buffer[first + i];
  }
  bufferptr[len_buffer] = 0;
  return bufferptr;
}

/* ------------------ MatchSMV ------------------------ */

int MatchSMV(char *buffer, const char *key) { return Match(buffer, key); }

/* ------------------ ReadSMV ------------------------ */

int parse_hoc(SmvFile *smvfile, FILE *stream) {
  char buffer[255];
  if (fgets(buffer, 255, stream) == NULL) return 1;
  sscanf(buffer, "%zi", &(smvfile->nfuelinfo));
  if (smvfile->fuelinfo == NULL) {
    smvfile->fuelinfo = malloc(smvfile->nfuelinfo * sizeof(fueldata));
  } else {
    smvfile->fuelinfo =
        realloc(smvfile->fuelinfo, smvfile->nfuelinfo * sizeof(fueldata));
  }

  for (size_t i = 0; i < smvfile->nfuelinfo; i++) {
    if (fgets(buffer, 255, stream) == NULL) return 1;
    fueldata *fueli = smvfile->fuelinfo + i;
    sscanf(buffer, "%f", &(fueli->hoc));
  }
  return 0;
}

int parse_fuel(SmvFile *smvfile, FILE *stream) {
  char buffer[255];
  if (fgets(buffer, 255, stream) == NULL) return 1;
  int n = sscanf(buffer, "%zi", &(smvfile->nfuelinfo));
  if (n < 1) {
    return 1;
  }
  if (smvfile->fuelinfo == NULL) {
    smvfile->fuelinfo = malloc(smvfile->nfuelinfo * sizeof(fueldata));
  } else {
    smvfile->fuelinfo =
        realloc(smvfile->fuelinfo, smvfile->nfuelinfo * sizeof(fueldata));
  }

  for (size_t i = 0; i < smvfile->nfuelinfo; i++) {
    if (fgets(buffer, 255, stream) == NULL) return 1;
    fueldata *fueli = smvfile->fuelinfo + i;
    fueli->fuel = GetStringPtr(buffer);
  }
  return 0;
}

int parse_title(SmvFile *smvfile, FILE *stream) {
  char buffer[255];
  if (fgets(buffer, 255, stream) == NULL) return 1;
  char *fds_title_local = TrimFrontBack(buffer);
  if (fds_title_local == NULL) return 2;
  size_t len_title = strlen(fds_title_local);
  if (len_title == 0) return 2;
  smvfile->fds_title = malloc(len_title + 1);
  strcpy(smvfile->fds_title, fds_title_local);
  return 0;
}

int parse_fds_version(SmvFile *smvfile, FILE *stream) {
  char buffer[255];
  if (fgets(buffer, 255, stream) == NULL) return 1;
  char *s = TrimFrontBack(buffer);
  if (s == NULL) return 2;
  size_t len = strlen(s);
  // If version is empty, set it to a default.
  if (len == 0) {
    s = "unknown";
    len = strlen(s);
  }
  smvfile->fds_version = malloc(len + 1);
  strcpy(smvfile->fds_version, s);
  return 0;
}

typedef struct _colordata {
  float color[4], full_color[4], bw_color[4];
  struct _colordata *nextcolor;
} colordata;

int parse_surface(SmvFile *smvfile, FILE *stream) {
  char buffer[255];
  // If length is equal to capacity, we need more capacity.
  if (smvfile->nsurfinfo >= smvfile->cap_surfinfo) {
    if (smvfile->cap_surfinfo == 0) {
      smvfile->surfinfo = NULL;
    }
    size_t new_cap =
        (smvfile->cap_surfinfo == 0) ? 1 : smvfile->cap_surfinfo * 2;
    smvfile->surfinfo = realloc(smvfile->surfinfo, new_cap * sizeof(surfdata));
    if (smvfile->surfinfo == NULL) {
      fprintf(stderr, "failed to expand array");
      return 1;
    };
    smvfile->cap_surfinfo = new_cap;
  }

  surfdata *surfi = smvfile->surfinfo + smvfile->nsurfinfo;
  if (fgets(buffer, 255, stream) == NULL) return 1;
  char *surfacelabel = TrimFrontBack(buffer);
  size_t len = strlen(surfacelabel);
  surfi->surfacelabel = malloc((len + 1) * sizeof(char));
  strcpy(surfi->surfacelabel, surfacelabel);
  if (fgets(buffer, 255, stream) == NULL) return 1;
  sscanf(buffer, "%f %f", &(surfi->temp_ignition), &(surfi->emis));
  if (fgets(buffer, 255, stream) == NULL) return 1;
  sscanf(buffer, "%i %f %f %f %f %f %f", &(surfi->type), &(surfi->t_width),
         &(surfi->t_height), &(surfi->color[0]), &(surfi->color[1]),
         &(surfi->color[2]), &(surfi->color[3]));

  surfi->geom_surf_color[0] = CLAMP(255 * surfi->color[0], 0, 255);
  surfi->geom_surf_color[1] = CLAMP(255 * surfi->color[1], 0, 255);
  surfi->geom_surf_color[2] = CLAMP(255 * surfi->color[2], 0, 255);

  if (fgets(buffer, 255, stream) == NULL) return 1;
  TrimBack(buffer);
  char *buffer3 = TrimFront(buffer);
  if (MatchSMV(buffer3, "null") == 1) {
    surfi->texture = NULL;
  } else {
    surfi->texture = malloc((strlen(buffer3) + 1) * sizeof(char));
    strcpy(surfi->texture, buffer3);
  }
  smvfile->nsurfinfo++;
  return 0;
}

int ReadSMV(SmvFile *smvfile, FILE *stream) {
  char buffer[255];
  int error = 0;
  while (!feof(stream)) {
    if (fgets(buffer, 255, stream) == NULL) break;
    TrimBack(buffer);
    if (strncmp(buffer, " ", 1) == 0 || buffer[0] == 0) continue;
    if (MatchSMV(buffer, "PL3D") == 1) {
      continue;
    }
    if (MatchSMV(buffer, "HoC") == 1) {
      error = parse_hoc(smvfile, stream);
      if (error) return error;
      continue;
    }
    if (MatchSMV(buffer, "FUEL") == 1) {
      error = parse_fuel(smvfile, stream);
      if (error) return error;
      continue;
    }
    if (MatchSMV(buffer, "TITLE") == 1) {
      error = parse_title(smvfile, stream);
      if (error) return error;
      continue;
    }
    if (MatchSMV(buffer, "SOLID_HT3D") == 1) {
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &solid_ht3d);
      //   ONEORZERO(solid_ht3d);
      //   if (solid_ht3d == 1) show_slice_in_obst = GAS_AND_SOLID;
      continue;
    }
    if (MatchSMV(buffer, "IBLANK") == 1) {
      //   fgets(buffer, 255, stream);
      //   if (iblank_set_on_commandline == 0) {
      //     sscanf(buffer, "%i", &use_iblank);
      //     use_iblank = CLAMP(use_iblank, 0, 1);
      //   }
      continue;
    }
    if (MatchSMV(buffer, "GVEC") == 1) {
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%f %f %f", gvecphys, gvecphys + 1, gvecphys + 2);
      //   gvecunit[0] = gvecphys[0];
      //   gvecunit[1] = gvecphys[1];
      //   gvecunit[2] = gvecphys[2];
      //   NORMALIZE3(gvecunit);
      //   if (NORM3(gvecphys) > 0.0) {
      //     have_gvec = 1;
      //   }
      continue;
    }
    if (MatchSMV(buffer, "SMOKEDIFF") == 1) {
      //   smokediff = 1;
      continue;
    }
    if (MatchSMV(buffer, "ALBEDO") == 1) {
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%f", &smoke_albedo);
      //   smoke_albedo = CLAMP(smoke_albedo, 0.0, 1.0);
      //   smoke_albedo_base = smoke_albedo;
      continue;
    }
    if (MatchSMV(buffer, "NORTHANGLE") == 1) {
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%f", &northangle);
      //   northangle = CLAMP(northangle, -180.0, 180.0);
      //   have_northangle = 1;
      continue;
    }
    if (MatchSMV(buffer, "AUTOTERRAIN") == 1) {
      //   int len_buffer;
      //   char *buff2;

      //   is_terrain_case = 1;
      //   auto_terrain = 1;
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &visTerrainType);
      //   visTerrainType = CLAMP(visTerrainType, 0, 4);
      //   if (visTerrainType == TERRAIN_HIDDEN) {
      //     if (visOtherVents != visOtherVentsSAVE)
      //       visOtherVents = visOtherVentsSAVE;
      //   } else {
      //     if (visOtherVents != 0) {
      //       visOtherVentsSAVE = visOtherVents;
      //       visOtherVents = 0;
      //     }
      //   }
      //   update_terrain_type = 1;
      //   fgets(buffer, 255, stream);
      //   buff2 = TrimFront(buffer);
      //   TrimBack(buff2);
      //   len_buffer = strlen(buff2);
      //   if (len_buffer > 0 && strcmp(buff2, "null") != 0) {
      //     nterrain_textures = 1;
      //     NewMemory((void **)&terrain_textures, sizeof(texturedata));
      //     NewMemory((void **)&(terrain_textures->file),
      //               (len_buffer + 1) * sizeof(char));
      //     strcpy(terrain_textures->file, buff2);
      //   }
      //   have_auto_terrain_image = 1;
      //   continue;
    }
    if (MatchSMV(buffer, "TERRAINIMAGE") == 1) {
      //   int len_buffer;
      //   char *buff2, *blank;

      //   is_terrain_case = 1;
      //   if (have_auto_terrain_image == 1) {
      //     FREEMEMORY(terrain_textures->file);
      //     FREEMEMORY(terrain_textures);
      //   }
      //   nterrain_textures = 1;
      //   blank = strchr(buffer, ' ');
      //   if (blank != NULL) {
      //     int nvals = 0;

      //     sscanf(blank + 1, "%i", &nvals);
      //     if (nvals != 0) nterrain_textures = MAX(nvals, 0);
      //   }

      //   if (nterrain_textures > 0) {
      //     NewMemory((void **)&terrain_textures,
      //               nterrain_textures * sizeof(texturedata));

      //     for (i = 0; i < nterrain_textures; i++) {
      //       fgets(buffer, 255, stream);
      //       buff2 = TrimFrontBack(buffer);
      //       len_buffer = strlen(buff2);
      //       if (len_buffer > 0 && strcmp(buff2, "null") != 0) {
      //         NewMemory((void **)&terrain_textures[i].file,
      //                   (len_buffer + 1) * sizeof(char));
      //         strcpy(terrain_textures[i].file, buff2);
      //       }
      //     }
      //   }
      continue;
    }
    if (MatchSMV(buffer, "FDSVERSION") == 1) {
      error = parse_fds_version(smvfile, stream);
      if (error) return error;
      continue;
    }
    if (MatchSMV(buffer, "TOFFSET") == 1) {
      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   sscanf(buffer, "%f %f %f", texture_origin, texture_origin + 1,
      //          texture_origin + 2);
      continue;
    }

    if (MatchSMV(buffer, "USETEXTURES") == 1) {
      //   usetextures = 1;
      continue;
    }

    if (MatchSMV(buffer, "CADTEXTUREPATH") == 1 ||
        MatchSMV(buffer, "TEXTUREDIR") == 1) {
      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   TrimBack(buffer);
      //   {
      //     size_t texturedirlen;

      //     texturedirlen = strlen(TrimFront(buffer));
      //     if (texturedirlen > 0) {
      //       FREEMEMORY(texturedir);
      //       NewMemory((void **)&texturedir, texturedirlen + 1);
      //       strcpy(texturedir, TrimFront(buffer));
      //     }
      //   }
      continue;
    }

    if (MatchSMV(buffer, "VIEWTIMES") == 1) {
      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   sscanf(buffer, "%f %f %i", &tour_tstart, &tour_tstop, &tour_ntimes);
      //   global_tbegin = tour_tstart;
      //   tload_begin = tour_tstart;

      //   global_tend = tour_tstop;
      //   tload_end = tour_tstop;
      //   if (tour_ntimes < 2) tour_ntimes = 2;
      //   ReallocTourMemory();
      continue;
    }

    //*** SLCF

    if ((MatchSMV(buffer, "SLCF") == 1) || (MatchSMV(buffer, "SLCC") == 1) ||
        (MatchSMV(buffer, "SLCD") == 1) || (MatchSMV(buffer, "SLCT") == 1) ||
        (MatchSMV(buffer, "BNDS") == 1)) {
      //   int return_val;

      //   return_val = ParseSLCFCount(NO_SCAN, stream, buffer, &nslicefiles);
      //   if (return_val == RETURN_BREAK) {
      //     BREAK;
      //   }
      //   if (return_val == RETURN_CONTINUE) {
      //     continue;
      //   }
      continue;
    }

    // if (fds_version == NULL) {
    //   NewMemory((void **)&fds_version, 7 + 1);
    //   strcpy(fds_version, "unknown");
    // }
    // if (fds_githash == NULL) {
    //   NewMemory((void **)&fds_githash, 7 + 1);
    //   strcpy(fds_githash, "unknown");
    // }
    // if (nisoinfo > 0 && nmeshes > 0)
    //   nisos_per_mesh = MAX(nisoinfo / nmeshes, 1);
    // NewMemory((void **)&csvfileinfo,
    //           (ncsvfileinfo + CFAST_CSV_MAX + 1) * sizeof(csvfiledata));
    // ncsvfileinfo = 0;
    // if (ngeominfo > 0) {
    //   NewMemory((void **)&geominfo, ngeominfo * sizeof(geomdata));
    //   ngeominfo = 0;
    // }
    // if (ncgeominfo > 0) {
    //   NewMemory((void **)&cgeominfo, ncgeominfo * sizeof(geomdata));
    //   ncgeominfo = 0;
    // }
    // if (npropinfo > 0) {
    //   NewMemory((void **)&propinfo, npropinfo * sizeof(propdata));
    //   npropinfo = 1; // the 0'th prop is the default human property
    // }
    // if (nterraininfo > 0) {
    //   NewMemory((void **)&terraininfo, nterraininfo * sizeof(terraindata));
    //   nterraininfo = 0;
    // }
    // if (npartclassinfo >= 0) {
    //   float rgb_class[4];
    //   partclassdata *partclassi;
    //   size_t len;

    //   NewMemory((void **)&partclassinfo,
    //             (npartclassinfo + 1) * sizeof(partclassdata));

    //   // define a dummy class

    //   partclassi = partclassinfo + npartclassinfo;
    //   strcpy(buffer, "Default");
    //   TrimBack(buffer);
    //   len = strlen(buffer);
    //   partclassi->name = NULL;
    //   if (len > 0) {
    //     NewMemory((void **)&partclassi->name, len + 1);
    //     STRCPY(partclassi->name, TrimFront(buffer));
    //   }

    //   rgb_class[0] = 1.0;
    //   rgb_class[1] = 0.0;
    //   rgb_class[2] = 0.0;
    //   rgb_class[3] = 1.0;
    //   partclassi->rgb = GetColorPtr(rgb_class);

    //   partclassi->ntypes = 0;
    //   partclassi->xyz = NULL;
    //   partclassi->maxpoints = 0;
    //   partclassi->labels = NULL;

    //   NewMemory((void **)&partclassi->labels, sizeof(flowlabels));
    //   CreateNullLabel(partclassi->labels);

    //   npartclassinfo = 0;
    // }

    //     ibartemp = 2;
    //     jbartemp = 2;
    //     kbartemp = 2;

    //     /* --------- set up multi-block data structures ------------- */

    //     /*
    //        The keywords TRNX, TRNY, TRNZ, GRID, PDIM, OBST and VENT are not
    //        required BUT if any one is present then the number of each must be
    //        equal
    //     */

    //     if (nmeshes == 0 && ntrnx == 0 && ntrny == 0 && ntrnz == 0 && npdim
    //     == 0 &&
    //         nOBST == 0 && nVENT == 0 && noffset == 0) {
    //       nmeshes = 1;
    //       ntrnx = 1;
    //       ntrny = 1;
    //       ntrnz = 1;
    //       npdim = 1;
    //       nOBST = 1;
    //       noffset = 1;
    //     } else {
    //       if (nmeshes > 1) {
    //         if ((nmeshes != ntrnx || nmeshes != ntrny || nmeshes != ntrnz ||
    //              nmeshes != npdim || nmeshes != nOBST || nmeshes != nVENT ||
    //              nmeshes != noffset) &&
    //             (nCVENT != 0 && nCVENT != nmeshes)) {
    //           fprintf(stderr, "*** Error:\n");
    //           if (nmeshes != ntrnx)
    //             fprintf(stderr,
    //                     "*** Error:  found %i TRNX keywords, was expecting
    //                     %i\n", ntrnx, nmeshes);
    //           if (nmeshes != ntrny)
    //             fprintf(stderr,
    //                     "*** Error:  found %i TRNY keywords, was expecting
    //                     %i\n", ntrny, nmeshes);
    //           if (nmeshes != ntrnz)
    //             fprintf(stderr,
    //                     "*** Error:  found %i TRNZ keywords, was expecting
    //                     %i\n", ntrnz, nmeshes);
    //           if (nmeshes != npdim)
    //             fprintf(stderr,
    //                     "*** Error:  found %i PDIM keywords, was expecting
    //                     %i\n", npdim, nmeshes);
    //           if (nmeshes != nOBST)
    //             fprintf(stderr,
    //                     "*** Error:  found %i OBST keywords, was expecting
    //                     %i\n", nOBST, nmeshes);
    //           if (nmeshes != nVENT)
    //             fprintf(stderr,
    //                     "*** Error:  found %i VENT keywords, was expecting
    //                     %i\n", nVENT, nmeshes);
    //           if (nCVENT != 0 && nmeshes != nCVENT)
    //             fprintf(stderr,
    //                     "*** Error:  found %i CVENT keywords, was expecting
    //                     %i\n", noffset, nmeshes);
    //           return 2;
    //         }
    //       }
    //     }
    //     FREEMEMORY(meshinfo);
    //     if (NewMemory((void **)&meshinfo, nmeshes * sizeof(meshdata)) == 0)
    //       return 2;
    //     FREEMEMORY(supermeshinfo);
    //     if (NewMemory((void **)&supermeshinfo, nmeshes *
    //     sizeof(supermeshdata)) ==
    //         0)
    //       return 2;
    //     meshinfo->plot3dfilenum = -1;
    //     UpdateCurrentMesh(meshinfo);
    //     for (i = 0; i < nmeshes; i++) {
    //       meshdata *meshi;
    //       supermeshdata *smeshi;

    //       smeshi = supermeshinfo + i;
    //       smeshi->nmeshes = 0;

    //       meshi = meshinfo + i;
    //       meshi->ibar = 0;
    //       meshi->jbar = 0;
    //       meshi->kbar = 0;
    //       meshi->nbptrs = 0;
    //       meshi->nvents = 0;
    //       meshi->ncvents = 0;
    //       meshi->plotn = 1;
    //       meshi->itextureoffset = 0;
    //     }
    //     if (setPDIM == 0) {
    //       meshdata *meshi;

    //       if (roomdefined == 0) {
    //         xbar0 = 0.0;
    //         xbar = 1.0;
    //         ybar0 = 0.0;
    //         ybar = 1.0;
    //         zbar0 = 0.0;
    //         zbar = 1.0;
    //       }
    //       meshi = meshinfo;
    //       meshi->xyz_bar0[XXX] = xbar0;
    //       meshi->xyz_bar[XXX] = xbar;
    //       meshi->xcen = (xbar + xbar0) / 2.0;
    //       meshi->xyz_bar0[YYY] = ybar0;
    //       meshi->xyz_bar[YYY] = ybar;
    //       meshi->ycen = (ybar + ybar0) / 2.0;
    //       meshi->xyz_bar0[ZZZ] = zbar0;
    //       meshi->xyz_bar[ZZZ] = zbar;
    //       meshi->zcen = (zbar + zbar0) / 2.0;
    //     }

    //     // define labels and memory for default colorbars

    //     FREEMEMORY(partinfo);
    //     if (npartinfo != 0) {
    //       if (NewMemory((void **)&partinfo, npartinfo * sizeof(partdata)) ==
    //       0)
    //         return 2;
    //     }

    //     FREEMEMORY(vsliceinfo);
    //     FREEMEMORY(sliceinfo);
    //     FREEMEMORY(fedinfo);
    //     if (nsliceinfo > 0) {
    //       if (NewMemory((void **)&vsliceinfo,
    //                     3 * nsliceinfo * sizeof(vslicedata)) == 0 ||
    //           NewMemory((void **)&sliceinfo, nsliceinfo * sizeof(slicedata))
    //           == 0 || NewMemory((void **)&fedinfo, nsliceinfo *
    //           sizeof(feddata)) == 0 || NewMemory((void **)&slice_loadstack,
    //           nsliceinfo * sizeof(int)) == 0 || NewMemory((void
    //           **)&vslice_loadstack, nsliceinfo * sizeof(int)) ==
    //               0 ||
    //           NewMemory((void **)&subslice_menuindex, nsliceinfo *
    //           sizeof(int)) ==
    //               0 ||
    //           NewMemory((void **)&msubslice_menuindex, nsliceinfo *
    //           sizeof(int)) ==
    //               0 ||
    //           NewMemory((void **)&subvslice_menuindex, nsliceinfo *
    //           sizeof(int)) ==
    //               0 ||
    //           NewMemory((void **)&msubvslice_menuindex, nsliceinfo *
    //           sizeof(int)) ==
    //               0 ||
    //           NewMemory((void **)&mslice_loadstack, nsliceinfo * sizeof(int))
    //           ==
    //               0 ||
    //           NewMemory((void **)&mvslice_loadstack, nsliceinfo *
    //           sizeof(int)) ==
    //               0) {
    //         return 2;
    //       }
    //       sliceinfo_copy = sliceinfo;
    //       nslice_loadstack = nsliceinfo;
    //       islice_loadstack = 0;
    //       nvslice_loadstack = nsliceinfo;
    //       ivslice_loadstack = 0;
    //       nmslice_loadstack = nsliceinfo;
    //       imslice_loadstack = 0;
    //       nmvslice_loadstack = nsliceinfo;
    //       imvslice_loadstack = 0;
    //     }
    //     if (nsmoke3dinfo > 0) {
    //       if (NewMemory((void **)&smoke3dinfo,
    //                     nsmoke3dinfo * sizeof(smoke3ddata)) == 0)
    //         return 2;
    //     }

    //     FREEMEMORY(patchinfo);
    //     FREEMEMORY(boundarytypes);
    //     if (npatchinfo != 0) {
    //       if (NewMemory((void **)&patchinfo, npatchinfo * sizeof(patchdata))
    //       == 0)
    //         return 2;
    //       for (i = 0; i < npatchinfo; i++) {
    //         patchdata *patchi;

    //         patchi = patchinfo + i;
    //         patchi->reg_file = NULL;
    //         patchi->comp_file = NULL;
    //         patchi->file = NULL;
    //         patchi->size_file = NULL;
    //       }
    //       if (NewMemory((void **)&boundarytypes, npatchinfo * sizeof(int)) ==
    //       0)
    //         return 2;
    //     }
    //     FREEMEMORY(isoinfo);
    //     FREEMEMORY(isotypes);
    //     if (nisoinfo > 0) {
    //       if (NewMemory((void **)&isoinfo, nisoinfo * sizeof(isodata)) == 0)
    //         return 2;
    //       if (NewMemory((void **)&isotypes, nisoinfo * sizeof(int)) == 0)
    //       return 2;
    //     }
    //     FREEMEMORY(roominfo);
    //     if (nrooms > 0) {
    //       if (NewMemory((void **)&roominfo, (nrooms + 1) * sizeof(roomdata))
    //       == 0)
    //         return 2;
    //     }
    //     FREEMEMORY(fireinfo);
    //     if (nfires > 0) {
    //       if (NewMemory((void **)&fireinfo, nfires * sizeof(firedata)) == 0)
    //         return 2;
    //     }
    //     FREEMEMORY(zoneinfo);
    //     if (nzoneinfo > 0) {
    //       if (NewMemory((void **)&zoneinfo, nzoneinfo * sizeof(zonedata)) ==
    //       0)
    //         return 2;
    //     }
    //     FREEMEMORY(zventinfo);
    //     if (nzventsnew > 0) nzvents = nzventsnew;
    //     if (nzvents > 0) {
    //       if (NewMemory((void **)&zventinfo, nzvents * sizeof(zventdata)) ==
    //       0)
    //         return 2;
    //     }
    //     nzvents = 0;
    //     nzhvents = 0;
    //     nzvvents = 0;
    //     nzmvents = 0;

    //     FREEMEMORY(textureinfo);
    //     FREEMEMORY(surfinfo);
    //     if (NewMemory((void **)&surfinfo,
    //                   (nsurfinfo + MAX_ISO_COLORS + 1) * sizeof(surfdata)) ==
    //                   0)
    //       return 2;

    //     {
    //       matldata *matli;
    //       float s_color[4];

    //       FREEMEMORY(matlinfo);
    //       if (NewMemory((void **)&matlinfo, nmatlinfo * sizeof(matldata)) ==
    //       0)
    //         return 2;
    //       matli = matlinfo;
    //       InitMatl(matli);
    //       s_color[0] = matli->color[0];
    //       s_color[1] = matli->color[1];
    //       s_color[2] = matli->color[2];
    //       s_color[3] = matli->color[3];
    //       matli->color = GetColorPtr(s_color);
    //     }

    //     if (cadgeominfo != NULL) FreeCADInfo();
    //     if (ncadgeom > 0) {
    //       if (NewMemory((void **)&cadgeominfo, ncadgeom *
    //       sizeof(cadgeomdata)) == 0)
    //         return 2;
    //     }

    //     if (noutlineinfo > 0) {
    //       if (NewMemory((void **)&outlineinfo,
    //                     noutlineinfo * sizeof(outlinedata)) == 0)
    //         return 2;
    //       for (i = 0; i < noutlineinfo; i++) {
    //         outlinedata *outlinei;

    //         outlinei = outlineinfo + i;
    //         outlinei->x1 = NULL;
    //         outlinei->x2 = NULL;
    //         outlinei->y1 = NULL;
    //         outlinei->y2 = NULL;
    //         outlinei->z1 = NULL;
    //         outlinei->z2 = NULL;
    //       }
    //     }
    //     if (ntickinfo > 0) {
    //       if (NewMemory((void **)&tickinfo, ntickinfo * sizeof(tickdata)) ==
    //       0)
    //         return 2;
    //       ntickinfo = 0;
    //       ntickinfo_smv = 0;
    //     }

    //     if (npropinfo > 0) {
    //       npropinfo = 0;
    //       InitDefaultProp();
    //       npropinfo = 1;
    //     }

    //     if (noGRIDpresent == 1 && startpass == 1) {
    //       strcpy(buffer, "GRID");
    //       startpass = 0;
    //     } else {
    //       if (fgets(buffer, 255, stream) == NULL) {
    //         BREAK;
    //       }
    //       TrimBack(buffer);
    //       if (strncmp(buffer, " ", 1) == 0 || buffer[0] == 0) continue;
    //     }
    //     if (MatchSMV(buffer, "PL3D") == 1) {
    //       continue;
    //     }
    //     /*
    //     +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //     +++++++++++++++++++++++++ HVACVALS ++++++++++++++++++++++++++
    //     +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //   */
    if (MatchSMV(buffer, "HVACVALS") == 1) {
      //   FREEMEMORY(hvacductvalsinfo);
      //   NewMemory((void **)&hvacductvalsinfo, sizeof(hvacvalsdata));
      //   hvacductvalsinfo->times = NULL;
      //   hvacductvalsinfo->loaded = 0;
      //   hvacductvalsinfo->node_vars = NULL;
      //   hvacductvalsinfo->duct_vars = NULL;

      //   if (fgets(buffer, 255, stream) == NULL) BREAK;
      //   hvacductvalsinfo->file = GetCharPtr(TrimFrontBack(buffer));

      //   if (fgets(buffer, 255, stream) == NULL) BREAK;
      //   sscanf(buffer, "%i", &hvacductvalsinfo->n_node_vars);

      //   if (hvacductvalsinfo->n_node_vars > 0) {
      //     NewMemory((void **)&hvacductvalsinfo->node_vars,
      //               hvacductvalsinfo->n_node_vars * sizeof(hvacvaldata));
      //     for (i = 0; i < hvacductvalsinfo->n_node_vars; i++) {
      //       hvacvaldata *hi;
      //       flowlabels *labeli;

      //       hi = hvacductvalsinfo->node_vars + i;
      //       InitHvacData(hi);
      //       labeli = &hi->label;
      //       ReadLabels(labeli, stream, NULL);
      //     }
      //   }

      //   if (fgets(buffer, 255, stream) == NULL) BREAK;
      //   sscanf(buffer, "%i", &hvacductvalsinfo->n_duct_vars);

      //   if (hvacductvalsinfo->n_duct_vars > 0) {
      //     NewMemory((void **)&hvacductvalsinfo->duct_vars,
      //               hvacductvalsinfo->n_duct_vars * sizeof(hvacvaldata));
      //     for (i = 0; i < hvacductvalsinfo->n_duct_vars; i++) {
      //       hvacvaldata *hi;
      //       flowlabels *labeli;

      //       hi = hvacductvalsinfo->duct_vars + i;
      //       InitHvacData(hi);
      //       labeli = &hi->label;
      //       ReadLabels(labeli, stream, NULL);
      //     }
      //   }
      //   FREEMEMORY(hvacnodevalsinfo);
      //   NewMemory((void **)&hvacnodevalsinfo, sizeof(hvacvalsdata));
      //   memcpy(hvacnodevalsinfo, hvacductvalsinfo, sizeof(hvacvalsdata));
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ HVAC ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if (MatchSMV(buffer, "HVAC") == 1) {
      //   // HVAC
      //   //  NODES
      //   //  n_nodes
      //   if (fgets(buffer, 255, stream) == NULL) BREAK;
      //   if (fgets(buffer, 255, stream) == NULL) BREAK;
      //   sscanf(buffer, "%i", &nhvacnodeinfo);
      //   nhvacnodeinfo = MAX(nhvacnodeinfo, 0);
      //   if (nhvacnodeinfo == 0) continue;

      //   FREEMEMORY(hvacnodeinfo);
      //   NewMemory((void **)&hvacnodeinfo, nhvacnodeinfo *
      //   sizeof(hvacnodedata));

      //   // node_id duct_label network_label
      //   // x y z filter_flag vent_label

      //   for (i = 0; i < nhvacnodeinfo; i++) {
      //     hvacnodedata *nodei;
      //     char *filter, *node_label, *network_label, *connect_id;

      //     nodei = hvacnodeinfo + i;

      //     if (fgets(buffer, 255, stream) == NULL) BREAK;
      //     sscanf(buffer, "%i", &nodei->node_id);
      //     TrimBack(buffer);
      //     strtok(buffer, "%");
      //     node_label = strtok(NULL, "%");
      //     network_label = strtok(NULL, "%");
      //     connect_id = strtok(NULL, "%");
      //     nodei->node_name = GetCharPtr(node_label);
      //     network_label = TrimFrontBack(network_label);
      //     if (strcmp(network_label, "null") == 0) {
      //       nodei->network_name = GetCharPtr("Unassigned");
      //     } else {
      //       nodei->network_name = GetCharPtr(network_label);
      //     }
      //     nodei->duct = NULL;
      //     nodei->connect_id = -1;
      //     if (connect_id != NULL) sscanf(connect_id, "%i",
      //     &nodei->connect_id);

      //     if (fgets(buffer, 255, stream) == NULL) BREAK;
      //     sscanf(buffer, "%f %f %f", nodei->xyz, nodei->xyz + 1, nodei->xyz +
      //     2); memcpy(nodei->xyz_orig, nodei->xyz, 3 * sizeof(float));
      //     strtok(buffer, "%");
      //     filter = strtok(NULL, "%");
      //     filter = TrimFrontBack(filter);
      //     nodei->filter = HVAC_FILTER_NO;
      //     strcpy(nodei->c_filter, "");
      //     if (filter != NULL && strcmp(filter, "FILTER") == 0) {
      //       nodei->filter = HVAC_FILTER_YES;
      //       strcpy(nodei->c_filter, "FI");
      //       nhvacfilters++;
      //     }
      //   }
      //   // DUCTS
      //   // n_ducts
      //   // duct_id node_id1 node_id2 duct_name network_name
      //   // fan_type
      //   // duct_name
      //   // component Fan(F), Aircoil(A), Damper(D), none(-)
      //   // waypoint xyz
      //   if (fgets(buffer, 255, stream) == NULL) BREAK;
      //   if (fgets(buffer, 255, stream) == NULL) BREAK;
      //   sscanf(buffer, "%i", &nhvacductinfo);
      //   nhvacductinfo = MAX(nhvacductinfo, 0);
      //   if (nhvacductinfo == 0) {
      //     FREEMEMORY(hvacnodeinfo);
      //     nhvacnodeinfo = 0;
      //     break;
      //   }

      //   FREEMEMORY(hvacductinfo);
      //   NewMemory((void **)&hvacductinfo, nhvacductinfo *
      //   sizeof(hvacductdata)); for (i = 0; i < nhvacductinfo; i++) {
      //     hvacductdata *ducti;
      //     char *duct_label, *network_label, *hvac_label, *connect_id;
      //     int j;

      //     ducti = hvacductinfo + i;
      //     if (fgets(buffer, 255, stream) == NULL) BREAK;
      //     sscanf(buffer, "%i %i %i", &ducti->duct_id, &ducti->node_id_from,
      //            &ducti->node_id_to);
      //     ducti->node_id_from--;
      //     ducti->node_id_to--;

      //     ducti->node_from = hvacnodeinfo + ducti->node_id_from;
      //     ducti->node_to = hvacnodeinfo + ducti->node_id_to;

      //     if (ducti->node_from->duct == NULL) ducti->node_from->duct = ducti;
      //     if (ducti->node_to->duct == NULL) ducti->node_to->duct = ducti;

      //     strtok(buffer, "%");
      //     duct_label = strtok(NULL, "%");
      //     network_label = strtok(NULL, "%");
      //     connect_id = strtok(NULL, "%");
      //     ducti->duct_name = GetCharPtr(duct_label);
      //     network_label = TrimFrontBack(network_label);
      //     if (strcmp(network_label, "null") == 0) {
      //       ducti->network_name = GetCharPtr("Unassigned");
      //     } else {
      //       ducti->network_name = GetCharPtr(network_label);
      //     }
      //     ducti->act_times = NULL;
      //     ducti->act_states = NULL;
      //     ducti->nact_times = 0;
      //     ducti->metro_path = DUCT_XYZ;
      //     ducti->connect_id = -1;
      //     ducti->xyz_reg = NULL;
      //     ducti->cell_met = NULL;
      //     ducti->cell_reg = NULL;
      //     ducti->nxyz_met = 0;
      //     ducti->nxyz_reg = 0;
      //     ducti->xyz_met_cell = NULL;
      //     ducti->xyz_reg_cell = NULL;
      //     ducti->nxyz_met_cell = 0;
      //     ducti->nxyz_reg_cell = 0;

      //     if (connect_id != NULL) sscanf(connect_id, "%i",
      //     &ducti->connect_id);

      //     if (fgets(buffer, 255, stream) == NULL) BREAK;
      //     if (fgets(buffer, 255, stream) == NULL) BREAK;
      //     sscanf(buffer, "%i", &ducti->nduct_cells);
      //     strtok(buffer, "%");
      //     hvac_label = strtok(NULL, "%");
      //     hvac_label = TrimFrontBack(hvac_label);

      //     char *c_component[4] = {"-", "F", "A", "D"};
      //     ducti->component = HVAC_NONE;
      //     if (hvac_label != NULL) {
      //       if (hvac_label[0] == 'F') ducti->component = HVAC_FAN;
      //       if (hvac_label[0] == 'A') ducti->component = HVAC_AIRCOIL;
      //       if (hvac_label[0] == 'D') ducti->component = HVAC_DAMPER;
      //     }
      //     if (ducti->component != HVAC_NONE) nhvaccomponents++;
      //     strcpy(ducti->c_component, c_component[ducti->component]);

      //     if (fgets(buffer, 255, stream) == NULL) BREAK;
      //     if (fgets(buffer, 255, stream) == NULL) BREAK;
      //     int n_waypoints;
      //     sscanf(buffer, "%i", &n_waypoints);

      //     float *waypoints;

      //     NewMemory((void **)&waypoints, 3 * (n_waypoints + 2) *
      //     sizeof(float)); ducti->xyz_reg = waypoints; ducti->nxyz_reg =
      //     n_waypoints + 2;

      //     hvacnodedata *node_from, *node_to;
      //     float *xyz0, *xyz1;

      //     node_from = hvacnodeinfo + ducti->node_id_from;
      //     node_to = hvacnodeinfo + ducti->node_id_to;
      //     xyz0 = node_from->xyz;
      //     xyz1 = node_to->xyz;

      //     memcpy(ducti->xyz_reg, xyz0, 3 * sizeof(float)); // first point
      //     memcpy(ducti->xyz_reg + 3 * (n_waypoints + 1), xyz1,
      //            3 * sizeof(float)); // last point

      //     waypoints += 3;
      //     for (j = 0; j < n_waypoints;
      //          j++) { // points between first and last point
      //       if (fgets(buffer, 255, stream) == NULL) BREAK;
      //       sscanf(buffer, "%f %f %f", waypoints, waypoints + 1, waypoints +
      //       2); waypoints += 3;
      //     }
      //   }
      //   char **hvac_network_labels = NULL;

      //   NewMemory((void **)&hvac_network_labels,
      //             (nhvacnodeinfo + nhvacductinfo) * sizeof(char *));
      //   for (i = 0; i < nhvacnodeinfo; i++) {
      //     hvac_network_labels[i] = hvacnodeinfo[i].network_name;
      //   }
      //   for (i = 0; i < nhvacductinfo; i++) {
      //     hvac_network_labels[i + nhvacnodeinfo] =
      //     hvacductinfo[i].network_name;
      //   }
      //   qsort((char *)hvac_network_labels,
      //         (size_t)(nhvacnodeinfo + nhvacductinfo), sizeof(char *),
      //         CompareLabel);
      //   nhvacinfo = 1;
      //   for (i = 1; i < nhvacnodeinfo + nhvacductinfo; i++) {
      //     if (strcmp(hvac_network_labels[nhvacinfo - 1],
      //                hvac_network_labels[i]) == 0)
      //       continue;
      //     hvac_network_labels[nhvacinfo] = hvac_network_labels[i];
      //     nhvacinfo++;
      //   }
      //   NewMemory((void **)&hvacinfo, nhvacinfo * sizeof(hvacdata));
      //   for (i = 0; i < nhvacinfo; i++) {
      //     hvacdata *hvaci;

      //     hvaci = hvacinfo + i;
      //     hvaci->network_name = hvac_network_labels[i];
      //     hvaci->display = 0;
      //     hvaci->show_node_labels = 0;
      //     hvaci->show_duct_labels = 0;
      //     hvaci->show_filters = NODE_FILTERS_HIDE;
      //     hvaci->show_component = DUCT_COMPONENT_HIDE;
      //     hvaci->component_size = 1.0;
      //     hvaci->filter_size = 1.0;
      //     hvaci->node_size = 8.0;
      //     hvaci->cell_node_size = 8.0;
      //     hvaci->duct_width = 4.0;
      //     memcpy(hvaci->node_color, hvac_node_color, 3 * sizeof(int));
      //     memcpy(hvaci->duct_color, hvac_duct_color, 3 * sizeof(int));
      //   }
      //   FREEMEMORY(hvac_network_labels);
      //   SetHVACInfo();
    }
    /*
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  +++++++++++++++++++++++++++++ CSVF ++++++++++++++++++++++++++
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
    if (MatchSMV(buffer, "CSVF") == 1) {

      //           char *file_ptr;
      //   char buffer2[256];

      //   fgets(buffer, 255, stream);
      //   TrimBack(buffer);

      //   fgets(buffer2, 255, stream);
      //   TrimBack(buffer2);
      //   file_ptr = TrimFront(buffer2);
      //   if (FILE_EXISTS_CASEDIR(file_ptr) == YES) ncsvfileinfo++;
      //   continue;
      //   csvfiledata *csvi;
      //   char *type_ptr, *file_ptr;
      //   char buffer2[256];

      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   TrimBack(buffer);
      //   type_ptr = TrimFront(buffer);

      //   if (fgets(buffer2, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   TrimBack(buffer2);
      //   file_ptr = TrimFront(buffer2);
      //   if (FILE_EXISTS_CASEDIR(file_ptr) == NO) continue;

      //   csvi = csvfileinfo + ncsvfileinfo;
      //   InitCSV(csvi, file_ptr, type_ptr, CSV_FDS_FORMAT);

      //   ncsvfileinfo++;
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++ TIMES +++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "TIMES") == 1) {
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%f %f", &global_tbegin, &global_tend);
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++ BOXGEOM ++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "BOXGEOM") == 1) {
      //   int nbounds = 0;

      //   TrimBack(buffer);
      //   if (strlen(buffer) > 7) {
      //     sscanf(buffer + 7, "%i", &nbounds);
      //   }
      //   if (nbounds > 0) {
      //     float *bounds;

      //     NewMemory((void **)&bounds, 6 * nbounds * sizeof(float));
      //     for (i = 0; i < nbounds; i++) {
      //       float *xyz;

      //       xyz = bounds + 6 * i;
      //       fgets(buffer, 255, stream);
      //       sscanf(buffer, "%f %f %f %f %f %f", xyz, xyz + 1, xyz + 2, xyz +
      //       3,
      //              xyz + 4, xyz + 5);
      //     }
      //     if (ngeominfo > 0) {
      //       geomdata *geomi;

      //       geomi = geominfo + ngeominfo - 1;
      //       for (i = 0; i < MIN(nbounds, geomi->ngeomobjinfo); i++) {
      //         geomobjdata *geomobji;

      //         geomobji = geomi->geomobjinfo + i;
      //         geomobji->bounding_box = bounds + 6 * i;
      //       }
      //     }
      //   }
    }

    /*
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++ CGEOM ++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
    if (MatchSMV(buffer, "CGEOM") == 1) {
      //   geomdata *geomi;
      //   char *buff2;
      //   int have_vectors = CFACE_NORMALS_NO;

      //   geomi = cgeominfo + ncgeominfo;
      //   buff2 = buffer + 6;
      //   sscanf(buff2, "%i", &have_vectors);
      //   if (have_vectors != CFACE_NORMALS_YES) have_vectors =
      //   CFACE_NORMALS_NO; if (have_vectors == CFACE_NORMALS_YES)
      //     have_cface_normals = CFACE_NORMALS_YES;
      //   InitGeom(geomi, GEOM_CGEOM, FDSBLOCK, have_vectors);
      //   geomi->memory_id = ++nmemory_ids;

      //   fgets(buffer, 255, stream);
      //   TrimBack(buffer);
      //   buff2 = TrimFront(buffer);
      //   NewMemory((void **)&geomi->file, strlen(buff2) + 1);
      //   strcpy(geomi->file, buff2);
      //   ncgeominfo++;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ GEOM ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if (MatchSMV(buffer, "GEOM") == 1 || MatchSMV(buffer, "SGEOM") == 1) {
      //   geomdata *geomi;
      //   char *buff2;
      //   int ngeomobjinfo = 0;
      //   int is_geom = 0;

      //   geomi = geominfo + ngeominfo;
      //   geomi->ngeomobjinfo = 0;
      //   geomi->geomobjinfo = NULL;
      //   geomi->memory_id = ++nmemory_ids;

      //   TrimBack(buffer);
      //   if (strlen(buffer) > 4) {

      //     buff2 = buffer + 5;
      //     sscanf(buff2, "%i", &ngeomobjinfo);
      //   }
      //   if (MatchSMV(buffer, "SGEOM") == 1) {
      //     InitGeom(geomi, GEOM_SLICE, NOT_FDSBLOCK, CFACE_NORMALS_NO);
      //   } else {
      //     is_geom = 1;
      //     InitGeom(geomi, GEOM_GEOM, FDSBLOCK, CFACE_NORMALS_NO);
      //   }

      //   fgets(buffer, 255, stream);
      //   TrimBack(buffer);
      //   buff2 = TrimFront(buffer);
      //   NewMemory((void **)&geomi->file, strlen(buff2) + 1);
      //   strcpy(geomi->file, buff2);

      //   geomi->file2 = NULL;
      //   if (fast_startup == 0 && is_geom == 1) {
      //     char *ext;

      //     ext = strrchr(buff2, '.');
      //     if (ext != NULL) {
      //       ext[0] = 0;
      //       strcat(buff2, ".ge2");
      //       if (FILE_EXISTS_CASEDIR(buff2) == YES) {
      //         NewMemory((void **)&geomi->file2, strlen(buff2) + 1);
      //         strcpy(geomi->file2, buff2);
      //         ReadGeomFile2(geomi);
      //       }
      //     }
      //   }

      //   if (ngeomobjinfo > 0) {
      //     geomi->ngeomobjinfo = ngeomobjinfo;
      //     NewMemory((void **)&geomi->geomobjinfo,
      //               ngeomobjinfo * sizeof(geomobjdata));
      //     for (i = 0; i < ngeomobjinfo; i++) {
      //       geomobjdata *geomobji;
      //       float *center;
      //       char *texture_mapping = NULL, *texture_vals = NULL;
      //       char *colorlabel;

      //       geomobji = geomi->geomobjinfo + i;

      //       geomobji->texture_name = NULL;
      //       geomobji->texture_mapping = TEXTURE_RECTANGULAR;

      //       fgets(buffer, 255, stream);

      //       colorlabel = strchr(buffer, '!');
      //       geomobji->color = NULL;
      //       geomobji->use_geom_color = 0;
      //       geomobji->bounding_box = NULL;
      //       if (colorlabel != NULL) {
      //         int colors[3] = {-1, -1, -1};
      //         float transparency = -1.0;

      //         colorlabel++;
      //         if (colorlabel != buffer) colorlabel[-1] = 0;
      //         sscanf(colorlabel, "%i %i %i %f", colors, colors + 1, colors +
      //         2,
      //                &transparency);
      //         if (colors[0] >= 0 && colors[1] >= 0 && colors[2] >= 0) {
      //           float fcolors[4];

      //           fcolors[0] = colors[0] / 255.0;
      //           fcolors[1] = colors[1] / 255.0;
      //           fcolors[2] = colors[2] / 255.0;
      //           if (transparency < 0.0) transparency = 1.0;
      //           fcolors[3] = transparency;
      //           geomobji->color = GetColorPtr(fcolors);
      //           geomobji->use_geom_color = 1;
      //         }
      //       }

      //       texture_mapping = TrimFront(buffer);
      //       if (texture_mapping != NULL)
      //         texture_vals = strchr(texture_mapping, ' ');

      //       if (texture_vals != NULL) {
      //         char *surflabel;
      //         int is_terrain = 0;

      //         texture_vals++;
      //         texture_vals[-1] = 0;
      //         center = geomobji->texture_center;
      //         surflabel = strchr(texture_vals, '%');
      //         if (surflabel != NULL) {
      //           surflabel++;
      //           surflabel[-1] = 0;
      //           TrimBack(surflabel);
      //           surflabel = TrimFront(surflabel + 1);
      //           geomi->surfgeom = GetSurface(surflabel);
      //           if (geomobji->color == NULL)
      //             geomobji->color = geomi->surfgeom->color;
      //         }
      //         sscanf(texture_vals, "%f %f %f %i", center, center + 1, center
      //         + 2,
      //                &is_terrain);
      //         geomi->is_terrain = is_terrain;
      //       }
      //       if (geomi->is_terrain == 1) {
      //         is_terrain_case = 1;
      //         auto_terrain = 1;
      //       }
      //       if (texture_mapping != NULL &&
      //           strcmp(texture_mapping, "SPHERICAL") == 0) {
      //         geomobji->texture_mapping = TEXTURE_SPHERICAL;
      //       }
      //     }
      //   }

      //   ngeominfo++;
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ OBST ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if (MatchSMV(buffer, "OBST") == 1) {
      //   int nobsts = 0;
      //   meshdata *meshi;
      //   unsigned char *is_block_terrain;
      //   int nn;

      //   iobst++;

      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &nobsts);

      //   meshi = meshinfo + iobst - 1;

      //   if (nobsts <= 0) continue;

      //   NewMemory((void **)&meshi->is_block_terrain,
      //             nobsts * sizeof(unsigned char));
      //   is_block_terrain = meshi->is_block_terrain;

      //   for (nn = 0; nn < nobsts; nn++) {
      //     fgets(buffer, 255, stream);
      //   }
      //   for (nn = 0; nn < nobsts; nn++) {
      //     int ijk2[6], colorindex_local = 0, blocktype_local = -1;

      //     fgets(buffer, 255, stream);
      //     sscanf(buffer, "%i %i %i %i %i %i %i %i", ijk2, ijk2 + 1, ijk2 + 2,
      //            ijk2 + 3, ijk2 + 4, ijk2 + 5, &colorindex_local,
      //            &blocktype_local);
      //     if (blocktype_local >= 0 && (blocktype_local & 8) == 8) {
      //       is_block_terrain[nn] = 1;
      //     } else {
      //       is_block_terrain[nn] = 0;
      //     }
      //   }
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ PROP ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if (MatchSMV(buffer, "PROP") == 1) {
      //   propdata *propi;
      //   char *file_buffer;
      //   char proplabel[255];
      //   int lenbuf;
      //   int ntextures_local;
      //   int nsmokeview_ids;
      //   char *smokeview_id;

      //   propi = propinfo + npropinfo;

      //   if (fgets(proplabel, 255, stream) == NULL) {
      //     BREAK; // prop label
      //   }
      //   TrimBack(proplabel);
      //   file_buffer = TrimFront(proplabel);

      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK; // number of smokeview_id's
      //   }
      //   sscanf(buffer, "%i", &nsmokeview_ids);

      //   InitProp(propi, nsmokeview_ids, file_buffer);
      //   for (i = 0; i < nsmokeview_ids; i++) {
      //     if (fgets(buffer, 255, stream) == NULL) {
      //       BREAK; // smokeview_id
      //     }
      //     TrimBack(buffer);
      //     file_buffer = TrimFront(buffer);
      //     lenbuf = strlen(file_buffer);
      //     NewMemory((void **)&smokeview_id, lenbuf + 1);
      //     strcpy(smokeview_id, file_buffer);
      //     propi->smokeview_ids[i] = smokeview_id;
      //     propi->smv_objects[i] =
      //         GetSmvObjectType(propi->smokeview_ids[i], missing_device);
      //   }
      //   propi->smv_object = propi->smv_objects[0];
      //   propi->smokeview_id = propi->smokeview_ids[0];

      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK; // keyword_values
      //   }
      //   sscanf(buffer, "%i", &propi->nvars_indep);
      //   propi->vars_indep = NULL;
      //   propi->svals = NULL;
      //   propi->texturefiles = NULL;
      //   ntextures_local = 0;
      //   if (propi->nvars_indep > 0) {
      //     NewMemory((void **)&propi->vars_indep,
      //               propi->nvars_indep * sizeof(char *));
      //     NewMemory((void **)&propi->svals, propi->nvars_indep * sizeof(char
      //     *)); NewMemory((void **)&propi->fvals, propi->nvars_indep *
      //     sizeof(float)); NewMemory((void **)&propi->vars_indep_index,
      //               propi->nvars_indep * sizeof(int));
      //     NewMemory((void **)&propi->texturefiles,
      //               propi->nvars_indep * sizeof(char *));

      //     for (i = 0; i < propi->nvars_indep; i++) {
      //       char *equal;

      //       propi->svals[i] = NULL;
      //       propi->vars_indep[i] = NULL;
      //       propi->fvals[i] = 0.0;
      //       fgets(buffer, 255, stream);
      //       equal = strchr(buffer, '=');
      //       if (equal != NULL) {
      //         char *buf1, *buf2, *keyword, *val;
      //         int lenkey, lenval;
      //         char *texturefile;

      //         buf1 = buffer;
      //         buf2 = equal + 1;
      //         *equal = 0;

      //         TrimBack(buf1);
      //         keyword = TrimFront(buf1);
      //         lenkey = strlen(keyword);

      //         TrimBack(buf2);
      //         val = TrimFront(buf2);
      //         lenval = strlen(val);

      //         if (lenkey == 0 || lenval == 0) continue;

      //         if (val[0] == '"') {
      //           val[0] = ' ';
      //           if (val[lenval - 1] == '"') val[lenval - 1] = ' ';
      //           TrimBack(val);
      //           val = TrimFront(val);
      //           NewMemory((void **)&propi->svals[i], lenval + 1);
      //           strcpy(propi->svals[i], val);
      //           texturefile = strstr(val, "t%");
      //           if (texturefile != NULL) {
      //             texturefile += 2;
      //             texturefile = TrimFront(texturefile);
      //             propi->texturefiles[ntextures_local] = propi->svals[i];
      //             strcpy(propi->svals[i], texturefile);

      //             ntextures_local++;
      //           }
      //         }

      //         NewMemory((void **)&propi->vars_indep[i], lenkey + 1);
      //         strcpy(propi->vars_indep[i], keyword);

      //         sscanf(val, "%f", propi->fvals + i);
      //       }
      //     }
      //     GetIndepVarIndices(propi->smv_object, propi->vars_indep,
      //                        propi->nvars_indep, propi->vars_indep_index);
      //   }
      //   propi->ntextures = ntextures_local;
      //   npropinfo++;
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ TERRAIN +++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if (MatchSMV(buffer, "TERRAIN") == 1) {
      //   //     manual_terrain = 1;
      //   //   fgets(buffer, 255, stream);
      //   //   nterraininfo++;
      //   //   continue;
      //   terraindata *terraini;
      //   int len_buffer;
      //   char *file, *buffer_ptr;
      //   int mesh_terrain = -1;

      //   if (strlen(buffer) > 7) {
      //     sscanf(buffer + 7, "%i", &mesh_terrain);
      //   }

      //   fgets(buffer, 255, stream);
      //   buffer_ptr = TrimFrontBack(buffer);
      //   len_buffer = strlen(buffer_ptr);
      //   NewMemory((void **)&file, len_buffer + 1);
      //   strcpy(file, buffer_ptr);

      //   terraini = terraininfo + nterraininfo;
      //   terraini->file = file;
      //   if (mesh_terrain == -1) {
      //     mesh_terrain =
      //         nterraininfo; // no mesh_terrain on TERRAIN line so assume that
      //                       // number of TERRAIN and MESH lines are the same
      //   } else {
      //     mesh_terrain--; // mesh_terrain on TERRAIN line goes from 1 to
      //     number
      //                     // of meshes so subtract 1
      //   }
      //   meshinfo[mesh_terrain].terrain = terraini;
      //   terraini->terrain_mesh = meshinfo + mesh_terrain;
      //   terraini->defined = 0;
      //   nterraininfo++;
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++ CLASS_OF_PARTICLES +++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    // CLASS_OF_PARTICLES
    //  name
    //  r g b (color)
    // ntypes
    // long label
    // short label
    // unit
    // ....
    // ...
    // ...
    // long label
    // short label
    // unit

    // 1'st type  hidden
    // 2'nd type  default (uniform)
    // 3'rd type first type read in
    // 2+ntypes  ntypes type read in

    if (MatchSMV(buffer, "CLASS_OF_PARTICLES") == 1 ||
        MatchSMV(buffer, "CLASS_OF_HUMANS") == 1) {
      //   float rgb_class[4];
      //   partclassdata *partclassi;
      //   char *device_ptr;
      //   char *prop_id;
      //   char prop_buffer[255];
      //   size_t len;

      //   partclassi = partclassinfo + npartclassinfo;
      //   partclassi->kind = PARTICLES;
      //   if (MatchSMV(buffer, "CLASS_OF_HUMANS") == 1) partclassi->kind =
      //   HUMANS; fgets(buffer, 255, stream);

      //   GetLabels(buffer, partclassi->kind, &device_ptr, &prop_id,
      //   prop_buffer); if (prop_id != NULL) {
      //     device_ptr = NULL;
      //   }
      //   partclassi->prop = NULL;

      //   partclassi->sphere = NULL;
      //   partclassi->smv_device = NULL;
      //   partclassi->device_name = NULL;
      //   if (device_ptr != NULL) {
      //     partclassi->sphere = GetSmvObjectType("SPHERE", missing_device);

      //     partclassi->smv_device = GetSmvObjectType(device_ptr,
      //     missing_device); if (partclassi->smv_device != NULL) {
      //       len = strlen(device_ptr);
      //       NewMemory((void **)&partclassi->device_name, len + 1);
      //       STRCPY(partclassi->device_name, device_ptr);
      //     } else {
      //       char tube[10];

      //       strcpy(tube, "TUBE");
      //       len = strlen(tube);
      //       NewMemory((void **)&partclassi->device_name, len + 1);
      //       STRCPY(partclassi->device_name, tube);
      //       partclassi->smv_device = GetSmvObjectType(tube, missing_device);
      //     }
      //   }

      //   TrimBack(buffer);
      //   len = strlen(buffer);
      //   partclassi->name = NULL;
      //   if (len > 0) {
      //     NewMemory((void **)&partclassi->name, len + 1);
      //     STRCPY(partclassi->name, TrimFront(buffer));
      //   }

      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%f %f %f", rgb_class, rgb_class + 1, rgb_class + 2);
      //   rgb_class[3] = 1.0;
      //   partclassi->rgb = GetColorPtr(rgb_class);

      //   partclassi->ntypes = 0;
      //   partclassi->xyz = NULL;
      //   partclassi->maxpoints = 0;
      //   partclassi->labels = NULL;

      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &partclassi->ntypes);
      //   partclassi->ntypes += 2;
      //   partclassi->nvars_dep =
      //       partclassi->ntypes - 2 + 3; // subtract off two "dummies" at
      //                                   // beginning and add 3 at end for
      //                                   r,g,b
      //   if (partclassi->ntypes > 0) {
      //     flowlabels *labelj;
      //     char shortdefaultlabel[] = "Uniform";
      //     char longdefaultlabel[] = "Uniform color";
      //     int j;

      //     NewMemory((void **)&partclassi->labels,
      //               partclassi->ntypes * sizeof(flowlabels));

      //     labelj = partclassi->labels; // placeholder for hidden

      //     labelj->longlabel = NULL;
      //     labelj->shortlabel = NULL;
      //     labelj->unit = NULL;

      //     labelj = partclassi->labels + 1; // placeholder for default

      //     labelj->longlabel = NULL;
      //     NewMemory((void **)&labelj->longlabel, strlen(longdefaultlabel) +
      //     1); strcpy(labelj->longlabel, longdefaultlabel); labelj->shortlabel
      //     = NULL; NewMemory((void **)&labelj->shortlabel,
      //     strlen(shortdefaultlabel) + 1); strcpy(labelj->shortlabel,
      //     shortdefaultlabel); labelj->unit = NULL;

      //     partclassi->col_azimuth = -1;
      //     partclassi->col_diameter = -1;
      //     partclassi->col_elevation = -1;
      //     partclassi->col_length = -1;
      //     partclassi->col_u_vel = -1;
      //     partclassi->col_v_vel = -1;
      //     partclassi->col_w_vel = -1;
      //     partclassi->vis_type = PART_POINTS;
      //     for (j = 2; j < partclassi->ntypes; j++) {
      //       labelj = partclassi->labels + j;
      //       labelj->longlabel = NULL;
      //       labelj->shortlabel = NULL;
      //       labelj->unit = NULL;
      //       ReadLabels(labelj, stream, NULL);
      //       partclassi->vars_dep[j - 2] = labelj->shortlabel;
      //       if (strcmp(labelj->shortlabel, "DIAMETER") == 0) {
      //         partclassi->col_diameter = j - 2;
      //       }
      //       if (strcmp(labelj->shortlabel, "LENGTH") == 0) {
      //         partclassi->col_length = j - 2;
      //       }
      //       if (strcmp(labelj->shortlabel, "AZIMUTH") == 0) {
      //         partclassi->col_azimuth = j - 2;
      //       }
      //       if (strcmp(labelj->shortlabel, "ELEVATION") == 0) {
      //         partclassi->col_elevation = j - 2;
      //       }
      //       if (STRCMP(labelj->shortlabel, "U-VEL") == 0) {
      //         partclassi->col_u_vel = j - 2;
      //       }
      //       if (STRCMP(labelj->shortlabel, "V-VEL") == 0) {
      //         partclassi->col_v_vel = j - 2;
      //       }
      //       if (STRCMP(labelj->shortlabel, "W-VEL") == 0) {
      //         partclassi->col_w_vel = j - 2;
      //       }
      //     }
      //   }
      //   partclassi->diameter = 1.0;
      //   partclassi->length = 1.0;
      //   partclassi->azimuth = 0.0;
      //   partclassi->elevation = 0.0;
      //   partclassi->dx = 0.0;
      //   partclassi->dy = 0.0;
      //   partclassi->dz = 0.0;
      //   if (device_ptr != NULL) {
      //     float diameter, length, azimuth, elevation;

      //     fgets(buffer, 255, stream);
      //     sscanf(buffer, "%f %f %f %f", &diameter, &length, &azimuth,
      //     &elevation); partclassi->diameter = diameter; partclassi->length =
      //     length; partclassi->azimuth = azimuth; partclassi->elevation =
      //     elevation;
      //   }
      //   npartclassinfo++;
      continue;
    }

    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ LABEL ++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    if (MatchSMV(buffer, "LABEL") == 1) {

      //   /*
      //   LABEL
      //   x y z r g b tstart tstop
      //   label

      //   */
      //   {
      //     float *xyz, *frgbtemp, *tstart_stop;
      //     int *rgbtemp;
      //     labeldata labeltemp, *labeli;
      //     char *bufferptr;

      //     labeli = &labeltemp;

      //     xyz = labeli->xyz;
      //     frgbtemp = labeli->frgb;
      //     rgbtemp = labeli->rgb;
      //     tstart_stop = labeli->tstart_stop;

      //     labeli->labeltype = TYPE_SMV;
      //     fgets(buffer, 255, stream);
      //     frgbtemp[0] = -1.0;
      //     frgbtemp[1] = -1.0;
      //     frgbtemp[2] = -1.0;
      //     frgbtemp[3] = 1.0;
      //     tstart_stop[0] = -1.0;
      //     tstart_stop[1] = -1.0;
      //     sscanf(buffer, "%f %f %f %f %f %f %f %f", xyz, xyz + 1, xyz + 2,
      //            frgbtemp, frgbtemp + 1, frgbtemp + 2, tstart_stop,
      //            tstart_stop + 1);

      //     if (frgbtemp[0] < 0.0 || frgbtemp[1] < 0.0 || frgbtemp[2] < 0.0 ||
      //         frgbtemp[0] > 1.0 || frgbtemp[1] > 1.0 || frgbtemp[2] > 1.0) {
      //       labeli->useforegroundcolor = 1;
      //     } else {
      //       labeli->useforegroundcolor = 0;
      //     }
      //     fgets(buffer, 255, stream);
      //     TrimBack(buffer);
      //     bufferptr = TrimFront(buffer);

      //     strcpy(labeli->name, bufferptr);
      //     rgbtemp[0] = frgbtemp[0] * 255;
      //     rgbtemp[1] = frgbtemp[1] * 255;
      //     rgbtemp[2] = frgbtemp[2] * 255;
      //     LabelInsert(labeli);
      //   }
      continue;
    }

    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ TICKS ++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    if (MatchSMV(buffer, "TICKS") == 1) {
      //   ntickinfo++;
      //   ntickinfo_smv++;
      //   {
      //     tickdata *ticki;
      //     float *begt, *endt;
      //     int *nbarst;
      //     float term;
      //     float length = 0.0;
      //     float *dxyz;
      //     float sum;

      //     ticki = tickinfo + ntickinfo - 1;
      //     begt = ticki->begin;
      //     endt = ticki->end;
      //     nbarst = &ticki->nbars;
      //     dxyz = ticki->dxyz;

      //     if (fgets(buffer, 255, stream) == NULL) {
      //       BREAK;
      //     }
      //     *nbarst = 0;
      //     sscanf(buffer, "%f %f %f %f %f %f %i", begt, begt + 1, begt + 2,
      //     endt,
      //            endt + 1, endt + 2, nbarst);
      //     if (*nbarst < 1) *nbarst = 1;
      //     if (fgets(buffer, 255, stream) == NULL) {
      //       BREAK;
      //     }
      //     {
      //       float *rgbtemp;

      //       rgbtemp = ticki->rgb;
      //       rgbtemp[0] = -1.0;
      //       rgbtemp[1] = -1.0;
      //       rgbtemp[2] = -1.0;
      //       ticki->width = -1.0;
      //       sscanf(buffer, "%f %i %f %f %f %f", &ticki->dlength, &ticki->dir,
      //              rgbtemp, rgbtemp + 1, rgbtemp + 2, &ticki->width);
      //       if (rgbtemp[0] < 0.0 || rgbtemp[0] > 1.0 || rgbtemp[1] < 0.0 ||
      //           rgbtemp[1] > 1.0 || rgbtemp[2] < 0.0 || rgbtemp[2] > 1.0) {
      //         ticki->useforegroundcolor = 1;
      //       } else {
      //         ticki->useforegroundcolor = 0;
      //       }
      //       if (ticki->width < 0.0) ticki->width = 1.0;
      //     }
      //     for (i = 0; i < 3; i++) {
      //       term = endt[i] - begt[i];
      //       length += term * term;
      //     }
      //     if (length <= 0.0) {
      //       endt[0] = begt[0] + 1.0;
      //       length = 1.0;
      //     }
      //     ticki->length = sqrt(length);
      //     dxyz[0] = 0.0;
      //     dxyz[1] = 0.0;
      //     dxyz[2] = 0.0;
      //     switch (ticki->dir) {
      //     case XRIGHT:
      //     case XLEFT:
      //       dxyz[0] = 1.0;
      //       break;
      //     case YFRONT:
      //     case YBACK:
      //       dxyz[1] = 1.0;
      //       break;
      //     case ZBOTTOM:
      //     case ZTOP:
      //       dxyz[2] = 1.0;
      //       break;
      //     default:
      //       ASSERT(FFALSE);
      //       break;
      //     }
      //     if (ticki->dir < 0) {
      //       for (i = 0; i < 3; i++) {
      //         dxyz[i] = -dxyz[i];
      //       }
      //     }
      //     sum = 0.0;
      //     sum = dxyz[0] * dxyz[0] + dxyz[1] * dxyz[1] + dxyz[2] * dxyz[2];
      //     if (sum > 0.0) {
      //       sum = sqrt(sum);
      //       dxyz[0] *= (ticki->dlength / sum);
      //       dxyz[1] *= (ticki->dlength / sum);
      //       dxyz[2] *= (ticki->dlength / sum);
      //     }
      //   }
      continue;
    }

    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ OUTLINE ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    if (MatchSMV(buffer, "OUTLINE") == 1) {
      //   outlinedata *outlinei;

      //   noutlineinfo++;
      //   outlinei = outlineinfo + noutlineinfo - 1;
      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   sscanf(buffer, "%i", &outlinei->nlines);
      //   if (outlinei->nlines > 0) {
      //     NewMemory((void **)&outlinei->x1, outlinei->nlines *
      //     sizeof(float)); NewMemory((void **)&outlinei->y1, outlinei->nlines
      //     * sizeof(float)); NewMemory((void **)&outlinei->z1,
      //     outlinei->nlines * sizeof(float)); NewMemory((void
      //     **)&outlinei->x2, outlinei->nlines * sizeof(float));
      //     NewMemory((void **)&outlinei->y2, outlinei->nlines *
      //     sizeof(float)); NewMemory((void **)&outlinei->z2, outlinei->nlines
      //     * sizeof(float)); for (i = 0; i < outlinei->nlines; i++) {
      //       fgets(buffer, 255, stream);
      //       sscanf(buffer, "%f %f %f %f %f %f", outlinei->x1 + i,
      //              outlinei->y1 + i, outlinei->z1 + i, outlinei->x2 + i,
      //              outlinei->y2 + i, outlinei->z2 + i);
      //     }
      //   }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ CADGEOM ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "CADGEOM") == 1) {
      //   size_t len;
      //   char *bufferptr;

      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   bufferptr = TrimFrontBack(buffer);
      //   len = strlen(bufferptr);
      //   cadgeominfo[ncadgeom].order = NULL;
      //   cadgeominfo[ncadgeom].quad = NULL;
      //   cadgeominfo[ncadgeom].file = NULL;
      //   if (FILE_EXISTS_CASEDIR(bufferptr) == YES) {
      //     if (NewMemory((void **)&cadgeominfo[ncadgeom].file,
      //                   (unsigned int)(len + 1)) == 0)
      //       return 2;
      //     STRCPY(cadgeominfo[ncadgeom].file, bufferptr);
      //     ReadCADGeom(cadgeominfo + ncadgeom);
      //     ncadgeom++;
      //   } else {
      //     PRINTF(_("***Error: CAD geometry file: %s could not be opened"),
      //            bufferptr);
      //     PRINTF("\n");
      //   }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "OFFSET") == 1) {
      //   ioffset++;
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SURFDEF ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SURFDEF") == 1) {
      //   char *bufferptr;

      //   fgets(buffer, 255, stream);
      //   bufferptr = TrimFrontBack(buffer);
      //   strcpy(surfacedefaultlabel, TrimFront(bufferptr));
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SURFACE ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SURFACE") == 1) {
      error = parse_surface(smvfile, stream);
      if (error) return error;
      continue;
    }
    /*
      ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ MATERIAL  +++++++++++++++++++++++++
      ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "MATERIAL") == 1) {
      //   matldata *matli;
      //   float s_color[4];
      //   int len;

      //   matli = matlinfo + nmatlinfo;
      //   InitMatl(matli);

      //   fgets(buffer, 255, stream);
      //   TrimBack(buffer);
      //   len = strlen(buffer);
      //   NewMemory((void **)&matli->matllabel, (len + 1) * sizeof(char));
      //   strcpy(matli->matllabel, TrimFront(buffer));

      //   s_color[0] = matli->color[0];
      //   s_color[1] = matli->color[1];
      //   s_color[2] = matli->color[2];
      //   s_color[3] = matli->color[3];
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%f %f %f %f", s_color, s_color + 1, s_color + 2,
      //          s_color + 3);

      //   s_color[0] = CLAMP(s_color[0], 0.0, 1.0);
      //   s_color[1] = CLAMP(s_color[1], 0.0, 1.0);
      //   s_color[2] = CLAMP(s_color[2], 0.0, 1.0);
      //   s_color[3] = CLAMP(s_color[3], 0.0, 1.0);

      //   matli->color = GetColorPtr(s_color);

      //   nmatlinfo++;
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ GRID ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "GRID") == 1) {
      //   meshdata *meshi;
      //   float *xp, *yp, *zp;
      //   float *xp2, *yp2, *zp2;
      //   float *xplt_cen, *yplt_cen, *zplt_cen;
      //   int *imap, *jmap, *kmap;

      //   //      int lenbuffer;

      //   //      TrimBack(buffer);
      //   //      lenbuffer=strlen(buffer);
      //   //      if(lenbuffer>4){
      //   //        if(buffer[5]!=' ')continue;
      //   //      }

      //   igrid++;
      //   if (meshinfo != NULL) {
      //     meshi = meshinfo + igrid - 1;
      //     InitMesh(meshi);
      //     {
      //       size_t len_meshlabel;
      //       char *meshlabel;

      //       len_meshlabel = 0;
      //       if (strlen(buffer) > 5) {
      //         meshlabel = TrimFront(buffer + 5);
      //         TrimBack(meshlabel);
      //         len_meshlabel = strlen(meshlabel);
      //       }
      //       if (len_meshlabel > 0) {
      //         NewMemory((void **)&meshi->label, (len_meshlabel + 1));
      //         strcpy(meshi->label, meshlabel);
      //       } else {
      //         sprintf(buffer, "%i", igrid);
      //         NewMemory((void **)&meshi->label, strlen(buffer) + 1);
      //         strcpy(meshi->label, buffer);
      //       }
      //     }
      //   }
      //   setGRID = 1;
      //   if (noGRIDpresent == 1) {
      //     ibartemp = 2;
      //     jbartemp = 2;
      //     kbartemp = 2;
      //   } else {
      //     fgets(buffer, 255, stream);
      //     sscanf(buffer, "%i %i %i", &ibartemp, &jbartemp, &kbartemp);
      //   }
      //   if (ibartemp < 1) ibartemp = 1;
      //   if (jbartemp < 1) jbartemp = 1;
      //   if (kbartemp < 1) kbartemp = 1;
      //   xp = NULL;
      //   yp = NULL;
      //   zp = NULL;
      //   xp2 = NULL;
      //   yp2 = NULL;
      //   zp2 = NULL;
      //   if (NewMemory((void **)&xp, sizeof(float) * (ibartemp + 1)) == 0 ||
      //       NewMemory((void **)&yp, sizeof(float) * (jbartemp + 1)) == 0 ||
      //       NewMemory((void **)&zp, sizeof(float) * (kbartemp + 1)) == 0 ||
      //       NewMemory((void **)&xplt_cen, sizeof(float) * ibartemp) == 0 ||
      //       NewMemory((void **)&yplt_cen, sizeof(float) * jbartemp) == 0 ||
      //       NewMemory((void **)&zplt_cen, sizeof(float) * kbartemp) == 0 ||
      //       NewMemory((void **)&xp2, sizeof(float) * (ibartemp + 1)) == 0 ||
      //       NewMemory((void **)&yp2, sizeof(float) * (jbartemp + 1)) == 0 ||
      //       NewMemory((void **)&zp2, sizeof(float) * (kbartemp + 1)) == 0)
      //     return 2;
      //   if (NewMemory((void **)&imap, sizeof(int) * (ibartemp + 1)) == 0 ||
      //       NewMemory((void **)&jmap, sizeof(int) * (jbartemp + 1)) == 0 ||
      //       NewMemory((void **)&kmap, sizeof(int) * (kbartemp + 1)) == 0)
      //     return 2;
      //   if (meshinfo != NULL) {
      //     meshi->xplt = xp;
      //     meshi->yplt = yp;
      //     meshi->zplt = zp;
      //     meshi->xplt_cen = xplt_cen;
      //     meshi->yplt_cen = yplt_cen;
      //     meshi->zplt_cen = zplt_cen;
      //     meshi->xplt_orig = xp2;
      //     meshi->yplt_orig = yp2;
      //     meshi->zplt_orig = zp2;
      //     meshi->ibar = ibartemp;
      //     meshi->jbar = jbartemp;
      //     meshi->kbar = kbartemp;
      //     meshi->plotx = ibartemp / 2;
      //     meshi->ploty = jbartemp / 2;
      //     meshi->plotz = kbartemp / 2;
      //     meshi->imap = imap;
      //     meshi->n_imap = 0;
      //     meshi->jmap = jmap;
      //     meshi->n_jmap = 0;
      //     meshi->kmap = kmap;
      //     meshi->n_kmap = 0;
      //   }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ ZONE ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "ZONE") == 1) {
      //   char *filename;
      //   zonedata *zonei;
      //   char buffer_csv[1000], *buffer_csvptr;
      //   char *period = NULL;
      //   size_t len;
      //   int n;
      //   char *bufferptr;

      //   zonei = zoneinfo + izone_local;
      //   if (fgets(buffer, 255, stream) == NULL) {
      //     nzoneinfo--;
      //     BREAK;
      //   }
      //   bufferptr = TrimFrontBack(buffer);
      //   len = strlen(bufferptr);
      //   zonei->loaded = 0;
      //   zonei->display = 0;

      //   buffer_csvptr = buffer_csv;
      //   strcpy(buffer_csv, bufferptr);
      //   filename = GetZoneFileName(buffer_csvptr);
      //   if (filename != NULL) period = strrchr(filename, '.');
      //   if (filename != NULL && period != NULL && strcmp(period, ".csv") ==
      //   0) {
      //     zonei->csv = 1;
      //     zonecsv = 1;
      //   } else {
      //     zonei->csv = 0;
      //     zonecsv = 0;
      //     filename = GetZoneFileName(bufferptr);
      //   }

      //   if (filename == NULL) {
      //     int nn;

      //     for (nn = 0; nn < 4; nn++) {
      //       if (ReadLabels(&zonei->label[nn], stream, NULL) == LABEL_ERR) {
      //         return 2;
      //       }
      //     }
      //     nzoneinfo--;
      //   } else {
      //     len = strlen(filename);
      //     NewMemory((void **)&zonei->file, (unsigned int)(len + 1));
      //     STRCPY(zonei->file, filename);
      //     for (n = 0; n < 4; n++) {
      //       if (ReadLabels(&zonei->label[n], stream, NULL) == LABEL_ERR) {
      //         return 2;
      //       }
      //     }
      //     izone_local++;
      //   }
      //   if (colorlabelzone != NULL) {
      //     for (n = 0; n < MAXRGB; n++) {
      //       FREEMEMORY(colorlabelzone[n]);
      //     }
      //     FREEMEMORY(colorlabelzone);
      //   }
      //   CheckMemory;
      //   NewMemory((void **)&colorlabelzone, MAXRGB * sizeof(char *));
      //   for (n = 0; n < MAXRGB; n++) {
      //     colorlabelzone[n] = NULL;
      //   }
      //   for (n = 0; n < nrgb; n++) {
      //     NewMemory((void **)&colorlabelzone[n], 11);
      //   }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ ROOM ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "ROOM") == 1) {
      //   roomdata *roomi;

      //   isZoneFireModel = 1;
      //   visFrame = 0;
      //   roomdefined = 1;
      //   iroom++;
      //   roomi = roominfo + iroom - 1;
      //   roomi->valid = 0;
      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   sscanf(buffer, "%f %f %f", &roomi->dx, &roomi->dy, &roomi->dz);
      //   roomi->valid = 1;
      //   if (fgets(buffer, 255, stream) == NULL) {
      //     roomi->x0 = 0.0;
      //     roomi->y0 = 0.0;
      //     roomi->z0 = 0.0;
      //   } else {
      //     sscanf(buffer, "%f %f %f", &roomi->x0, &roomi->y0, &roomi->z0);
      //   }
      //   roomi->x1 = roomi->x0 + roomi->dx;
      //   roomi->y1 = roomi->y0 + roomi->dy;
      //   roomi->z1 = roomi->z0 + roomi->dz;

      //   if (setPDIM == 0) {
      //     if (roomi->x0 < xbar0) xbar0 = roomi->x0;
      //     if (roomi->y0 < ybar0) ybar0 = roomi->y0;
      //     if (roomi->z0 < zbar0) zbar0 = roomi->z0;
      //     if (roomi->x1 > xbar) xbar = roomi->x1;
      //     if (roomi->y1 > ybar) ybar = roomi->y1;
      //     if (roomi->z1 > zbar) zbar = roomi->z1;
      //   }
      continue;
    }

    if (MatchSMV(buffer, "AMBIENT") == 1) {
      //   if (fgets(buffer, 255, stream) == NULL) {
      //     BREAK;
      //   }
      //   sscanf(buffer, "%f %f %f", &pref, &pamb, &tamb);
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ CUTCELLS ++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if (MatchSMV(buffer, "CUTCELLS") == 1) {
      //   meshdata *meshi;
      //   int imesh, ncutcells;

      //   sscanf(buffer + 10, "%i", &imesh);
      //   imesh = CLAMP(imesh - 1, 0, nmeshes - 1);
      //   meshi = meshinfo + imesh;

      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &ncutcells);
      //   meshi->ncutcells = ncutcells;

      //   if (ncutcells > 0) {
      //     NewMemory((void **)&meshi->cutcells, ncutcells * sizeof(int));
      //     for (i = 0; i < 1 + (ncutcells - 1) / 15; i++) {
      //       int cc[15], j;

      //       fgets(buffer, 255, stream);
      //       for (j = 0; j < 15; j++) {
      //         cc[j] = 0;
      //       }
      //       sscanf(buffer, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
      //       cc,
      //              cc + 1, cc + 2, cc + 3, cc + 4, cc + 5, cc + 6, cc + 7, cc
      //              + 8, cc + 9, cc + 10, cc + 11, cc + 12, cc + 13, cc + 14);
      //       for (j = 15 * i; j < MIN(15 * (i + 1), ncutcells); j++) {
      //         meshi->cutcells[j] = cc[j % 15];
      //       }
      //     }
      //   }

      continue;
    }

    if ((MatchSMV(buffer, "DEVICE") == 1) &&
        (MatchSMV(buffer, "DEVICE_ACT") != 1)) {
      //   devicedata *devicei;

      //   devicei = deviceinfo + ndeviceinfo;
      //   ParseDevicekeyword(stream, devicei);
      //   CheckMemory;
      //   update_device = 1;
      //   ndeviceinfo++;
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ THCP ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "THCP") == 1) {
      //   meshdata *meshi;
      //   float normdenom;
      //   char *device_label;
      //   int tempval;

      //   if (ioffset == 0) ioffset = 1;
      //   meshi = meshinfo + ioffset - 1;
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &tempval);
      //   if (tempval < 0) tempval = 0;
      //   meshi->ntc = tempval;
      //   ntc_total += meshi->ntc;
      //   hasSensorNorm = 0;
      //   if (meshi->ntc > 0) {
      //     int nn;

      //     for (nn = 0; nn < meshi->ntc; nn++) {
      //       float *xyz, *xyznorm;
      //       fgets(buffer, 255, stream);

      //       strcpy(devicecopy->deviceID, "");
      //       xyz = devicecopy->xyz;
      //       xyznorm = devicecopy->xyznorm;
      //       xyz[0] = 0.0;
      //       xyz[1] = 0.0;
      //       xyz[2] = 0.0;
      //       xyznorm[0] = 0.0;
      //       xyznorm[1] = 0.0;
      //       xyznorm[2] = -1.0;
      //       device_label = GetDeviceLabel(buffer);
      //       sscanf(buffer, "%f %f %f %f %f %f", xyz, xyz + 1, xyz + 2,
      //       xyznorm,
      //              xyznorm + 1, xyznorm + 2);
      //       normdenom = 0.0;
      //       normdenom += xyznorm[0] * xyznorm[0];
      //       normdenom += xyznorm[1] * xyznorm[1];
      //       normdenom += xyznorm[2] * xyznorm[2];
      //       if (normdenom > 0.1) {
      //         hasSensorNorm = 1;
      //         normdenom = sqrt(normdenom);
      //         xyznorm[0] /= normdenom;
      //         xyznorm[1] /= normdenom;
      //         xyznorm[2] /= normdenom;
      //       }
      //       if (device_label == NULL) {
      //         if (isZoneFireModel == 1) {
      //           devicecopy->object =
      //               GetSmvObjectType("target", thcp_object_backup);
      //         } else {
      //           devicecopy->object =
      //               GetSmvObjectType("thermoc4", thcp_object_backup);
      //         }
      //       } else {
      //         devicecopy->object =
      //             GetSmvObjectType(device_label, thcp_object_backup);
      //       }
      //       GetElevAz(xyznorm, &devicecopy->dtheta, devicecopy->rotate_axis,
      //                 NULL);

      //       InitDevice(devicecopy, xyz, 0, NULL, NULL, xyznorm, 0, 0, NULL,
      //                  "target");
      //       devicecopy->prop = NULL;

      //       devicecopy++;
      //       ndeviceinfo++;
      //     }
      //   }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SPRK ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SPRK") == 1) {
      //   meshdata *meshi;
      //   char *device_label;
      //   int tempval;

      //   meshi = meshinfo + ioffset - 1;
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &tempval);
      //   if (tempval < 0) tempval = 0;
      //   meshi->nspr = tempval;
      //   nspr_total += meshi->nspr;
      //   if (meshi->nspr > 0) {
      //     float *xsprcopy, *ysprcopy, *zsprcopy;
      //     int nn;

      //     FREEMEMORY(meshi->xspr);
      //     FREEMEMORY(meshi->yspr);
      //     FREEMEMORY(meshi->zspr);
      //     FREEMEMORY(meshi->tspr);
      //     if (NewMemory((void **)&meshi->xspr, meshi->nspr * sizeof(float))
      //     ==
      //             0 ||
      //         NewMemory((void **)&meshi->yspr, meshi->nspr * sizeof(float))
      //         ==
      //             0 ||
      //         NewMemory((void **)&meshi->zspr, meshi->nspr * sizeof(float))
      //         ==
      //             0 ||
      //         NewMemory((void **)&meshi->tspr, meshi->nspr * sizeof(float))
      //         ==
      //             0 ||
      //         NewMemory((void **)&meshi->xsprplot, meshi->nspr *
      //         sizeof(float)) ==
      //             0 ||
      //         NewMemory((void **)&meshi->ysprplot, meshi->nspr *
      //         sizeof(float)) ==
      //             0 ||
      //         NewMemory((void **)&meshi->zsprplot, meshi->nspr *
      //         sizeof(float)) ==
      //             0)
      //       return 2;
      //     for (nn = 0; nn < meshi->nspr; nn++) {
      //       meshi->tspr[nn] = 99999.;
      //     }
      //     xsprcopy = meshi->xspr;
      //     ysprcopy = meshi->yspr;
      //     zsprcopy = meshi->zspr;
      //     for (nn = 0; nn < meshi->nspr; nn++) {
      //       float *xyznorm;
      //       float normdenom;

      //       fgets(buffer, 255, stream);
      //       xyznorm = devicecopy->xyznorm;
      //       xyznorm[0] = 0.0;
      //       xyznorm[1] = 0.0;
      //       xyznorm[2] = -1.0;
      //       device_label = GetDeviceLabel(buffer);
      //       sscanf(buffer, "%f %f %f %f %f %f", xsprcopy, ysprcopy, zsprcopy,
      //              xyznorm, xyznorm + 1, xyznorm + 2);
      //       devicecopy->act_time = -1.0;
      //       devicecopy->type = DEVICE_SPRK;
      //       devicecopy->xyz[0] = *xsprcopy;
      //       devicecopy->xyz[1] = *ysprcopy;
      //       devicecopy->xyz[2] = *zsprcopy;
      //       normdenom = 0.0;
      //       normdenom += xyznorm[0] * xyznorm[0];
      //       normdenom += xyznorm[1] * xyznorm[1];
      //       normdenom += xyznorm[2] * xyznorm[2];
      //       normdenom = sqrt(normdenom);
      //       if (normdenom > 0.001) {
      //         xyznorm[0] /= normdenom;
      //         xyznorm[1] /= normdenom;
      //         xyznorm[2] /= normdenom;
      //       }
      //       if (device_label == NULL) {
      //         devicecopy->object = GetSmvObjectType(
      //             "sprinkler_upright", sprinkler_upright_object_backup);
      //       } else {
      //         devicecopy->object =
      //             GetSmvObjectType(device_label,
      //             sprinkler_upright_object_backup);
      //       }
      //       GetElevAz(xyznorm, &devicecopy->dtheta, devicecopy->rotate_axis,
      //                 NULL);

      //       InitDevice(devicecopy, NULL, 0, NULL, NULL, xyznorm, 0, 0, NULL,
      //                  NULL);

      //       devicecopy++;
      //       ndeviceinfo++;

      //       xsprcopy++;
      //       ysprcopy++;
      //       zsprcopy++;
      //     }
      //   }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ HEAT ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "HEAT") == 1) {
      //   meshdata *meshi;
      //   char *device_label;
      //   int tempval;
      //   int nn;

      //   meshi = meshinfo + ioffset - 1;
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &tempval);
      //   if (tempval < 0) tempval = 0;
      //   meshi->nheat = tempval;
      //   nheat_total += meshi->nheat;
      //   if (meshi->nheat > 0) {
      //     float *xheatcopy, *yheatcopy, *zheatcopy;

      //     FREEMEMORY(meshi->xheat);
      //     FREEMEMORY(meshi->yheat);
      //     FREEMEMORY(meshi->zheat);
      //     FREEMEMORY(meshi->theat);
      //     FREEMEMORY(meshi->xheatplot);
      //     FREEMEMORY(meshi->yheatplot);
      //     FREEMEMORY(meshi->zheatplot);
      //     if (NewMemory((void **)&meshi->xheat, meshi->nheat * sizeof(float))
      //     ==
      //             0 ||
      //         NewMemory((void **)&meshi->yheat, meshi->nheat * sizeof(float))
      //         ==
      //             0 ||
      //         NewMemory((void **)&meshi->zheat, meshi->nheat * sizeof(float))
      //         ==
      //             0 ||
      //         NewMemory((void **)&meshi->theat, meshi->nheat * sizeof(float))
      //         ==
      //             0 ||
      //         NewMemory((void **)&meshi->xheatplot,
      //                   meshi->nheat * sizeof(float)) == 0 ||
      //         NewMemory((void **)&meshi->yheatplot,
      //                   meshi->nheat * sizeof(float)) == 0 ||
      //         NewMemory((void **)&meshi->zheatplot,
      //                   meshi->nheat * sizeof(float)) == 0)
      //       return 2;
      //     for (nn = 0; nn < meshi->nheat; nn++) {
      //       meshi->theat[nn] = 99999.;
      //     }
      //     xheatcopy = meshi->xheat;
      //     yheatcopy = meshi->yheat;
      //     zheatcopy = meshi->zheat;
      //     for (nn = 0; nn < meshi->nheat; nn++) {
      //       float *xyznorm;
      //       float normdenom;
      //       fgets(buffer, 255, stream);
      //       xyznorm = devicecopy->xyznorm;
      //       xyznorm[0] = 0.0;
      //       xyznorm[1] = 0.0;
      //       xyznorm[2] = -1.0;
      //       device_label = GetDeviceLabel(buffer);
      //       sscanf(buffer, "%f %f %f %f %f %f", xheatcopy, yheatcopy,
      //       zheatcopy,
      //              xyznorm, xyznorm + 1, xyznorm + 2);
      //       devicecopy->type = DEVICE_HEAT;
      //       devicecopy->act_time = -1.0;
      //       devicecopy->xyz[0] = *xheatcopy;
      //       devicecopy->xyz[1] = *yheatcopy;
      //       devicecopy->xyz[2] = *zheatcopy;
      //       normdenom = 0.0;
      //       normdenom += xyznorm[0] * xyznorm[0];
      //       normdenom += xyznorm[1] * xyznorm[1];
      //       normdenom += xyznorm[2] * xyznorm[2];
      //       normdenom = sqrt(normdenom);
      //       if (normdenom > 0.001) {
      //         xyznorm[0] /= normdenom;
      //         xyznorm[1] /= normdenom;
      //         xyznorm[2] /= normdenom;
      //       }
      //       if (device_label == NULL) {
      //         devicecopy->object =
      //             GetSmvObjectType("heat_detector",
      //             heat_detector_object_backup);
      //       } else {
      //         devicecopy->object =
      //             GetSmvObjectType(device_label,
      //             heat_detector_object_backup);
      //       }
      //       GetElevAz(xyznorm, &devicecopy->dtheta, devicecopy->rotate_axis,
      //                 NULL);

      //       InitDevice(devicecopy, NULL, 0, NULL, NULL, xyznorm, 0, 0, NULL,
      //                  NULL);
      //       devicecopy->prop = NULL;

      //       devicecopy++;
      //       ndeviceinfo++;
      //       xheatcopy++;
      //       yheatcopy++;
      //       zheatcopy++;
      //     }
      //   }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SMOD ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SMOD") == 1) {
      //   float xyz[3];
      //   int sdnum;
      //   char *device_label;
      //   int nn;

      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &sdnum);
      //   if (sdnum < 0) sdnum = 0;
      //   for (nn = 0; nn < sdnum; nn++) {
      //     float *xyznorm;
      //     float normdenom;

      //     xyznorm = devicecopy->xyznorm;
      //     xyznorm[0] = 0.0;
      //     xyznorm[1] = 0.0;
      //     xyznorm[2] = -1.0;
      //     device_label = GetDeviceLabel(buffer);
      //     fgets(buffer, 255, stream);
      //     sscanf(buffer, "%f %f %f %f %f %f", xyz, xyz + 1, xyz + 2, xyznorm,
      //            xyznorm + 1, xyznorm + 2);
      //     devicecopy->type = DEVICE_SMOKE;
      //     devicecopy->act_time = -1.0;
      //     devicecopy->xyz[0] = xyz[0];
      //     devicecopy->xyz[1] = xyz[1];
      //     devicecopy->xyz[2] = xyz[2];
      //     normdenom = 0.0;
      //     normdenom += xyznorm[0] * xyznorm[0];
      //     normdenom += xyznorm[1] * xyznorm[1];
      //     normdenom += xyznorm[2] * xyznorm[2];
      //     normdenom = sqrt(normdenom);
      //     if (normdenom > 0.001) {
      //       xyznorm[0] /= normdenom;
      //       xyznorm[1] /= normdenom;
      //       xyznorm[2] /= normdenom;
      //     }
      //     if (device_label == NULL) {
      //       devicecopy->object =
      //           GetSmvObjectType("smoke_detector",
      //           smoke_detector_object_backup);
      //     } else {
      //       devicecopy->object =
      //           GetSmvObjectType(device_label, smoke_detector_object_backup);
      //     }
      //     GetElevAz(xyznorm, &devicecopy->dtheta, devicecopy->rotate_axis,
      //     NULL);

      //     InitDevice(devicecopy, xyz, 0, NULL, NULL, xyznorm, 0, 0, NULL,
      //     NULL);

      //     devicecopy++;
      //     ndeviceinfo++;
      //   }
      continue;
    }
    if (MatchSMV(buffer, "CLASS_OF_PARTICLES") == 1 ||
        MatchSMV(buffer, "CLASS_OF_HUMANS") == 1) {
      //   partclassdata *partclassi;
      //   char *device_ptr;
      //   char *prop_id;
      //   char prop_buffer[255];

      //   partclassi = partclassinfo + npartclassinfo;
      //   partclassi->kind = PARTICLES;
      //   if (MatchSMV(buffer, "CLASS_OF_HUMANS") == 1) partclassi->kind =
      //   HUMANS; fgets(buffer, 255, stream);

      //   GetLabels(buffer, partclassi->kind, &device_ptr, &prop_id,
      //   prop_buffer); partclassi->prop = GetPropID(prop_id);
      //   UpdatePartClassDepend(partclassi);

      //   npartclassinfo++;
      continue;
    }

    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ HRRPUVCUT ++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "HRRPUVCUT") == 1) {
      //   int nhrrpuvcut;

      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i", &nhrrpuvcut);
      //   if (nhrrpuvcut >= 1) {
      //     fgets(buffer, 255, stream);
      //     sscanf(buffer, "%f", &global_hrrpuv_cutoff_default);
      //     global_hrrpuv_cutoff = global_hrrpuv_cutoff_default;
      //     load_hrrpuv_cutoff = global_hrrpuv_cutoff;
      //     for (i = 1; i < nhrrpuvcut; i++) {
      //       fgets(buffer, 255, stream);
      //     }
      //   }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "OFFSET") == 1) {
      //   meshdata *meshi;

      //   ioffset++;
      //   meshi = meshinfo + ioffset - 1;
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%f %f %f", meshi->offset, meshi->offset + 1,
      //          meshi->offset + 2);
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ FIRE ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "FIRE") == 1) {
      // firedata *firei;
      // int roomnumber;

      // ifire++;
      // if (fgets(buffer, 255, stream) == NULL) {
      //   BREAK;
      // }
      // firei = fireinfo + ifire - 1;
      // sscanf(buffer, "%i %f %f %f", &roomnumber, &firei->x, &firei->y,
      //        &firei->z);
      // if (roomnumber >= 1 && roomnumber <= nrooms) {
      //   roomdata *roomi;

      //   roomi = roominfo + roomnumber - 1;
      //   firei->valid = 1;
      //   firei->roomnumber = roomnumber;
      //   firei->absx = roomi->x0 + firei->x;
      //   firei->absy = roomi->y0 + firei->y;
      //   firei->absz = roomi->z0 + firei->z;
      // } else {
      //   firei->valid = 0;
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ PDIM ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "PDIM") == 1) {
      // meshdata *meshi;
      // float *meshrgb;

      // ipdim++;
      // meshi = meshinfo + ipdim - 1;
      // meshrgb = meshi->meshrgb;

      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%f %f %f %f %f %f %f %f %f", &xbar0, &xbar, &ybar0,
      // &ybar,
      //        &zbar0, &zbar, meshrgb, meshrgb + 1, meshrgb + 2);

      // if (meshrgb[0] != 0.0 || meshrgb[1] != 0.0 || meshrgb[2] != 0.0) {
      //   meshi->meshrgb_ptr = meshi->meshrgb;
      // } else {
      //   meshi->meshrgb_ptr = NULL;
      // }

      // meshi->xyz_bar0[XXX] = xbar0;
      // meshi->xyz_bar[XXX] = xbar;
      // meshi->xcen = (xbar + xbar0) / 2.0;
      // meshi->xyz_bar0[YYY] = ybar0;
      // meshi->xyz_bar[YYY] = ybar;
      // meshi->ycen = (ybar + ybar0) / 2.0;
      // meshi->xyz_bar0[ZZZ] = zbar0;
      // meshi->xyz_bar[ZZZ] = zbar;
      // meshi->zcen = (zbar + zbar0) / 2.0;
      // InitBoxClipInfo(&(meshi->box_clipinfo), xbar0, xbar, ybar0, ybar,
      // zbar0,
      //                 zbar);
      // if (ntrnx == 0) {
      //   int nn;

      //   for (nn = 0; nn <= meshi->ibar; nn++) {
      //     meshi->xplt[nn] =
      //         xbar0 + (float)nn * (xbar - xbar0) / (float)meshi->ibar;
      //   }
      // }
      // if (ntrny == 0) {
      //   int nn;

      //   for (nn = 0; nn <= meshi->jbar; nn++) {
      //     meshi->yplt[nn] =
      //         ybar0 + (float)nn * (ybar - ybar0) / (float)meshi->jbar;
      //   }
      // }
      // if (ntrnz == 0) {
      //   int nn;

      //   for (nn = 0; nn <= meshi->kbar; nn++) {
      //     meshi->zplt[nn] =
      //         zbar0 + (float)nn * (zbar - zbar0) / (float)meshi->kbar;
      //   }
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ TRNX ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "TRNX") == 1) {
      // float *xpltcopy;
      // int idummy;
      // int nn;

      // itrnx++;
      // xpltcopy = meshinfo[itrnx - 1].xplt;
      // ibartemp = meshinfo[itrnx - 1].ibar;
      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%i ", &idummy);
      // for (nn = 0; nn < idummy; nn++) {
      //   fgets(buffer, 255, stream);
      // }
      // for (nn = 0; nn <= ibartemp; nn++) {
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i %f", &idummy, xpltcopy);
      //   xpltcopy++;
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ TRNY ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "TRNY") == 1) {
      // float *ypltcopy;
      // int idummy;
      // int nn;

      // itrny++;
      // ypltcopy = meshinfo[itrny - 1].yplt;
      // jbartemp = meshinfo[itrny - 1].jbar;
      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%i ", &idummy);
      // for (nn = 0; nn < idummy; nn++) {
      //   fgets(buffer, 255, stream);
      // }
      // for (nn = 0; nn <= jbartemp; nn++) {
      //   //        if(jbartemp==2&&nn==2)continue;
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i %f", &idummy, ypltcopy);
      //   ypltcopy++;
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ TRNZ ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "TRNZ") == 1) {
      // float *zpltcopy;
      // int idummy;
      // int nn;

      // itrnz++;
      // zpltcopy = meshinfo[itrnz - 1].zplt;
      // kbartemp = meshinfo[itrnz - 1].kbar;
      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%i ", &idummy);
      // for (nn = 0; nn < idummy; nn++) {
      //   fgets(buffer, 255, stream);
      // }
      // for (nn = 0; nn <= kbartemp; nn++) {
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i %f", &idummy, zpltcopy);
      //   zpltcopy++;
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ TREE ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    /*
typedef struct {
  float xyz[3];
  float trunk_diam;
  float tree_height;
  float base_diam;
  float base_height;
*/
    if (MatchSMV(buffer, "TREE") == 1) {
      // fgets(buffer, 255, stream);
      // if (ntreeinfo != 0) continue;
      // sscanf(buffer, "%i", &ntreeinfo);
      // if (ntreeinfo > 0) {
      //   NewMemory((void **)&treeinfo, sizeof(treedata) * ntreeinfo);
      //   for (i = 0; i < ntreeinfo; i++) {
      //     treedata *treei;
      //     float *xyz;

      //     treei = treeinfo + i;
      //     treei->time_char = -1.0;
      //     treei->time_complete = -1.0;
      //     xyz = treei->xyz;
      //     fgets(buffer, 255, stream);
      //     sscanf(buffer, "%f %f %f %f %f %f %f", xyz, xyz + 1, xyz + 2,
      //            &treei->trunk_diam, &treei->tree_height, &treei->base_diam,
      //            &treei->base_height);
      //   }
      // }
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TREESTATE ++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "TREESTATE") == 1) {
      // int tree_index, tree_state;
      // float tree_time;
      // treedata *treei;

      // if (ntreeinfo == 0) continue;
      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%i %i %f", &tree_index, &tree_state, &tree_time);
      // if (tree_index >= 1 && tree_index <= ntreeinfo) {
      //   treei = treeinfo + tree_index - 1;
      //   if (tree_state == 1) {
      //     treei->time_char = tree_time;
      //   } else if (tree_state == 2) {
      //     treei->time_complete = tree_time;
      //   }
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ OBST ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "OBST") == 1) {
      // meshdata *meshi;
      // propdata *prop;
      // char *proplabel;
      // int n_blocks = 0;
      // int n_blocks_normal = 0;
      // unsigned char *is_block_terrain = NULL;
      // int iblock;
      // int nn;
      // size_t len;

      // CheckMemoryOff;
      // iobst++;
      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%i", &n_blocks);

      // meshi = meshinfo + iobst - 1;
      // if (n_blocks <= 0) n_blocks = 0;
      // meshi->nbptrs = n_blocks;
      // n_blocks_normal = n_blocks;
      // if (n_blocks == 0) continue;

      // if (auto_terrain == 1 || manual_terrain == 1) {
      //   is_block_terrain = meshi->is_block_terrain;
      //   n_blocks_normal = 0;
      //   for (iblock = 0; iblock < n_blocks; iblock++) {
      //     if (is_block_terrain == NULL || is_block_terrain[iblock] == 0)
      //       n_blocks_normal++;
      //   }
      //   meshi->nbptrs = n_blocks_normal;
      // }
      // meshi->blockageinfoptrs = NULL;
      // if (n_blocks_normal > 0) {
      //   NewMemory((void **)&meshi->blockageinfoptrs,
      //             sizeof(blockagedata *) * n_blocks_normal);
      // }

      // ntotal_blockages += n_blocks_normal;
      // nn = -1;
      // for (iblock = 0; iblock < n_blocks; iblock++) {
      //   int s_num[6];
      //   blockagedata *bc;

      //   if ((auto_terrain == 1 || manual_terrain == 1) &&
      //       meshi->is_block_terrain != NULL &&
      //       meshi->is_block_terrain[iblock] == 1) {
      //     fgets(buffer, 255, stream);
      //     continue;
      //   }
      //   nn++;
      //   meshi->blockageinfoptrs[nn] = NULL;
      //   NewMemory((void **)&meshi->blockageinfoptrs[nn],
      //   sizeof(blockagedata)); bc = meshi->blockageinfoptrs[nn]; InitObst(bc,
      //   surfacedefault, nn + 1, iobst - 1); fgets(buffer, 255, stream);

      //   char id_label[100], *id_labelptr;

      //   id_labelptr = strchr(buffer, '!');
      //   if (id_labelptr == NULL) {
      //     sprintf(id_label, "OB%i_%i", bc->meshindex + 1, iblock + 1);
      //     id_labelptr = id_label;
      //   } else {
      //     char *id_labelptr2;

      //     id_labelptr2 = id_labelptr;
      //     id_labelptr = TrimFrontBack(id_labelptr + 1);
      //     *id_labelptr2 = 0;
      //   }
      //   bc->id_label = GetCharPtr(id_labelptr);

      //   TrimBack(buffer);
      //   for (i = 0; i < 6; i++) {
      //     s_num[i] = -1;
      //   }
      //   proplabel = strchr(buffer, '%');
      //   prop = NULL;
      //   if (proplabel != NULL) {
      //     proplabel++;
      //     TrimBack(proplabel);
      //     proplabel = TrimFront(proplabel);
      //     for (i = 0; i < npropinfo; i++) {
      //       propdata *propi;

      //       propi = propinfo + i;
      //       if (STRCMP(proplabel, propi->label) == 0) {
      //         prop = propi;
      //         propi->inblockage = 1;
      //         break;
      //       }
      //     }
      //   }
      //   bc->prop = prop;
      //   {
      //     float t_origin[3];
      //     float *xyzEXACT;

      //     t_origin[0] = texture_origin[0];
      //     t_origin[1] = texture_origin[1];
      //     t_origin[2] = texture_origin[2];
      //     xyzEXACT = bc->xyzEXACT;
      //     sscanf(buffer, "%f %f %f %f %f %f %i %i %i %i %i %i %i %f %f %f",
      //            xyzEXACT, xyzEXACT + 1, xyzEXACT + 2, xyzEXACT + 3,
      //            xyzEXACT + 4, xyzEXACT + 5, &(bc->blockage_id), s_num +
      //            DOWN_X, s_num + UP_X, s_num + DOWN_Y, s_num + UP_Y, s_num +
      //            DOWN_Z, s_num + UP_Z, t_origin, t_origin + 1, t_origin + 2);

      //     UpdateObstBoundingBox(xyzEXACT);
      //     bc->xmin = xyzEXACT[0];
      //     bc->xmax = xyzEXACT[1];
      //     bc->ymin = xyzEXACT[2];
      //     bc->ymax = xyzEXACT[3];
      //     bc->zmin = xyzEXACT[4];
      //     bc->zmax = xyzEXACT[5];
      //     bc->texture_origin[0] = t_origin[0];
      //     bc->texture_origin[1] = t_origin[1];
      //     bc->texture_origin[2] = t_origin[2];
      //     if (bc->blockage_id < 0) {
      //       bc->changed = 1;
      //       bc->blockage_id = -bc->blockage_id;
      //     }
      //   }

      //   /* define block label */

      //   sprintf(buffer, "**blockage %i", bc->blockage_id);
      //   len = strlen(buffer);
      //   ResizeMemory((void **)&bc->label, (len + 1) * sizeof(char));
      //   strcpy(bc->label, buffer);

      //   for (i = 0; i < 6; i++) {
      //     surfdata *surfi;

      //     if (surfinfo == NULL || s_num[i] < 0 || s_num[i] >= nsurfinfo)
      //       continue;
      //     surfi = surfinfo + s_num[i];
      //     bc->surf[i] = surfi;
      //   }
      //   for (i = 0; i < 6; i++) {
      //     bc->surf[i]->used_by_obst = 1;
      //   }
      //   SetSurfaceIndex(bc);
      // }

      // nn = -1;
      // for (iblock = 0; iblock < n_blocks; iblock++) {
      //   blockagedata *bc;
      //   int *ijk;
      //   int colorindex, blocktype;

      //   if ((auto_terrain == 1 || manual_terrain == 1) &&
      //       meshi->is_block_terrain != NULL &&
      //       meshi->is_block_terrain[iblock] == 1) {
      //     fgets(buffer, 255, stream);
      //     continue;
      //   }
      //   nn++;

      //   bc = meshi->blockageinfoptrs[nn];
      //   colorindex = -1;
      //   blocktype = -1;

      //   /*
      //   OBST format:
      //   nblockages
      //   xmin xmax ymin ymax zmin zmax bid s_i1 s_i2 s_j1 s_j2 s_k1 s_k2 t_x0
      //   t_y0 t_z0
      //   ...
      //   ...

      //   bid             - blockage id
      //   s_i1, ..., s_k2 - surf indices for i1, ..., k2 blockage faces
      //   t_x0,t_y0,t_z0  - texture origin

      //   i1 i2 j1 j2 k1 k2 colorindex blocktype r g b : ignore rgb if
      //   blocktype
      //   != -3
      //   ...
      //   ...

      //   int colorindex, blocktype;
      //   colorindex: -1 default color
      //               -2 invisible
      //               -3 use r g b color
      //               >=0 color/color2/texture index
      //   blocktype: 0 regular block
      //              2 outline
      //              3 smoothed block
      //              -1 (use type from surf)
      //              (note: if blocktype&8 == 8 then this is a "terrain"
      //   blockage if so then subtract 8 and set bc->is_wuiblock=1) r g b
      //   colors used if colorindex==-3
      //   */

      //   fgets(buffer, 255, stream);
      //   ijk = bc->ijk;
      //   sscanf(buffer, "%i %i %i %i %i %i %i %i", ijk, ijk + 1, ijk + 2,
      //          ijk + 3, ijk + 4, ijk + 5, &colorindex, &blocktype);
      //   if ((blocktype & 3) == 3)
      //     blocktype -= 3; // convert any smooth blocks to 'normal' blocks
      //   if (blocktype > 0 && (blocktype & 8) == 8) {
      //     bc->is_wuiblock = 1;
      //     blocktype -= 8;
      //   }

      //   /* custom color */

      //   if (colorindex == 0 || colorindex == 7) colorindex = -3;

      //   if (colorindex == -3) {
      //     float s_color[4];

      //     ijk = bc->ijk;

      //     s_color[0] = -1.0;
      //     s_color[1] = -1.0;
      //     s_color[2] = -1.0;
      //     s_color[3] = 1.0;

      //     sscanf(buffer, "%i %i %i %i %i %i %i %i %f %f %f %f", ijk, ijk + 1,
      //            ijk + 2, ijk + 3, ijk + 4, ijk + 5, &colorindex, &blocktype,
      //            s_color, s_color + 1, s_color + 2, s_color + 3);
      //     if (blocktype > 0 && (blocktype & 8) == 8) {
      //       bc->is_wuiblock = 1;
      //       blocktype -= 8;
      //     }
      //     if (s_color[3] < 0.999) {
      //       bc->transparent = 1;
      //     }
      //     if (colorindex == 0 || colorindex == 7) {
      //       switch (colorindex) {
      //       case 0:
      //         s_color[0] = 1.0;
      //         s_color[1] = 1.0;
      //         s_color[2] = 1.0;
      //         break;
      //       case 7:
      //         s_color[0] = 0.0;
      //         s_color[1] = 0.0;
      //         s_color[2] = 0.0;
      //         break;
      //       default:
      //         ASSERT(FFALSE);
      //         break;
      //       }
      //       colorindex = -3;
      //     }
      //     if (s_color[0] >= 0.0 && s_color[1] >= 0.0 && s_color[2] >= 0.0) {
      //       bc->color = GetColorPtr(s_color);
      //     }
      //     bc->nnodes = (ijk[1] + 1 - ijk[0]) * (ijk[3] + 1 - ijk[2]) *
      //                  (ijk[5] + 1 - ijk[4]);
      //     bc->useblockcolor = 1;
      //   } else {
      //     if (colorindex >= 0) {
      //       bc->color = GetColorPtr(rgb[nrgb + colorindex]);
      //       bc->usecolorindex = 1;
      //       bc->colorindex = colorindex;
      //       updateindexcolors = 1;
      //     }
      //   }

      //   bc->dup = 0;
      //   bc->colorindex = colorindex;
      //   bc->type = blocktype;

      //   if (colorindex == COLOR_INVISIBLE) {
      //     bc->type = BLOCK_hidden;
      //     bc->invisible = 1;
      //   }
      //   if (bc->useblockcolor == 0) {
      //     bc->color = bc->surf[0]->color;
      //   } else {
      //     if (colorindex == -1) {
      //       updateindexcolors = 1;
      //     }
      //   }
      // }
      // CheckMemoryOn;
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ CVENT ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    /*
    CVENT
    ncvents
    xmin xmax ymin ymax zmin zmax id surface_index tx ty tz % x0 y0 z0 radius
      ....  (ncvents rows)
      ....
    imin imax jmin jmax kmin kmax ventindex venttype r g b
      ....
      ....

      ventindex: -99 or 99 : use default color
                 +n or -n : use n'th palette color
                 < 0       : DO NOT draw boundary file over this vent
                 > 0       : DO draw boundary file over this vent
      vent type: 0 solid surface
                 2 outline
                 -2 hidden
      r g b:     red, green, blue color components
                 only specify if you wish to over-ride surface or default
                 range from 0.0 to 1.0
    */
    if (MatchSMV(buffer, "CVENT") == 1) {
      // cventdata *cvinfo;
      // meshdata *meshi;
      // float *origin;
      // int ncv;
      // int j;

      // icvent++;
      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%i", &ncv);

      // meshi = meshinfo + icvent - 1;
      // meshi->cventinfo = NULL;
      // meshi->ncvents = ncv;
      // if (ncv == 0) continue;
      // ncvents += ncv;

      // NewMemory((void **)&cvinfo, ncv * sizeof(cventdata));
      // meshi->cventinfo = cvinfo;

      // for (j = 0; j < ncv; j++) {
      //   cventdata *cvi;
      //   char *cbuf;
      //   int s_num[1];
      //   float t_origin[3];

      //   cvi = meshi->cventinfo + j;
      //   cvi->isOpenvent = 0;
      //   cvi->surf[0] = vent_surfacedefault;
      //   cvi->textureinfo[0] = NULL;
      //   cvi->texture_origin[0] = texture_origin[0];
      //   cvi->texture_origin[1] = texture_origin[1];
      //   cvi->texture_origin[2] = texture_origin[2];
      //   cvi->useventcolor = 1;
      //   cvi->hideboundary = 0;
      //   cvi->cvent_id = -1;
      //   cvi->color = NULL;
      //   cvi->blank = NULL;
      //   cvi->showtime = NULL;
      //   cvi->showhide = NULL;
      //   cvi->showtimelist = NULL;

      //   origin = cvi->origin;
      //   s_num[0] = -1;
      //   t_origin[0] = texture_origin[0];
      //   t_origin[1] = texture_origin[1];
      //   t_origin[2] = texture_origin[2];
      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%f %f %f %f %f %f %i %i %f %f %f", &cvi->xmin,
      //          &cvi->xmax, &cvi->ymin, &cvi->ymax, &cvi->zmin, &cvi->zmax,
      //          &cvi->cvent_id, s_num, t_origin, t_origin + 1, t_origin + 2);

      //   if (surfinfo != NULL && s_num[0] >= 0 && s_num[0] < nsurfinfo) {
      //     cvi->surf[0] = surfinfo + s_num[0];
      //     if (cvi->surf[0] != NULL &&
      //         strncmp(cvi->surf[0]->surfacelabel, "OPEN", 4) == 0) {
      //       cvi->isOpenvent = 1;
      //     }
      //     cvi->surf[0]->used_by_vent = 1;
      //   }
      //   if (t_origin[0] <= -998.0) {
      //     t_origin[0] = texture_origin[0];
      //     t_origin[1] = texture_origin[1];
      //     t_origin[2] = texture_origin[2];
      //   }
      //   cvi->texture_origin[0] = t_origin[0];
      //   cvi->texture_origin[1] = t_origin[1];
      //   cvi->texture_origin[2] = t_origin[2];
      //   origin[0] = 0.0;
      //   origin[1] = 0.0;
      //   origin[2] = 0.0;
      //   cvi->radius = 0.0;
      //   cbuf = strchr(buffer, '%');
      //   if (cbuf != NULL) {
      //     TrimBack(cbuf);
      //     cbuf++;
      //     cbuf = TrimFront(cbuf);
      //     if (strlen(cbuf) > 0) {
      //       sscanf(cbuf, "%f %f %f %f", origin, origin + 1, origin + 2,
      //              &cvi->radius);
      //     }
      //   }
      // }
      // for (j = 0; j < ncv; j++) {
      //   cventdata *cvi;
      //   float *vcolor;
      //   int venttype, ventindex;
      //   float s_color[4], s2_color[4];

      //   s2_color[0] = -1.0;
      //   s2_color[1] = -1.0;
      //   s2_color[2] = -1.0;
      //   s2_color[3] = 1.0;

      //   cvi = meshi->cventinfo + j;

      //   // use properties from &SURF

      //   cvi->type = cvi->surf[0]->type;
      //   vcolor = cvi->surf[0]->color;
      //   cvi->color = vcolor;

      //   s_color[0] = vcolor[0];
      //   s_color[1] = vcolor[1];
      //   s_color[2] = vcolor[2];
      //   s_color[3] = vcolor[3];
      //   venttype = -99;

      //   fgets(buffer, 255, stream);
      //   sscanf(buffer, "%i %i %i %i %i %i %i %i %f %f %f", &cvi->imin,
      //          &cvi->imax, &cvi->jmin, &cvi->jmax, &cvi->kmin, &cvi->kmax,
      //          &ventindex, &venttype, s2_color, s2_color + 1, s2_color + 2);

      //   // use color from &VENT

      //   if (s2_color[0] >= 0.0 && s2_color[1] >= 0.0 && s2_color[2] >= 0.0) {
      //     s_color[0] = s2_color[0];
      //     s_color[1] = s2_color[1];
      //     s_color[2] = s2_color[2];
      //     cvi->useventcolor = 1;
      //   }
      //   if (ventindex < 0) cvi->hideboundary = 1;
      //   if (venttype != -99) cvi->type = venttype;

      //   // use pallet color

      //   if (ABS(ventindex) != 99) {
      //     ventindex = ABS(ventindex);
      //     if (ventindex > nrgb2 - 1) ventindex = nrgb2 - 1;
      //     s_color[0] = rgb[nrgb + ventindex][0];
      //     s_color[1] = rgb[nrgb + ventindex][1];
      //     s_color[2] = rgb[nrgb + ventindex][2];
      //     s_color[3] = 1.0;
      //     cvi->useventcolor = 1;
      //   }
      //   s_color[3] = 1.0; // set color to opaque until CVENT transparency is
      //                     // implemented
      //   cvi->color = GetColorPtr(s_color);
      // }
      continue;
    }

    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ VENT ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    /*
    new VENT format:

    xmin xmax ymin ymax zmin zmax ventid surf_index tx ty tz

    xyz min/max - vent boundaries
    ventid - index of vent
    surf_index - index of SURF
    tx, ty, yz - texture origin

    i1 i2 j1 j2 k1 k2 ventindex venttype r g b a

    ventindex: -99 or 99 : use default color
               +n or -n : use n'th palette color
               < 0       : DO NOT draw boundary file over this vent
               > 0       : DO draw boundary file over this vent
    vent type: 0 solid surface
               2 outline
              -2 hidden
    r g b:     red, green, blue color components
               only specify if you wish to over-ride surface or default
               range from 0.0 to 1.0
    */

    if (MatchSMV(buffer, "VENT") == 1) {
      // meshdata *meshi;
      // ventdata *vinfo;
      // float *xplttemp, *yplttemp, *zplttemp;
      // int nn;

      // ivent++;
      // meshi = meshinfo + ivent - 1;
      // xplttemp = meshi->xplt;
      // yplttemp = meshi->yplt;
      // zplttemp = meshi->zplt;
      // if (nVENT == 0) {
      //   strcpy(buffer, "0 0");
      //   nVENT = 1;
      // } else {
      //   fgets(buffer, 255, stream);
      // }
      // ndummyvents = 0;
      // sscanf(buffer, "%i %i", &nvents, &ndummyvents);
      // if (ndummyvents != 0) {
      //   visFloor = 0;
      //   visCeiling = 0;
      //   visWalls = 0;
      // }
      // meshi->nvents = nvents;
      // meshi->ndummyvents = ndummyvents;
      // vinfo = NULL;
      // meshi->ventinfo = vinfo;
      // if (NewMemory((void **)&vinfo, (nvents + 12) * sizeof(ventdata)) == 0)
      //   return 2;
      // meshi->ventinfo = vinfo;

      // for (nn = 0; nn < nvents + 12; nn++) {
      //   int s_num[6];
      //   ventdata *vi;

      //   vi = vinfo + nn;
      //   vi->type = VENT_SOLID;
      //   vi->dummyptr = NULL;
      //   vi->transparent = 0;
      //   vi->useventcolor = 0;
      //   vi->usecolorindex = 0;
      //   vi->nshowtime = 0;
      //   vi->isOpenvent = 0;
      //   vi->isMirrorvent = 0;
      //   vi->hideboundary = 0;
      //   vi->surf[0] = vent_surfacedefault;
      //   vi->textureinfo[0] = NULL;
      //   vi->texture_origin[0] = texture_origin[0];
      //   vi->texture_origin[1] = texture_origin[1];
      //   vi->texture_origin[2] = texture_origin[2];
      //   vi->colorindex = -1;
      //   if (nn > nvents - ndummyvents - 1 && nn < nvents) {
      //     vi->dummy = 1;
      //   } else {
      //     vi->dummy = 0;
      //   }
      //   s_num[0] = -1;
      //   if (nn < nvents) {
      //     float t_origin[3];

      //     t_origin[0] = texture_origin[0];
      //     t_origin[1] = texture_origin[1];
      //     t_origin[2] = texture_origin[2];
      //     fgets(buffer, 255, stream);
      //     sscanf(buffer, "%f %f %f %f %f %f %i %i %f %f %f", &vi->xmin,
      //            &vi->xmax, &vi->ymin, &vi->ymax, &vi->zmin, &vi->zmax,
      //            &vi->vent_id, s_num, t_origin, t_origin + 1, t_origin + 2);
      //     if (t_origin[0] <= -998.0) {
      //       t_origin[0] = texture_origin[0];
      //       t_origin[1] = texture_origin[1];
      //       t_origin[2] = texture_origin[2];
      //     }
      //     vi->texture_origin[0] = t_origin[0];
      //     vi->texture_origin[1] = t_origin[1];
      //     vi->texture_origin[2] = t_origin[2];
      //   } else {
      //     vi->xmin = meshi->xyz_bar0[XXX] + meshi->offset[XXX];
      //     vi->xmax = meshi->xyz_bar[XXX] + meshi->offset[XXX];
      //     vi->ymin = meshi->xyz_bar0[YYY] + meshi->offset[YYY];
      //     vi->ymax = meshi->xyz_bar[YYY] + meshi->offset[YYY];
      //     vi->zmin = meshi->xyz_bar0[ZZZ] + meshi->offset[ZZZ];
      //     vi->zmax = meshi->xyz_bar[ZZZ] + meshi->offset[ZZZ];
      //     s_num[0] = -1;
      //     switch (nn - nvents) {
      //     case DOWN_Y:
      //     case DOWN_Y + 6:
      //       vi->ymax = vi->ymin;
      //       break;
      //     case UP_X:
      //     case UP_X + 6:
      //       vi->xmin = vi->xmax;
      //       break;
      //     case UP_Y:
      //     case UP_Y + 6:
      //       vi->ymin = vi->ymax;
      //       break;
      //     case DOWN_X:
      //     case DOWN_X + 6:
      //       vi->xmax = vi->xmin;
      //       break;
      //     case DOWN_Z:
      //     case DOWN_Z + 6:
      //       vi->zmax = vi->zmin;
      //       break;
      //     case UP_Z:
      //     case UP_Z + 6:
      //       vi->zmin = vi->zmax;
      //       break;
      //     default:
      //       ASSERT(FFALSE);
      //       break;
      //     }
      //     if (nn >= nvents + 6) {
      //       vi->surf[0] = exterior_surfacedefault;
      //     }
      //   }
      //   if (surfinfo != NULL && s_num[0] >= 0 && s_num[0] < nsurfinfo) {
      //     vi->surf[0] = surfinfo + s_num[0];
      //     if (strncmp(vi->surf[0]->surfacelabel, "OPEN", 4) == 0)
      //       vi->isOpenvent = 1;
      //     if (strncmp(vi->surf[0]->surfacelabel, "MIRROR", 6) == 0)
      //       vi->isMirrorvent = 1;
      //     vi->surf[0]->used_by_vent = 1;
      //   }
      //   vi->color_bak = surfinfo[0].color;
      // }
      // for (nn = 0; nn < nvents + 12; nn++) {
      //   ventdata *vi;
      //   int iv1, iv2, jv1, jv2, kv1, kv2;
      //   float s_color[4];
      //   int venttype, ventindex;

      //   vi = vinfo + nn;
      //   vi->type = vi->surf[0]->type;
      //   vi->color = vi->surf[0]->color;
      //   s_color[0] = vi->surf[0]->color[0];
      //   s_color[1] = vi->surf[0]->color[1];
      //   s_color[2] = vi->surf[0]->color[2];
      //   s_color[3] = vi->surf[0]->color[3];
      //   venttype = -99;
      //   if (nn < nvents) {
      //     float s2_color[4];

      //     s2_color[0] = -1.0;
      //     s2_color[1] = -1.0;
      //     s2_color[2] = -1.0;
      //     s2_color[3] = 1.0;

      //     fgets(buffer, 255, stream);
      //     sscanf(buffer, "%i %i %i %i %i %i %i %i %f %f %f %f", &iv1, &iv2,
      //            &jv1, &jv2, &kv1, &kv2, &ventindex, &venttype, s2_color,
      //            s2_color + 1, s2_color + 2, s2_color + 3);
      //     if (s2_color[0] >= 0.0 && s2_color[1] >= 0.0 && s2_color[2] >= 0.0)
      //     {
      //       s_color[0] = s2_color[0];
      //       s_color[1] = s2_color[1];
      //       s_color[2] = s2_color[2];

      //       if (s2_color[3] < 0.99) {
      //         vi->transparent = 1;
      //         s_color[3] = s2_color[3];
      //       }
      //       vi->useventcolor = 1;
      //       ventindex = SIGN(ventindex) * 99;
      //     }
      //     if (ventindex < 0) vi->hideboundary = 1;
      //     if (venttype != -99) vi->type = venttype;
      //     if (ABS(ventindex) != 99) {
      //       ventindex = ABS(ventindex);
      //       if (ventindex > nrgb2 - 1) ventindex = nrgb2 - 1;
      //       s_color[0] = rgb[nrgb + ventindex][0];
      //       s_color[1] = rgb[nrgb + ventindex][1];
      //       s_color[2] = rgb[nrgb + ventindex][2];
      //       s_color[3] = 1.0;
      //       vi->colorindex = ventindex;
      //       vi->usecolorindex = 1;
      //       vi->useventcolor = 1;
      //       updateindexcolors = 1;
      //     }
      //     vi->color = GetColorPtr(s_color);
      //   } else {
      //     iv1 = 0;
      //     iv2 = meshi->ibar;
      //     jv1 = 0;
      //     jv2 = meshi->jbar;
      //     kv1 = 0;
      //     kv2 = meshi->kbar;
      //     ventindex = -99;
      //     vi->dir = nn - nvents;
      //     if (vi->dir > 5) vi->dir -= 6;
      //     vi->dir2 = 0;
      //     switch (nn - nvents) {
      //     case DOWN_Y:
      //     case DOWN_Y + 6:
      //       jv2 = jv1;
      //       if (nn >= nvents + 6) vi->dir = UP_Y;
      //       vi->dir2 = YDIR;
      //       break;
      //     case UP_X:
      //     case UP_X + 6:
      //       iv1 = iv2;
      //       if (nn >= nvents + 6) vi->dir = DOWN_X;
      //       vi->dir2 = XDIR;
      //       break;
      //     case UP_Y:
      //     case UP_Y + 6:
      //       jv1 = jv2;
      //       if (nn >= nvents + 6) vi->dir = DOWN_Y;
      //       vi->dir2 = YDIR;
      //       break;
      //     case DOWN_X:
      //     case DOWN_X + 6:
      //       iv2 = iv1;
      //       if (nn >= nvents + 6) vi->dir = UP_X;
      //       vi->dir2 = XDIR;
      //       break;
      //     case DOWN_Z:
      //     case DOWN_Z + 6:
      //       kv2 = kv1;
      //       if (nn >= nvents + 6) vi->dir = UP_Z;
      //       vi->dir2 = ZDIR;
      //       break;
      //     case UP_Z:
      //     case UP_Z + 6:
      //       kv1 = kv2;
      //       if (nn >= nvents + 6) vi->dir = DOWN_Z;
      //       vi->dir2 = ZDIR;
      //       break;
      //     default:
      //       ASSERT(FFALSE);
      //       break;
      //     }
      //   }
      //   if (vi->transparent == 1) nvent_transparent++;
      //   vi->linewidth = &ventlinewidth;
      //   vi->showhide = NULL;
      //   vi->showtime = NULL;
      //   vi->showtimelist = NULL;
      //   vi->xvent1 = xplttemp[iv1];
      //   vi->xvent2 = xplttemp[iv2];
      //   vi->yvent1 = yplttemp[jv1];
      //   vi->yvent2 = yplttemp[jv2];
      //   vi->zvent1 = zplttemp[kv1];
      //   vi->zvent2 = zplttemp[kv2];
      //   vi->xvent1_orig = xplttemp[iv1];
      //   vi->xvent2_orig = xplttemp[iv2];
      //   vi->yvent1_orig = yplttemp[jv1];
      //   vi->yvent2_orig = yplttemp[jv2];
      //   vi->zvent1_orig = zplttemp[kv1];
      //   vi->zvent2_orig = zplttemp[kv2];
      //   vi->imin = iv1;
      //   vi->imax = iv2;
      //   vi->jmin = jv1;
      //   vi->jmax = jv2;
      //   vi->kmin = kv1;
      //   vi->kmax = kv2;
      //   if (nn >= nvents && nn < nvents + 6) {
      //     vi->color = foregroundcolor;
      //   }
      //   ASSERT(vi->color != NULL);
      // }
      // for (nn = 0; nn < nvents - ndummyvents; nn++) {
      //   int j;
      //   ventdata *vi;

      //   vi = meshi->ventinfo + nn;
      //   for (j = nvents - ndummyvents; j < nvents;
      //        j++) { // look for dummy vent that matches real vent
      //     ventdata *vj;

      //     vj = meshi->ventinfo + j;
      //     if (vi->imin != vj->imin && vi->imax != vj->imax) continue;
      //     if (vi->jmin != vj->jmin && vi->jmax != vj->jmax) continue;
      //     if (vi->kmin != vj->kmin && vi->kmax != vj->kmax) continue;
      //     vi->dummyptr = vj;
      //     vj->dummyptr = vi;
      //   }
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ CHID +++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "CHID") == 1) {
      // int return_val;

      // return_val = ParseCHIDProcess(stream, NO_SCAN);
      // if (return_val == RETURN_BREAK) {
      //   BREAK;
      // } else if (return_val == RETURN_CONTINUE) {
      //   continue;
      // } else {
      //   ASSERT(FFALSE);
      // }
      continue;
    }

    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SMOKE3D ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SMOKE3D") == 1 || MatchSMV(buffer, "SMOKF3D") == 1 ||
        MatchSMV(buffer, "SMOKG3D") == 1) {
      // int return_val;

      // return_val = ParseSMOKE3DProcess(stream, buffer, &nn_smoke3d, &ioffset,
      //                                  &ismoke3dcount, &ismoke3d);
      // if (return_val == RETURN_BREAK) {
      //   BREAK;
      // } else if (return_val == RETURN_CONTINUE) {
      //   continue;
      // } else if (return_val == RETURN_TWO) {
      //   return 2;
      // } else {
      //   ASSERT(FFALSE);
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ PART ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "PRT5") == 1 || MatchSMV(buffer, "EVA5") == 1) {
      // int return_val;

      // return_val = ParsePRT5Process(stream, buffer, &nn_part, &ipart,
      // &ioffset); if (return_val == RETURN_BREAK) {
      //   BREAK;
      // } else if (return_val == RETURN_CONTINUE) {
      //   continue;
      // } else if (return_val == RETURN_TWO) {
      //   return 2;
      // } else {
      //   ASSERT(FFALSE);
      // }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SLCF ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if ((MatchSMV(buffer, "SLCF") == 1) || (MatchSMV(buffer, "SLCC") == 1) ||
        (MatchSMV(buffer, "SLCD") == 1) || (MatchSMV(buffer, "SLCT") == 1) ||
        (MatchSMV(buffer, "BNDS") == 1)) {
      // int return_val;

      // return_val =
      //     ParseSLCFProcess(NO_SCAN, stream, buffer, &nn_slice, ioffset,
      //                      &nslicefiles, &sliceinfo_copy, &patchgeom,
      //                      buffers);
      // if (return_val == RETURN_BREAK) {
      //   BREAK;
      // } else if (return_val == RETURN_CONTINUE) {
      //   continue;
      // } else if (return_val == RETURN_TWO) {
      //   return 2;
      // } else if (return_val == RETURN_PROCEED) {
      // } else {
      //   ASSERT(FFALSE);
      // }
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ BNDF ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "BNDF") == 1 || MatchSMV(buffer, "BNDC") == 1 ||
        MatchSMV(buffer, "BNDE") == 1 || MatchSMV(buffer, "BNDS") == 1) {
      // int return_val;

      // return_val = ParseBNDFProcess(stream, buffer, &nn_patch, &ioffset,
      //                               &patchgeom, &ipatch, buffers);
      // if (return_val == RETURN_BREAK) {
      //   BREAK;
      // } else if (return_val == RETURN_CONTINUE) {
      //   continue;
      // } else if (return_val == RETURN_TWO) {
      //   return 2;
      // } else {
      //   ASSERT(FFALSE);
      // }
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ ISOF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if (MatchSMV(buffer, "ISOF") == 1 || MatchSMV(buffer, "TISOF") == 1 ||
        MatchSMV(buffer, "ISOG") == 1 || MatchSMV(buffer, "TISOG") == 1) {
      // int return_val;

      // return_val = ParseISOFProcess(stream, buffer, &iiso, &ioffset, &nn_iso,
      //                               nisos_per_mesh);
      // if (return_val == RETURN_BREAK) {
      //   BREAK;
      // } else if (return_val == RETURN_CONTINUE) {
      //   continue;
      // } else {
      //   ASSERT(FFALSE);
      // }
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ MINMAXBNDF +++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    if (MatchSMV(buffer, "MINMAXBNDF") == 1) {
      // char *file_ptr, file2_local[1024];
      // float valmin, valmax;
      // float percentile_min, percentile_max;

      // fgets(buffer, 255, stream);
      // strcpy(file2_local, buffer);
      // file_ptr = file2_local;
      // TrimBack(file2_local);
      // file_ptr = TrimFront(file2_local);

      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%f %f %f %f", &valmin, &valmax, &percentile_min,
      //        &percentile_max);

      // for (i = 0; i < npatchinfo; i++) {
      //   patchdata *patchi;

      //   patchi = patchinfo + i;
      //   if (strcmp(file_ptr, patchi->file) == 0) {
      //     patchi->diff_valmin = percentile_min;
      //     patchi->diff_valmax = percentile_max;
      //     break;
      //   }
      // }
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ MINMAXSLCF +++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "MINMAXSLCF") == 1) {
      // char *file_ptr, file2_local[1024];
      // float valmin, valmax;
      // float percentile_min, percentile_max;

      // fgets(buffer, 255, stream);
      // strcpy(file2_local, buffer);
      // file_ptr = file2_local;
      // TrimBack(file2_local);
      // file_ptr = TrimFront(file2_local);

      // fgets(buffer, 255, stream);
      // sscanf(buffer, "%f %f %f %f", &valmin, &valmax, &percentile_min,
      //        &percentile_max);

      // for (i = 0; i < nsliceinfo; i++) {
      //   slicedata *slicei;

      //   slicei = sliceinfo + i;
      //   if (strcmp(file_ptr, slicei->file) == 0) {
      //     slicei->diff_valmin = percentile_min;
      //     slicei->diff_valmax = percentile_max;
      //     break;
      //   }
      // }
      continue;
    }
  }
  return 0;
}
