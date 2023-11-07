
/* ------------------ ReadSMVDynamic ------------------------ */

void ReadSMVDynamic(char *file) {
  int ioffset;
  float time_local;
  int i;
  int nn_plot3d = 0, iplot3d = 0;
  int do_pass2 = 0, do_pass3 = 0, minmaxpl3d = 0;
  int nplot3dinfo_old;
  bufferstreamdata streaminfo, *stream = &streaminfo;

  stream->fileinfo = fopen_buffer(file, "r", 1, 0);

  nplot3dinfo_old = nplot3dinfo;

  updatefacelists = 1;
  updatemenu = 1;
  if (nplot3dinfo > 0) {
    int n;

    for (i = 0; i < nplot3dinfo; i++) {
      plot3ddata *plot3di;

      plot3di = plot3dinfo + i;
      for (n = 0; n < 6; n++) {
        FreeLabels(&plot3di->label[n]);
      }
      FREEMEMORY(plot3di->reg_file);
      FREEMEMORY(plot3di->comp_file);
    }
    //    FREEMEMORY(plot3dinfo);
  }
  nplot3dinfo = 0;

  for (i = 0; i < nmeshes; i++) {
    meshdata *meshi;
    int j;

    meshi = meshinfo + i;
    for (j = 0; j < meshi->nbptrs; j++) {
      blockagedata *bc_local;

      bc_local = meshi->blockageinfoptrs[j];
      bc_local->nshowtime = 0;
      FREEMEMORY(bc_local->showtime);
      FREEMEMORY(bc_local->showhide);
    }
    for (j = 0; j < meshi->nvents; j++) {
      ventdata *vi;

      vi = meshi->ventinfo + j;
      vi->nshowtime = 0;
      FREEMEMORY(vi->showhide);
      FREEMEMORY(vi->showtime);
    }
    for (j = 0; j < meshi->ncvents; j++) {
      cventdata *cvi;

      cvi = meshi->cventinfo + j;
      cvi->nshowtime = 0;
      FREEMEMORY(cvi->showhide);
      FREEMEMORY(cvi->showtime);
    }
  }
  for (i = ndeviceinfo_exp; i < ndeviceinfo; i++) {
    devicedata *devicei;

    devicei = deviceinfo + i;
    devicei->nstate_changes = 0;
    FREEMEMORY(devicei->act_times);
    FREEMEMORY(devicei->state_values);
  }

  ioffset = 0;

  // ------------------------------- pass 1 dynamic - start
  // ------------------------------------

  for (;;) {
    char buffer[255], buffer2[255];

    if (FGETS(buffer, 255, stream) == NULL) break;
    if (strncmp(buffer, " ", 1) == 0 || buffer[0] == 0) continue;
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "OFFSET") == 1) {
      ioffset++;
      continue;
    }
    /*
        +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        ++++++++++++++++++++++ PL3D ++++++++++++++++++++++++++++++
        +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      */

    if (MatchSMV(buffer, "PL3D") == 1) {
      int n;

      do_pass2 = 1;
      if (setup_only == 1 || smoke3d_only == 1) continue;
      FGETS(buffer, 255, stream);
      for (n = 0; n < 5; n++) {
        if (ReadLabels(NULL, stream, NULL) == LABEL_ERR) break;
      }
      nplot3dinfo++;
      continue;
    }
    /*
        +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        ++++++++++++++++++++++ OPEN_VENT ++++++++++++++++++++++++++++++
        +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      */
    if (MatchSMV(buffer, "OPEN_VENT") == 1 ||
        MatchSMV(buffer, "CLOSE_VENT") == 1 ||
        MatchSMV(buffer, "OPEN_CVENT") == 1 ||
        MatchSMV(buffer, "CLOSE_CVENT") == 1) {
      meshdata *meshi;
      int len;
      int showvent, blocknumber, tempval, isvent;

      do_pass2 = 1;
      showvent = 1;
      isvent = 0;
      if (MatchSMV(buffer, "CLOSE_VENT") == 1 ||
          MatchSMV(buffer, "OPEN_VENT") == 1)
        isvent = 1;
      if (MatchSMV(buffer, "CLOSE_VENT") == 1 ||
          MatchSMV(buffer, "CLOSE_CVENT") == 1)
        showvent = 0;
      if (nmeshes > 1) {
        blocknumber = ioffset - 1;
      } else {
        blocknumber = 0;
      }
      TrimBack(buffer);
      len = strlen(buffer);
      if (showvent == 1) {
        if (len > 10) {
          sscanf(buffer + 10, "%i", &blocknumber);
          blocknumber--;
          if (blocknumber < 0) blocknumber = 0;
          if (blocknumber > nmeshes - 1) blocknumber = nmeshes - 1;
        }
      } else {
        if (len > 11) {
          sscanf(buffer + 11, "%i", &blocknumber);
          blocknumber--;
          if (blocknumber < 0) blocknumber = 0;
          if (blocknumber > nmeshes - 1) blocknumber = nmeshes - 1;
        }
      }
      meshi = meshinfo + blocknumber;
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f", &tempval, &time_local);
      tempval--;
      if (tempval < 0) continue;
      if (isvent == 1) {
        if (meshi->ventinfo == NULL || tempval >= meshi->nvents) continue;
      } else {
        if (meshi->cventinfo == NULL || tempval >= meshi->ncvents) continue;
      }
      if (isvent == 1) {
        ventdata *vi;

        vi = GetCloseVent(meshi, tempval);
        vi->nshowtime++;
      } else {
        cventdata *cvi;

        cvi = meshi->cventinfo + tempval;
        cvi->showtimelist = NULL;
        cvi->nshowtime++;
      }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SHOW_OBST ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SHOW_OBST") == 1 ||
        MatchSMV(buffer, "HIDE_OBST") == 1) {
      meshdata *meshi;
      int blocknumber, blocktemp, tempval;
      blockagedata *bc;

      do_pass2 = 1;
      if (nmeshes > 1) {
        blocknumber = ioffset - 1;
      } else {
        blocknumber = 0;
      }
      if (strlen(buffer) > 10) {
        sscanf(buffer, "%s %i", buffer2, &blocktemp);
        if (blocktemp > 0 && blocktemp <= nmeshes) blocknumber = blocktemp - 1;
      }
      meshi = meshinfo + blocknumber;
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f", &tempval, &time_local);
      tempval--;
      if (tempval < 0 || tempval >= meshi->nbptrs) continue;
      bc = meshi->blockageinfoptrs[tempval];
      bc->nshowtime++;
      have_animate_blockages = 1;
      continue;
    }

    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ DUCT_ACT +++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    // DUCT_ACT
    //  duct_label (char)
    //  time state  (float int)
    if (MatchSMV(buffer, "DUCT_ACT") == 1) {
      char *ductname;
      hvacductdata *ducti;
      float *act_time, *act_times;
      int *act_state, *act_states, dummy;

      ductname = strchr(buffer, ' ');
      if (ductname == NULL) continue;
      ductname = TrimFrontBack(ductname + 1);
      ducti = GetHVACDuctID(ductname);
      if (ducti == NULL) continue;

      act_times = ducti->act_times;
      act_states = ducti->act_states;
      ducti->nact_times++;
      if (ducti->nact_times == 1) {
        NewMemory((void **)&act_times, ducti->nact_times * sizeof(float));
        NewMemory((void **)&act_states, ducti->nact_times * sizeof(int));
      } else {
        ResizeMemory((void **)&act_times, ducti->nact_times * sizeof(float));
        ResizeMemory((void **)&act_states, ducti->nact_times * sizeof(int));
      }

      act_time = act_times + ducti->nact_times - 1;
      act_state = act_states + ducti->nact_times - 1;

      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &dummy, act_time, act_state);
      ONEORZERO(*act_state);
      ducti->act_times = act_times;
      ducti->act_states = act_states;
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ DEVICE_ACT +++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if (MatchSMV(buffer, "DEVICE_ACT") == 1) {
      devicedata *devicei;
      int idevice;
      float act_time;
      int act_state;

      do_pass2 = 1;
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &idevice, &act_time, &act_state);
      idevice--;
      if (idevice >= 0 && idevice < ndeviceinfo) {
        devicei = deviceinfo + idevice;
        devicei->act_time = act_time;
        devicei->nstate_changes++;
      }

      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ HEAT_ACT +++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "HEAT_ACT") == 1) {
      meshdata *meshi;
      int blocknumber, blocktemp;
      int nn;

      if (nmeshes > 1) {
        blocknumber = ioffset - 1;
      } else {
        blocknumber = 0;
      }
      if (strlen(buffer) > 9) {
        sscanf(buffer, "%s %i", buffer2, &blocktemp);
        if (blocktemp > 0 && blocktemp <= nmeshes) blocknumber = blocktemp - 1;
      }
      meshi = meshinfo + blocknumber;
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f", &nn, &time_local);
      if (meshi->theat != NULL && nn >= 1 && nn <= meshi->nheat) {
        int idev;
        int count = 0;

        meshi->theat[nn - 1] = time_local;
        for (idev = 0; idev < ndeviceinfo; idev++) {
          devicedata *devicei;

          devicei = deviceinfo + idev;
          if (devicei->type == DEVICE_HEAT) {
            count++;
            if (nn == count) {
              devicei->act_time = time_local;
              break;
            }
          }
        }
      }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SPRK_ACT ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SPRK_ACT") == 1) {
      meshdata *meshi;
      int blocknumber, blocktemp;
      int nn;

      if (nmeshes > 1) {
        blocknumber = ioffset - 1;
      } else {
        blocknumber = 0;
      }
      if (strlen(buffer) > 9) {
        sscanf(buffer, "%s %i", buffer2, &blocktemp);
        if (blocktemp > 0 && blocktemp <= nmeshes) blocknumber = blocktemp - 1;
      }
      meshi = meshinfo + blocknumber;
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f", &nn, &time_local);
      if (meshi->tspr != NULL && nn <= meshi->nspr && nn > 0) {
        int idev;
        int count = 0;

        meshi->tspr[nn - 1] = time_local;

        for (idev = 0; idev < ndeviceinfo; idev++) {
          devicedata *devicei;

          devicei = deviceinfo + idev;
          if (devicei->type == DEVICE_SPRK) {
            count++;
            if (nn == count) {
              devicei->act_time = time_local;
              break;
            }
          }
        }
      }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SMOD_ACT ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SMOD_ACT") == 1) {
      int idev;
      int count = 0;
      int nn;

      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f", &nn, &time_local);
      for (idev = 0; idev < ndeviceinfo; idev++) {
        devicedata *devicei;

        devicei = deviceinfo + idev;
        if (devicei->type == DEVICE_SMOKE) {
          count++;
          if (nn == count) {
            devicei->act_time = time_local;
            break;
          }
        }
      }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ MINMAXPL3D +++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "MINMAXPL3D") == 1) {
      minmaxpl3d = 1;
      do_pass2 = 1;
      continue;
    }
  }

  // ------------------------------- pass 1 dynamic - end
  // ------------------------------------

  if (nplot3dinfo > 0) {
    if (plot3dinfo == NULL) {
      NewMemory((void **)&plot3dinfo, nplot3dinfo * sizeof(plot3ddata));
    } else {
      ResizeMemory((void **)&plot3dinfo, nplot3dinfo * sizeof(plot3ddata));
    }
  }
  for (i = 0; i < ndeviceinfo; i++) {
    devicedata *devicei;

    devicei = deviceinfo + i;
    devicei->istate_changes = 0;
  }

  ioffset = 0;
  REWIND(stream);

  // ------------------------------- pass 2 dynamic - start
  // ------------------------------------

  while (do_pass2 == 1) {
    char buffer[255], buffer2[255];

    if (FGETS(buffer, 255, stream) == NULL) break;
    if (strncmp(buffer, " ", 1) == 0 || buffer[0] == 0) continue;
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "OFFSET") == 1) {
      ioffset++;
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PL3D ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if (MatchSMV(buffer, "PL3D") == 1) {
      plot3ddata *plot3di;
      int len, blocknumber, blocktemp;
      char *bufferptr;

      if (setup_only == 1 || smoke3d_only == 1) continue;
      if (minmaxpl3d == 1) do_pass3 = 1;
      nn_plot3d++;
      TrimBack(buffer);
      len = strlen(buffer);
      blocknumber = 0;
      if (nmeshes > 1) {
        blocknumber = ioffset - 1;
      } else {
        blocknumber = 0;
      }
      if (strlen(buffer) > 5) {
        sscanf(buffer, "%s %f %i", buffer2, &time_local, &blocktemp);
        if (blocktemp > 0 && blocktemp <= nmeshes) blocknumber = blocktemp - 1;
      } else {
        time_local = -1.0;
      }
      if (FGETS(buffer, 255, stream) == NULL) {
        nplot3dinfo--;
        break;
      }
      bufferptr = TrimFrontBack(buffer);
      len = strlen(bufferptr);

      plot3di = plot3dinfo + iplot3d;
      for (i = 0; i < 5; i++) {
        plot3di->valmin_fds[i] = 1.0;
        plot3di->valmax_fds[i] = 0.0;
        plot3di->valmin_smv[i] = 1.0;
        plot3di->valmax_smv[i] = 0.0;
      }
      plot3di->blocknumber = blocknumber;
      plot3di->seq_id = nn_plot3d;
      plot3di->autoload = 0;
      plot3di->time = time_local;
      plot3di->finalize = 1;
      nmemory_ids++;
      plot3di->memory_id = nmemory_ids;

      for (i = 0; i < MAXPLOT3DVARS; i++) {
        plot3di->histograms[i] = NULL;
      }
      if (plot3di > plot3dinfo + nplot3dinfo_old - 1) {
        plot3di->loaded = 0;
        plot3di->display = 0;
      }

      NewMemory((void **)&plot3di->reg_file, (unsigned int)(len + 1));
      STRCPY(plot3di->reg_file, bufferptr);

      NewMemory((void **)&plot3di->bound_file, (unsigned int)(len + 4 + 1));
      STRCPY(plot3di->bound_file, bufferptr);
      STRCAT(plot3di->bound_file, ".bnd");
      plot3di->have_bound_file = NO;

      NewMemory((void **)&plot3di->comp_file, (unsigned int)(len + 4 + 1));
      STRCPY(plot3di->comp_file, bufferptr);
      STRCAT(plot3di->comp_file, ".svz");

      //   if(FILE_EXISTS_CASEDIR(plot3di->comp_file)==YES){
      //     plot3di->compression_type=COMPRESSED_ZLIB;
      //     plot3di->file=plot3di->comp_file;
      //   }
      //   else{
      //     plot3di->compression_type=UNCOMPRESSED;
      //     plot3di->file=plot3di->reg_file;
      //   }
      // disable compression for now
      plot3di->compression_type = UNCOMPRESSED;
      plot3di->file = plot3di->reg_file;

      if (fast_startup == 1 || FILE_EXISTS_CASEDIR(plot3di->file) == YES) {
        int n;
        int read_ok = YES;

        plot3di->u = -1;
        plot3di->v = -1;
        plot3di->w = -1;
        for (n = 0; n < 5; n++) {
          if (ReadLabels(&plot3di->label[n], stream, NULL) != LABEL_OK) {
            read_ok = NO;
            break;
          }
          if (STRCMP(plot3di->label[n].shortlabel, "U-VEL") == 0) {
            plot3di->u = n;
          }
          if (STRCMP(plot3di->label[n].shortlabel, "V-VEL") == 0) {
            plot3di->v = n;
          }
          if (STRCMP(plot3di->label[n].shortlabel, "W-VEL") == 0) {
            plot3di->w = n;
          }
        }
        if (read_ok == NO) {
          nplot3dinfo--;
          continue;
        }
        if (plot3di->u > -1 || plot3di->v > -1 || plot3di->w > -1) {
          plot3di->nvars = MAXPLOT3DVARS;
        } else {
          plot3di->nvars = 5;
        }
        if (NewMemory((void **)&plot3di->label[5].longlabel, 6) == 0) return;
        if (NewMemory((void **)&plot3di->label[5].shortlabel, 6) == 0) return;
        if (NewMemory((void **)&plot3di->label[5].unit, 4) == 0) return;

        STRCPY(plot3di->label[5].longlabel, "Speed");
        STRCPY(plot3di->label[5].shortlabel, "Speed");
        STRCPY(plot3di->label[5].unit, "m/s");

        STRCPY(plot3di->longlabel, "");
        for (n = 0; n < 5; n++) {
          STRCAT(plot3di->longlabel, plot3di->label[n].shortlabel);
          if (n != 4) STRCAT(plot3di->longlabel, ", ");
        }

        iplot3d++;
      } else {
        int n;

        for (n = 0; n < 5; n++) {
          if (ReadLabels(&plot3di->label[n], stream, NULL) == LABEL_ERR) break;
        }
        nplot3dinfo--;
      }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ OPEN_VENT ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "OPEN_VENT") == 1 ||
        MatchSMV(buffer, "CLOSE_VENT") == 1 ||
        MatchSMV(buffer, "OPEN_CVENT") == 1 ||
        MatchSMV(buffer, "CLOSE_CVENT") == 1) {
      meshdata *meshi;
      int len, showvent, blocknumber, tempval, isvent;

      showvent = 1;
      isvent = 0;
      if (MatchSMV(buffer, "CLOSE_VENT") == 1 ||
          MatchSMV(buffer, "OPEN_VENT") == 1)
        isvent = 1;
      if (MatchSMV(buffer, "CLOSE_VENT") == 1 ||
          MatchSMV(buffer, "CLOSE_CVENT") == 1)
        showvent = 0;
      if (nmeshes > 1) {
        blocknumber = ioffset - 1;
      } else {
        blocknumber = 0;
      }
      TrimBack(buffer);
      len = strlen(buffer);
      if (showvent == 1) {
        if (len > 10) {
          sscanf(buffer + 10, "%i", &blocknumber);
          blocknumber--;
          if (blocknumber < 0) blocknumber = 0;
          if (blocknumber > nmeshes - 1) blocknumber = nmeshes - 1;
        }
      } else {
        if (len > 11) {
          sscanf(buffer + 11, "%i", &blocknumber);
          blocknumber--;
          if (blocknumber < 0) blocknumber = 0;
          if (blocknumber > nmeshes - 1) blocknumber = nmeshes - 1;
        }
      }
      meshi = meshinfo + blocknumber;
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f", &tempval, &time_local);
      tempval--;
      if (isvent == 1) {
        ventdata *vi;

        if (meshi->ventinfo == NULL || tempval < 0 || tempval >= meshi->nvents)
          continue;
        vi = GetCloseVent(meshi, tempval);
        if (vi->showtime == NULL) {
          NewMemory((void **)&vi->showtime,
                    (vi->nshowtime + 1) * sizeof(float));
          NewMemory((void **)&vi->showhide,
                    (vi->nshowtime + 1) * sizeof(unsigned char));
          vi->nshowtime = 1;
          vi->showtime[0] = 0.0;
          vi->showhide[0] = 1;
        }
        if (showvent == 1) {
          vi->showhide[vi->nshowtime] = 1;
        } else {
          vi->showhide[vi->nshowtime] = 0;
        }
        vi->showtime[vi->nshowtime++] = time_local;
      } else {
        cventdata *cvi;

        if (meshi->cventinfo == NULL || tempval < 0 ||
            tempval >= meshi->ncvents)
          continue;
        cvi = meshi->cventinfo + tempval;
        if (cvi->showtime == NULL) {
          NewMemory((void **)&cvi->showtime,
                    (cvi->nshowtime + 1) * sizeof(float));
          NewMemory((void **)&cvi->showhide,
                    (cvi->nshowtime + 1) * sizeof(unsigned char));
          cvi->nshowtime = 1;
          cvi->showtime[0] = 0.0;
          cvi->showhide[0] = 1;
        }
        if (showvent == 1) {
          cvi->showhide[cvi->nshowtime] = 1;
        } else {
          cvi->showhide[cvi->nshowtime] = 0;
        }
        cvi->showtime[cvi->nshowtime++] = time_local;
      }
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ SHOW_OBST ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "SHOW_OBST") == 1 ||
        MatchSMV(buffer, "HIDE_OBST") == 1) {
      meshdata *meshi;
      int blocknumber, tempval, showobst, blocktemp;
      blockagedata *bc;

      if (nmeshes > 1) {
        blocknumber = ioffset - 1;
      } else {
        blocknumber = 0;
      }
      if (strlen(buffer) > 10) {
        sscanf(buffer, "%s %i", buffer2, &blocktemp);
        if (blocktemp > 0 && blocktemp <= nmeshes) blocknumber = blocktemp - 1;
      }
      showobst = 0;
      if (MatchSMV(buffer, "SHOW_OBST") == 1) showobst = 1;
      meshi = meshinfo + blocknumber;
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f", &tempval, &time_local);
      tempval--;
      if (tempval < 0 || tempval >= meshi->nbptrs) continue;
      bc = meshi->blockageinfoptrs[tempval];

      if (bc->showtime == NULL) {
        if (time_local != 0.0) bc->nshowtime++;
        NewMemory((void **)&bc->showtime, bc->nshowtime * sizeof(float));
        NewMemory((void **)&bc->showhide,
                  bc->nshowtime * sizeof(unsigned char));
        bc->nshowtime = 0;
        if (time_local != 0.0) {
          bc->nshowtime = 1;
          bc->showtime[0] = 0.0;
          if (showobst == 1) {
            bc->showhide[0] = 0;
          } else {
            bc->showhide[0] = 1;
          }
        }
      }
      bc->nshowtime++;
      if (showobst == 1) {
        bc->showhide[bc->nshowtime - 1] = 1;
      } else {
        bc->showhide[bc->nshowtime - 1] = 0;
      }
      bc->showtime[bc->nshowtime - 1] = time_local;
      continue;
    }
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ DEVICE_ACT ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "DEVICE_ACT") == 1) {
      devicedata *devicei;
      int idevice;
      float act_time;
      int act_state = 1;

      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &idevice, &act_time, &act_state);
      idevice--;
      if (idevice >= 0 && idevice < ndeviceinfo) {
        int istate;

        devicei = deviceinfo + idevice;
        devicei->act_time = act_time;
        if (devicei->act_times == NULL) {
          devicei->nstate_changes++;
          NewMemory((void **)&devicei->act_times,
                    devicei->nstate_changes * sizeof(int));
          NewMemory((void **)&devicei->state_values,
                    devicei->nstate_changes * sizeof(int));
          devicei->act_times[0] = 0.0;
          devicei->state_values[0] = devicei->state0;
          devicei->istate_changes++;
        }
        istate = devicei->istate_changes++;
        devicei->act_times[istate] = act_time;
        devicei->state_values[istate] = act_state;
      }
      continue;
    }
  }

  // ------------------------------- pass 2 dynamic - end
  // ------------------------------------

  REWIND(stream);

  // ------------------------------- pass 3 dynamic - start
  // ------------------------------------

  while (do_pass3 == 1) {
    char buffer[255];

    if (FGETS(buffer, 255, stream) == NULL) break;
    if (strncmp(buffer, " ", 1) == 0 || buffer[0] == 0) continue;
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ MINMAXPL3D +++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if (MatchSMV(buffer, "MINMAXPL3D") == 1) {
      char *file_ptr, file2[1024];
      float valmin[5], valmax[5];
      float percentile_min[5], percentile_max[5];

      FGETS(buffer, 255, stream);
      strcpy(file2, buffer);
      file_ptr = file2;
      TrimBack(file2);
      file_ptr = TrimFront(file2);

      for (i = 0; i < 5; i++) {
        FGETS(buffer, 255, stream);
        sscanf(buffer, "%f %f %f %f", valmin + i, valmax + i,
               percentile_min + i, percentile_max + i);
      }

      for (i = 0; i < nplot3dinfo; i++) {
        plot3ddata *plot3di;

        plot3di = plot3dinfo + i;
        if (strcmp(file_ptr, plot3di->file) == 0) {
          int j;

          for (j = 0; j < 5; j++) {
            plot3di->diff_valmin[j] = percentile_min[j];
            plot3di->diff_valmax[j] = percentile_max[j];
          }
          break;
        }
      }
      continue;
    }
  }
  FCLOSE(stream);
  UpdatePlot3dMenuLabels();
  InitPlot3dTimeList();
  UpdateTimes();
  GetGlobalPlot3DBounds();
}

