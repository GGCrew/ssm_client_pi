#ifndef INC_SSM_SERVER_SCANNER_H
#define INC_SSM_SERVER_SCANNER_H


/**/


//#include <time.h>	// timespec structure

//#include "opengl_es.h"	// for the EGL_TYPE declaration


/**/


//#define DEFAULT_IMAGE_PATH "./images/black.png"
//#define DEFAULT_PLAY_STATE "play"
//#define DEFAULT_TRANSITION_TYPE "dissolve"


/**/


void scan_for_ssm_server(char *server_address);
bool verify_ssm_server_address(const char *server_address);
void get_local_ip_address(char *ip_address);


/**/


#endif  /* INC_SSM_SERVER_SCANNER_H */
