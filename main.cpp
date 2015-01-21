#include <stdio.h>
#include <ctype.h>	// isprint()
#include <time.h>	// nanosleep()

#include "opengl_es.h"
#include "ssm_client.h"


/**/


int main (int argc, char *argv[])
{
	EGL_TYPE _egl, *egl=&_egl;
	bool terminate;
	
	char *server_name = NULL;

	struct timespec current_time;
	struct timespec time_remaining;
	struct timespec hold_end_time;
	struct timespec sleep_end_time;


	/**/


	// Parse server name (or IP address) from command line
	int option_character;
	while ((option_character = getopt (argc, argv, "s:")) != -1)
	{
		switch(option_character)
		{
			case 's':
				server_name = optarg;
				break;

			case '?':
				switch(optopt)
				{
					case 's':
						fprintf(stderr, "Option -%c requires an argument.\n", optopt);
						break;

					default:
						if(isprint(optopt))
							fprintf(stderr, "Unknown option `-%c'.\n", optopt);
						else
							fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
						break;
				}
				return 1;

			default:
				abort();
		}
	}


	// Check that required parameters are set
	if(!server_name)
	{
		fprintf(stderr, "Missing option `-s', which specifies the server name or IP address.\n");
		return 1;
	}


	/**/


	ssm_init();


	// Start OGLES
	egl_init(egl);
	egl_init_shaders(egl);
	egl_init_model(egl);
	egl_init_matrices(egl);


	// Load starting textures
	egl->textureCurrent = 0;
	egl_load_texture(&egl->textures[egl->textureCurrent], DEFAULT_IMAGE_PATH);

	get_next_photo(egl, server_name);


	// Let's get crackin!
	terminate = false;
	egl->frameCounter = 0;
	egl->textureCurrent = 0;  // start with that black image we loaded a few lines ago
	egl->program_state = TRANSITION;

	// Go!
	while (!terminate)
	{
		switch(egl->program_state)
		{
			case NEED_PHOTO:
				clock_gettime(CLOCK_REALTIME, &current_time);

				get_play_state(egl, server_name);
				get_next_photo(egl, server_name);

				// hold_duration is set in get_next_photo()
				time_remaining.tv_sec = egl->hold_duration / 1000;
				time_remaining.tv_nsec = (egl->hold_duration % 1000) * 1000000;

				clock_addtimes(current_time, time_remaining, &hold_end_time);				

				egl->program_state = HOLD;
				break;

			case HOLD:
				// Assume we will sleep for 1 second (which is correct most of the time)
				clock_gettime(CLOCK_REALTIME, &current_time);
				clock_addtimes(current_time, (struct timespec){1, 0}, &sleep_end_time);
				
				switch(egl->play_state)
				{
					case PLAY:
						clock_subtracttimes(hold_end_time, current_time, &time_remaining);
						
						if(time_remaining.tv_sec > 0)
						{
							// More than 1 second remains
							// Check play_state from server	
							get_play_state(egl, server_name);
						}
						else
						{
							// Less than 1 second remains
							// Set sleep_end_time based on time remaining and prep for transition
							clock_addtimes(current_time, time_remaining, &sleep_end_time);

							egl->frameCounter = 0;
							egl->program_state = TRANSITION;
						}
						break;
						
					case PAUSE:
						// Ignore hold_end_time and continue displaying current image
						get_play_state(egl, server_name);
						break;
						
					case STOP:
						// Ignore hold_end_time and blank the screen
						egl_load_texture(&egl->textures[egl->textureCurrent], "./images/black.png");
						egl_render(egl);
						get_play_state(egl, server_name);
						break;
				}

				// Sleep unless sleep_end_time has passed
				clock_gettime(CLOCK_REALTIME, &current_time);
				if(clock_cmptimes(current_time, sleep_end_time) < 0)
				{
					// Sleep for whatever time is left in the sleep period
					clock_subtracttimes(sleep_end_time, current_time, &time_remaining);
					nanosleep(&time_remaining, NULL);
				}

				break;

			case TRANSITION:
				egl_render(egl);

				egl->frameCounter++;
				// egl->program_state is updated to TRANSITION_COMPLETE within egl_render() for transitions
				break;

			case TRANSITION_COMPLETE:
				egl->textureCurrent++;
				egl->textureCurrent = (egl->textureCurrent % IMAGE_COUNT);  // specify the active texture
				egl->program_state = NEED_PHOTO;
				break;
		}
	}

	egl_destroy(egl);

	return 0;
}