/* ------------------ ReadSMVOrig ------------------------ */

void ReadSMVOrig(void) {
  FILE *stream = NULL;

  stream = fopen(smv_orig_filename, "r");
  if (stream == NULL) return;
  PRINTF("reading  %s\n", smv_orig_filename);

  for (;;) {
    char buffer[255];

    if (fgets(buffer, 255, stream) == NULL) break;
    /*
    OBST format:
    nblockages
    xmin xmax ymin ymax zmin zmax bid s_i1 s_i2 s_j1 s_j2 s_k1 s_k2 t_x0 t_y0
    t_z0
    ...
    ...

    bid             - blockage id
    s_i1, ..., s_k2 - surf indices for i1, ..., k2 blockage faces
    t_x0,t_y0,t_z0  - texture origin

    i1 i2 j1 j2 k1 k2 colorindex blocktype r g b : ignore rgb if blocktype != -3
    ...
    ...

    int colorindex, blocktype;
    colorindex: -1 default color
                -2 invisible
                -3 use r g b color
                >=0 color/color2/texture index
    blocktype: 0 regular block
               2 outline
               3 smoothed block
               -1 (use type from surf)
               (note: if blocktype&8 == 8 then this is a "terrain" blockage
                     if so then subtract 8 and set bc->is_wuiblock=1)
    r g b           colors used if colorindex==-3
    */
    if (Match(buffer, "OBST") == 1) {
      float *xyz;
      int i;

      FREEMEMORY(obstinfo);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nobstinfo);
      NewMemory((void **)&obstinfo, nobstinfo * sizeof(xbdata));
      for (i = 0; i < nobstinfo; i++) {
        xbdata *obi;
        int blockid, *surf_index;

        obi = obstinfo + i;
        xyz = obi->xyz;
        surf_index = obi->surf_index;
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f %f %f %f %f %i %i %i %i %i %i %i", xyz, xyz + 1,
               xyz + 2, xyz + 3, xyz + 4, xyz + 5, &blockid, surf_index,
               surf_index + 1, surf_index + 2, surf_index + 3, surf_index + 4,
               surf_index + 5);
      }
      for (i = 0; i < nobstinfo; i++) {
        xbdata *obi;
        int dummy[6];
        float s_color[4];
        int colorindex, blocktype;

        obi = obstinfo + i;
        obi->transparent = 0;
        obi->invisible = 0;
        obi->usecolorindex = 0;
        obi->color = NULL;

        fgets(buffer, 255, stream);

        s_color[0] = -1.0;
        s_color[1] = -1.0;
        s_color[2] = -1.0;
        s_color[3] = 1.0;
        sscanf(buffer, "%i %i %i %i %i %i %i %i %f %f %f %f", dummy, dummy + 1,
               dummy + 2, dummy + 3, dummy + 4, dummy + 5, &colorindex,
               &blocktype, s_color, s_color + 1, s_color + 2, s_color + 3);

        if ((blocktype & 3) == 3)
          blocktype -= 3; // convert any smooth blocks to 'normal' blocks
        if (blocktype > 0 && (blocktype & 8) == 8) blocktype -= 8;
        if (s_color[3] < 0.999) obi->transparent = 1;

        /* custom color */

        if (colorindex == 0 || colorindex == 7) colorindex = -3;
        if (colorindex == COLOR_INVISIBLE) {
          obi->blocktype = BLOCK_hidden;
          obi->invisible = 1;
        }
        if (colorindex >= 0) {
          obi->color = GetColorPtr(rgb[nrgb + colorindex]);
          obi->usecolorindex = 1;
          obi->colorindex = colorindex;
          updateindexcolors = 1;
        }
        if (colorindex == -3) {
          obi->color = GetColorPtr(s_color);
          updateindexcolors = 1;
        }
        obi->colorindex = colorindex;
        obi->blocktype = blocktype;

        int j;

        for (j = 0; j < 6; j++) {
          obi->surfs[0] = NULL;
        }
        if (surfinfo != NULL) {
          for (j = 0; j < 6; j++) {
            if (obi->surf_index[j] >= 0)
              obi->surfs[j] = surfinfo + obi->surf_index[j];
          }
        }
        obi->bc = GetBlockagePtr(obi->xyz);
      }
      break;
    }
  }
  fclose(stream);
}


