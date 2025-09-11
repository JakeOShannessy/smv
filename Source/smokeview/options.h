#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

//#define pp_SHOW_UPDATE      // show what is being updated in the UpdateShowScene routine
//#define pp_TERRAIN_HIDE     // hide terrain when hiding internal blockages

//*** options: windows

#ifdef WIN32
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_REFRESH          // refresh glui dialogs when they change size
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#endif

//*** options: OSX

#ifdef pp_OSX
#define pp_REFRESH      // refresh glui dialogs when they change size
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_RECOMPUTE_DEBUG   // output debug message if bounds are recomputed
#define pp_BOUND_DEBUG       // output debug message in some bound routines
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif
