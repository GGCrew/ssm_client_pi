#include <string.h>
#include <unistd.h> // access(), fork(), execl()
#include <sys/wait.h> // waitpid()

#include <stdio.h> // printf()

#include "ssm_client.h"
#include "ssm_server_scanner.h"
#include "json.h"


/**/


// Declarations for private functions
void download_file(const char *protocol_prefix, const char *server_name, const char *server_path, char *local_path);
bool delete_downloaded_files();
bool init_network_connection();


/**/


bool ssm_init(char *server_address)
{
	bool success = false;	// Assume failure

	/**/

	success = init_network_connection();
	if(!success)
		fprintf(stdout, "Unable to initialize the network connection.\n");

	if(success)
	{
		if(strcmp(server_address, "") == 0)
			scan_for_ssm_server(server_address);
	}

	if(success)
	{
		success = verify_ssm_server_address(server_address);
		if(!success)
			fprintf(stdout, "Unable to connect to server %s\n", server_address);
	}

	if(success)
	{
		success = delete_downloaded_files();
		if(!success)
			fprintf(stdout, "Unable to delete existing photos.\n");
	}
	
	return success;
}


bool init_network_connection()
{
	char local_ip_address[256];

	bool local_ip_address_assigned = false;

	int attempt = 0;
	int max_attempts = 5;

	/**/

	// Check for valid IP address (which implies a working network connection)
	bzero((char *) &local_ip_address, sizeof(local_ip_address)); // wipe it clean!

	do
	{
		attempt++;
		fprintf(stdout, "\tChecking for local IP address (attempt %u/%u)...\n", attempt, max_attempts);
		get_local_ip_address(local_ip_address);
		local_ip_address_assigned = (strcmp(local_ip_address, "") != 0);
		if(!local_ip_address_assigned && (attempt < max_attempts))	// Skip sleeping if we're on last attempt.
			sleep(5);
	}
	while(!local_ip_address_assigned && (attempt < max_attempts));

	if(local_ip_address_assigned)
	{
		fprintf(stdout, "\tLocal IP address: %s\n", local_ip_address);
	}

	return local_ip_address_assigned;
}


void get_next_photo(EGL_TYPE *egl, const char *server_name)
{
	char json_response_text[512];
	char full_path[512];
	char local_path[512];
	char transition_type[512];

	int hold_duration;
	int transition_duration;
	int color_mode;
	bool vignette;

	int textureIndex;

	/**/

	json_get(server_name, "/photos/next.json", json_response_text);
	json_parse_string_from_json(json_response_text, "full_path", full_path);
	json_parse_string_from_json(json_response_text, "transition_type", transition_type);
	hold_duration = json_parse_int_from_json(json_response_text, "hold_duration");
	transition_duration = json_parse_int_from_json(json_response_text, "transition_duration");
	color_mode = json_parse_int_from_json(json_response_text, "color_mode");
	vignette = json_parse_bool_from_json(json_response_text, "effect_vignette");

	if(full_path[0] == '\0')
		//strcpy(local_path, DEFAULT_IMAGE_PATH);  // Error handling
		GET_DEFAULT_IMAGE_PATH(local_path);
	else
		download_file("http://", server_name, full_path, local_path);

	if(transition_type[0] == '\0')
		strcpy(transition_type, DEFAULT_TRANSITION_TYPE);	// Error handling

	// Set transition_duration for current-into-next transition_duration
	if(transition_duration > 0)
		egl->transition_duration = transition_duration;

	if(hold_duration > 0)
		egl->hold_duration = hold_duration;

	textureIndex = (egl->textureCurrent + 1) % IMAGE_COUNT;

	egl_load_texture(&egl->textures[textureIndex], local_path);
//	egl->hold_durations[textureIndex] = hold_duration;

	if(color_mode > 0)
		egl->textures[textureIndex].color_mode = color_mode;
	else
		egl->textures[textureIndex].color_mode = EGL_DEFAULT_COLOR_MODE;

	if(vignette)
		egl->textures[textureIndex].vignette = vignette;
	else
		egl->textures[textureIndex].vignette = EGL_DEFAULT_VIGNETTE;
}