/* ------------------ GetLabels ------------------------ */

void GetLabels(char *buffer, int kind, char **label1, char **label2,
               char prop_buffer[255]) {
  char *tok0, *tok1, *tok2;

  tok0 = NULL;
  tok1 = NULL;
  tok2 = NULL;
  tok0 = strtok(buffer, "%");
  if (tok0 != NULL) tok1 = strtok(NULL, "%");
  if (tok1 != NULL) tok2 = strtok(NULL, "%");
  if (tok1 != NULL) {
    TrimBack(tok1);
    tok1 = TrimFront(tok1);
    if (strlen(tok1) == 0) tok1 = NULL;
  }
  if (tok2 != NULL) {
    TrimBack(tok2);
    tok2 = TrimFront(tok2);
    if (strlen(tok2) == 0) tok2 = NULL;
  }
  if (label2 != NULL) {
    if (tok2 == NULL && kind == HUMANS) {
      strcpy(prop_buffer, "Human_props(default)");
      *label2 = prop_buffer;
    } else {
      *label2 = tok2;
    }
  }
  if (label1 != NULL) *label1 = tok1;
}

/* ------------------ GetPropID ------------------------ */

propdata *GetPropID(char *prop_id) {
  int i;

  if (propinfo == NULL || prop_id == NULL || strlen(prop_id) == 0) return NULL;
  for (i = 0; i < npropinfo; i++) {
    propdata *propi;

    propi = propinfo + i;

    if (strcmp(propi->label, prop_id) == 0) return propi;
  }
  return NULL;
}

