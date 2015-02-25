#ifndef INC_SSM_CLIENT_H
#define INC_SSM_CLIENT_H


/**/


#include <time.h>	// timespec structure

#include "opengl_es.h"	// for the EGL_TYPE declaration
#include "os_settings.h"


/**/


//#define DEFAULT_IMAGE_PATH "./images/black.png"
#define GET_DEFAULT_IMAGE_PATH(filename) sprintf(filename, "%s%s", LOCALSTATEDIR, "/snapshowmagic/images/black.png")
#define GET_DOWNLOAD_PATH(filename) sprintf(filename, "%s%s", LOCALSTATEDIR, "/snapshowmagic/downloads")
#define GET_DOWNLOADED_PHOTOS_PATH(filename) sprintf(filename, "%s%s", LOCALSTATEDIR, "/snapshowmagic/downloads/photos")
#define DEFAULT_PLAY_STATE "play"
#define DEFAULT_TRANSITION_TYPE "dissolve"


/**/


bool ssm_init(char *server_address);

void get_next_photo(EGL_TYPE *egl, const char *server_name);
void get_play_state(EGL_TYPE *egl, const char *server_name);

void clock_addtimes(timespec time1, timespec time2, timespec *result);
void clock_subtracttimes(timespec time1, timespec time2, timespec *result);

int clock_cmptimes(timespec time1, timespec time2);


/**/


#endif  /* INC_SSM_CLIENT_H */