void get_play_state(EGL_TYPE *egl, const char *server_name)
{
	char json_response_text[512];
	char play_state[512];

	/**/

	json_get(server_name, "/controls/state.json", json_response_text);
	json_parse_string_from_json(json_response_text, "play_state", play_state);

	if(play_state[0] == '\0')
		strcpy(play_state, DEFAULT_PLAY_STATE);	// Error handling

	if(strcmp(play_state, "play") == 0) egl->play_state = PLAY;
	else if(strcmp(play_state, "pause") == 0) egl->play_state = PAUSE;
	else if(strcmp(play_state, "stop") == 0) egl->play_state = STOP;
}


void download_file(const char *protocol_prefix, const char *server_name, const char *server_path, char *local_path)
{
	char url[512];
	char photos_path[512];
	char directory_prefix[512 + 32];

	pid_t pid;
	int pid_status;

	/**/

	GET_DOWNLOAD_PATH(photos_path);

	// Convert "server_path" from an absolute address to a local path
	strcpy(local_path, photos_path);
	strcat(local_path, server_path);

	// Check if local path exists
	if(access(local_path, F_OK) != -1)
	{
		// local copy exists!  Nothing else to do.
	}
	else
	{
		// No local copy!  Let's get one.
		strcpy(url, protocol_prefix);
		strcat(url, server_name);
		strcat(url, server_path);

		// http://www.gnu.org/software/libc/manual/html_node/Process-Creation-Example.html
		pid = fork();
		if(pid == 0)
		{
			// We're in the child process!
			// Execute the command...
			strcpy(directory_prefix, "--directory-prefix=");
			strcat(directory_prefix, photos_path);

			execl("/usr/bin/wget", "/usr/bin/wget", "--no-host-directories", "--force-directories", "--no-verbose", directory_prefix, url, NULL);

			// The following line will only run if the executed command fails.
			_exit(EXIT_FAILURE);
		}
		else if(pid < 0)
		{
			// The fork() failed.
		}
		else
		{
			// We're still in the parent process
			// Stall until the child process finishes...
			if(waitpid(pid, &pid_status, 0) != pid)
			{
				// Something went wrong with the child process
			}
		}
	}
}


bool delete_downloaded_files()
{
	pid_t pid;
	int pid_status;

	char photos_path[512];

	/**/

	pid = fork();
	if(pid == 0)
	{
		// We're in the child process!
		// Execute the command...
		GET_DOWNLOADED_PHOTOS_PATH(photos_path);

		fprintf(stdout, "\tDeleting existing photos from %s...\n", photos_path);
		execl("/bin/rm", "/bin/rm", "--recursive", "--force", photos_path, NULL);

		// The following line will only run if the executed command fails.
		_exit(EXIT_FAILURE);
	}
	else if(pid < 0)
	{
		// The fork() failed.
		fprintf(stderr,	"\tdelete_downloaded_files() -- fork() failure!!!\n");
	}
	else
	{
		// We're still in the parent process
		// Stall until the child process finishes...
		if(waitpid(pid, &pid_status, 0) != pid)
		{
			// Something went wrong with the child process
		}
		fprintf(stdout, "\tDeleting existing photos - DONE!\n");
	}

	return true;
}


void clock_addtimes(timespec time1, timespec time2, timespec *result)
{
	result->tv_sec = time1.tv_sec + time2.tv_sec;
	result->tv_nsec = time1.tv_nsec + time2.tv_nsec;
	
	while(result->tv_nsec >= 1000000000)
	{
		result->tv_sec += 1;
		result->tv_nsec -= 1000000000;
	}
}


void clock_subtracttimes(timespec time1, timespec time2, timespec *result)
{
	result->tv_sec = time1.tv_sec - time2.tv_sec;
	result->tv_nsec = time1.tv_nsec - time2.tv_nsec;
	
	while(result->tv_nsec < 0)
	{
		result->tv_sec -= 1;
		result->tv_nsec += 1000000000;
	}
}


int clock_cmptimes(timespec time1, timespec time2)
{
	//return an integer less than, equal to, or greater than zero if time1 is found, respectively, to be less than, to match, or be greater than s2. 
	int return_value;

	/**/

	if(time1.tv_sec < time2.tv_sec)
		return_value = -1;
	else if(time1.tv_sec > time2.tv_sec)
		return_value = 1;
	else
		if(time1.tv_nsec < time2.tv_nsec)
			return_value = -1;
		else if(time1.tv_nsec > time2.tv_nsec)
			return_value = 1;
		else
			return_value = 0;

	return return_value;
}