/* ------------------ ParseDevicekeyword ------------------------ */

void ParseDevicekeyword(BFILE *stream, devicedata *devicei) {
  float xyz[3] = {0.0, 0.0, 0.0}, xyzn[3] = {0.0, 0.0, 0.0};
  float xyz1[3] = {0.0, 0.0, 0.0}, xyz2[3] = {0.0, 0.0, 0.0};
  int state0 = 0;
  int nparams = 0, nparams_textures = 0;
  char *labelptr, *prop_id;
  char prop_buffer[255];
  char buffer[255], *buffer3;
  int i;
  char *tok1, *tok2, *tok3, *tok4;
  int is_beam = 0;

  devicei->type = DEVICE_DEVICE;
  FGETS(buffer, 255, stream);
  TrimCommas(buffer);

  tok1 = strtok(buffer, "%");
  tok1 = TrimFrontBack(tok1);

  tok2 = strtok(NULL, "%");
  tok2 = TrimFrontBack(tok2);

  tok3 = strtok(NULL, "%");
  tok3 = TrimFrontBack(tok3);

  tok4 = strtok(NULL, "%");
  tok4 = TrimFrontBack(tok4);

  strcpy(devicei->quantity, "");
  if (tok2 != NULL) {
    strcpy(devicei->quantity, tok2);
  }

  if (tok4 == NULL) {
    strcpy(devicei->csvlabel, tok1);
  } else {
    strcpy(devicei->csvlabel, tok4);
  }
  if (strlen(tok1) >= 4 && strncmp(tok1, "null", 4) == 0) {
    strcpy(devicei->deviceID, "null");
  } else {
    strcpy(devicei->deviceID, tok1);
  }
  devicei->object = GetSmvObjectType(tok1, missing_device);
  if (devicei->object == missing_device && tok3 != NULL) {
    devicei->object = GetSmvObjectType(tok3, missing_device);
  }
  if (devicei->object == missing_device) have_missing_objects = 1;
  devicei->params = NULL;
  devicei->times = NULL;
  devicei->vals = NULL;
  devicei->vals_orig = NULL;
  devicei->update_avg = 0;
  devicei->target_index = -1;
  devicei->global_valmin = 1.0;
  devicei->global_valmax = 0.0;
  FGETS(buffer, 255, stream);
  TrimCommas(buffer);

  sscanf(buffer, "%f %f %f %f %f %f %i %i %i", xyz, xyz + 1, xyz + 2, xyzn,
         xyzn + 1, xyzn + 2, &state0, &nparams, &nparams_textures);

  labelptr = strchr(buffer, '#'); // read in coordinates of beam detector
  if (labelptr != NULL) {
    sscanf(labelptr + 1, "%f %f %f %f %f %f", xyz1, xyz1 + 1, xyz1 + 2, xyz2,
           xyz2 + 1, xyz2 + 2);
    if (strcmp(devicei->quantity, "PATH OBSCURATION") == 0 ||
        strcmp(devicei->quantity, "TRANSMISSION") == 0) {
      is_beam = 1;
    }
  }
  devicei->is_beam = is_beam;

  GetLabels(buffer, -1, &prop_id, NULL, prop_buffer);
  devicei->prop = GetPropID(prop_id);
  if (prop_id != NULL && devicei->prop != NULL &&
      devicei->prop->smv_object != NULL) {
    devicei->object = devicei->prop->smv_object;
  } else {
    NewMemory((void **)&devicei->prop, sizeof(propdata));
    InitProp(devicei->prop, 1, devicei->deviceID);
    devicei->prop->smv_object = devicei->object;
    devicei->prop->smv_objects[0] = devicei->prop->smv_object;
  }
  if (nparams_textures < 0) nparams_textures = 0;
  if (nparams_textures > 1) nparams_textures = 1;
  devicei->ntextures = nparams_textures;
  if (nparams_textures > 0) {
    NewMemory((void **)&devicei->textureinfo, sizeof(texturedata));
  } else {
    devicei->textureinfo = NULL;
    devicei->texturefile = NULL;
  }

  labelptr = strchr(buffer, '%');
  if (labelptr != NULL) {
    TrimBack(labelptr);
    if (strlen(labelptr) > 1) {
      labelptr++;
      labelptr = TrimFront(labelptr);
      if (strlen(labelptr) == 0) labelptr = NULL;
    } else {
      labelptr = NULL;
    }
  }

  if (nparams <= 0) {
    InitDevice(devicei, xyz, is_beam, xyz1, xyz2, xyzn, state0, 0, NULL,
               labelptr);
  } else {
    float *params, *pc;
    int nsize;

    nsize = 6 * ((nparams - 1) / 6 + 1);
    NewMemory((void **)&params, (nsize + devicei->ntextures) * sizeof(float));
    pc = params;
    for (i = 0; i < nsize / 6; i++) {
      FGETS(buffer, 255, stream);
      TrimCommas(buffer);

      sscanf(buffer, "%f %f %f %f %f %f", pc, pc + 1, pc + 2, pc + 3, pc + 4,
             pc + 5);
      pc += 6;
    }
    InitDevice(devicei, xyz, is_beam, xyz1, xyz2, xyzn, state0, nparams, params,
               labelptr);
  }
  GetElevAz(devicei->xyznorm, &devicei->dtheta, devicei->rotate_axis, NULL);
  if (nparams_textures > 0) {
    FGETS(buffer, 255, stream);
    TrimCommas(buffer);
    TrimBack(buffer);
    buffer3 = TrimFront(buffer);
    NewMemory((void **)&devicei->texturefile, strlen(buffer3) + 1);
    strcpy(devicei->texturefile, buffer3);
  }
}

/* ------------------ ParseDevicekeyword2 ------------------------ */

void ParseDevicekeyword2(FILE *stream, devicedata *devicei) {
  float xyz[3] = {0.0, 0.0, 0.0}, xyzn[3] = {0.0, 0.0, 0.0};
  float xyz1[3] = {0.0, 0.0, 0.0}, xyz2[3] = {0.0, 0.0, 0.0};
  int state0 = 0;
  int nparams = 0, nparams_textures = 0;
  char *labelptr, *prop_id;
  char prop_buffer[255];
  char buffer[255], *buffer3;
  int i;
  char *tok1, *tok2, *tok3;
  int is_beam = 0;

  devicei->type = DEVICE_DEVICE;
  fgets(buffer, 255, stream);
  TrimCommas(buffer);

  tok1 = strtok(buffer, "%");
  tok1 = TrimFrontBack(tok1);

  tok2 = strtok(NULL, "%");
  tok2 = TrimFrontBack(tok2);

  tok3 = strtok(NULL, "%");
  tok3 = TrimFrontBack(tok3);

  strcpy(devicei->quantity, "");
  if (tok2 != NULL) {
    strcpy(devicei->quantity, tok2);
  }

  if (strlen(tok1) >= 4 && strncmp(tok1, "null", 4) == 0) {
    strcpy(devicei->deviceID, "null");
  } else {
    strcpy(devicei->deviceID, tok1);
  }
  devicei->object = GetSmvObjectType(tok1, missing_device);
  if (devicei->object == missing_device && tok3 != NULL) {
    devicei->object = GetSmvObjectType(tok3, missing_device);
  }
  if (devicei->object == missing_device) have_missing_objects = 1;
  devicei->params = NULL;
  devicei->times = NULL;
  devicei->vals = NULL;
  devicei->vals_orig = NULL;
  devicei->update_avg = 0;
  fgets(buffer, 255, stream);
  TrimCommas(buffer);

  sscanf(buffer, "%f %f %f %f %f %f %i %i %i", xyz, xyz + 1, xyz + 2, xyzn,
         xyzn + 1, xyzn + 2, &state0, &nparams, &nparams_textures);

  labelptr =
      strchr(buffer, '#'); // read in coordinates of a possible beam detector
  if (labelptr != NULL) {
    sscanf(labelptr + 1, "%f %f %f %f %f %f", xyz1, xyz1 + 1, xyz1 + 2, xyz2,
           xyz2 + 1, xyz2 + 2);
    if (strcmp(devicei->quantity, "PATH OBSCURATION") == 0 ||
        strcmp(devicei->quantity, "TRANSMISSION") == 0) {
      is_beam = 1;
    }
  }
  devicei->is_beam = is_beam;

  GetLabels(buffer, -1, &prop_id, NULL, prop_buffer);
  devicei->prop = GetPropID(prop_id);
  if (prop_id != NULL && devicei->prop != NULL &&
      devicei->prop->smv_object != NULL) {
    devicei->object = devicei->prop->smv_object;
  } else {
    NewMemory((void **)&devicei->prop, sizeof(propdata));
    InitProp(devicei->prop, 1, devicei->deviceID);
    devicei->prop->smv_object = devicei->object;
    devicei->prop->smv_objects[0] = devicei->prop->smv_object;
  }
  if (nparams_textures < 0) nparams_textures = 0;
  if (nparams_textures > 1) nparams_textures = 1;
  devicei->ntextures = nparams_textures;
  if (nparams_textures > 0) {
    NewMemory((void **)&devicei->textureinfo, sizeof(texturedata));
  } else {
    devicei->textureinfo = NULL;
    devicei->texturefile = NULL;
  }

  labelptr = strchr(buffer, '%');
  if (labelptr != NULL) {
    TrimBack(labelptr);
    if (strlen(labelptr) > 1) {
      labelptr++;
      labelptr = TrimFront(labelptr);
      if (strlen(labelptr) == 0) labelptr = NULL;
    } else {
      labelptr = NULL;
    }
  }

  if (nparams <= 0) {
    InitDevice(devicei, xyz, is_beam, xyz1, xyz2, xyzn, state0, 0, NULL,
               labelptr);
  } else {
    float *params, *pc;
    int nsize;

    nsize = 6 * ((nparams - 1) / 6 + 1);
    NewMemory((void **)&params, (nsize + devicei->ntextures) * sizeof(float));
    pc = params;
    for (i = 0; i < nsize / 6; i++) {
      fgets(buffer, 255, stream);
      TrimCommas(buffer);

      sscanf(buffer, "%f %f %f %f %f %f", pc, pc + 1, pc + 2, pc + 3, pc + 4,
             pc + 5);
      pc += 6;
    }
    InitDevice(devicei, xyz, is_beam, xyz1, xyz2, xyzn, state0, nparams, params,
               labelptr);
  }
  GetElevAz(devicei->xyznorm, &devicei->dtheta, devicei->rotate_axis, NULL);
  if (nparams_textures > 0) {
    fgets(buffer, 255, stream);
    TrimCommas(buffer);
    TrimBack(buffer);
    buffer3 = TrimFront(buffer);
    NewMemory((void **)&devicei->texturefile, strlen(buffer3) + 1);
    strcpy(devicei->texturefile, buffer3);
  }
}

bufferstreamdata *CopySMVBuffer(bufferstreamdata *stream_in);

/* ------------------ GetInpf ------------------------ */

int GetInpf(bufferstreamdata *stream_in) {
  char buffer[255], *bufferptr;
  bufferstreamdata *stream;
  int len;

  if (stream_in == NULL) return 1;
  stream = CopySMVBuffer(stream_in);
  if (stream == NULL) return 1;
  for (;;) {
    if (FEOF(stream) != 0) {
      BREAK;
    }
    if (FGETS(buffer, 255, stream) == NULL) {
      BREAK;
    }
    if (strncmp(buffer, " ", 1) == 0) continue;
    if (MatchSMV(buffer, "INPF") == 1) {
      if (FGETS(buffer, 255, stream) == NULL) {
        BREAK;
      }
      bufferptr = TrimFrontBack(buffer);

      len = strlen(bufferptr);
      FREEMEMORY(fds_filein);
      if (NewMemory((void **)&fds_filein, (unsigned int)(len + 1)) == 0)
        return 2;
      STRCPY(fds_filein, bufferptr);
      if (FILE_EXISTS_CASEDIR(fds_filein) == NO) {
        FreeMemory(fds_filein);
      }

      if (chidfilebase == NULL) {
        char *chidptr = NULL;
        char buffer_chid[1024];

        if (fds_filein != NULL) chidptr = GetChid(fds_filein, buffer_chid);
        if (chidptr != NULL) {
          NewMemory((void **)&chidfilebase,
                    (unsigned int)(strlen(chidptr) + 1));
          STRCPY(chidfilebase, chidptr);
        }
      }
      if (chidfilebase != NULL) {
        NewMemory((void **)&hrr_csv_filename,
                  (unsigned int)(strlen(chidfilebase) + 8 + 1));
        STRCPY(hrr_csv_filename, chidfilebase);
        STRCAT(hrr_csv_filename, "_hrr.csv");
        if (FILE_EXISTS_CASEDIR(hrr_csv_filename) == NO) {
          FREEMEMORY(hrr_csv_filename);
        }

        NewMemory((void **)&devc_csv_filename,
                  (unsigned int)(strlen(chidfilebase) + 9 + 1));
        STRCPY(devc_csv_filename, chidfilebase);
        STRCAT(devc_csv_filename, "_devc.csv");
        if (FILE_EXISTS_CASEDIR(devc_csv_filename) == NO) {
          FREEMEMORY(devc_csv_filename);
        }

        NewMemory((void **)&exp_csv_filename,
                  (unsigned int)(strlen(chidfilebase) + 8 + 1));
        STRCPY(exp_csv_filename, chidfilebase);
        STRCAT(exp_csv_filename, "_exp.csv");
        if (FILE_EXISTS_CASEDIR(exp_csv_filename) == NO) {
          FREEMEMORY(exp_csv_filename);
        }
      }
      break;
    }
  }
  return 0;
}
