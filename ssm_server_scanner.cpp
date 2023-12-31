#include <string.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <netinet/in.h>	//sockaddr_in, sockaddr_in6, INET_ADDRSTRLEN, INET6_ADDRSTRLEN
#include <arpa/inet.h>	//inet_ntop()
#include <unistd.h>	//sleep()

#include "ssm_server_scanner.h"
#include "json.h"
#include "os_settings.h"


/**/


#define GET_SERVER_HISTORY_PATH(filename) sprintf(filename, "%s%s", LOCALSTATEDIR, "/snapshowmagic/server_history")


/**/


// Declarations for private functions
void scan_network_for_ssm_server(const char *client_ip_address, char *server_ip_address);
void scan_local_network_for_ssm_server(char *server_ip_address);
void scan_for_ssm_server_from_data_file(char *server_ip_address);
void record_server_address(const char *server_ip_address);


/**/


void scan_for_ssm_server(char *server_address)
{
	scan_for_ssm_server_from_data_file(server_address);
	if(strcmp(server_address, "") == 0)
		scan_local_network_for_ssm_server(server_address);
}


void scan_for_ssm_server_from_data_file(char *server_ip_address)
{
	char addresses[10][256];
	char temp[256];
	char filename[512];

	FILE *file;

	int counter;

	bool server_found;

	/**/

	GET_SERVER_HISTORY_PATH(filename);

	file = fopen(filename, "r");
	if(file != NULL)
	{
		bzero((char *) &addresses, sizeof(addresses)); // wipe it clean!

		// Get in, read the data, and get out!
		for(counter = 0; counter < 10; counter++)
		{
			if(fgets(temp, 256, file) != NULL)
			{
				sscanf(temp, "%s", addresses[counter]);
			}
		}
		fclose(file);

		// Now let's check the addresses.
		// Breaking this into a separate loop because verify_ssm_server_address() writes to the file we just read from.  Trying to avoid a "file is locked" error from the file system.
		server_found = false;
		for(counter = 0; ((counter < 10) && (!server_found)); counter++)
		{
			if(strcmp(addresses[counter], "") != 0)	// Skip empty values
			{
				server_found = verify_ssm_server_address(addresses[counter]);
				if(server_found)
				{
					strncpy(server_ip_address, addresses[counter], 256);
				}
			}
		}
	}
}


bool verify_ssm_server_address(const char *server_address)
{
	// Check if server address really is the SSM server
	// and record the address in a local file for future use

	char json_response_text[512];
	char message_type[512];

	int errno;

	/**/

	fprintf(stdout, "\tScanning IP Address: %s...\n", server_address);
	errno = json_get(server_address, "/controls/state.json", json_response_text);
	if(errno >= 0)
	{
		fprintf(stdout, "\t\tHit -- Checking response...\n");
		//{"message_type":"control","play_state":"play"}
		json_parse_string_from_json(json_response_text, "message_type", message_type);
		if(strcmp(message_type, "control") == 0)
		{
			fprintf(stdout, "\t\tResponse from %s is valid!\n", server_address);
			record_server_address(server_address);
			return true;
		}
	}
	return false;
}


void record_server_address(const char *server_ip_address)
{
	// Save the server address in a local file
	char addresses[10][256];
	char temp[256];
	char filename[512];

	FILE *file;

	int counter;

	/**/

	GET_SERVER_HISTORY_PATH(filename);

	bzero((char *) &addresses, sizeof(addresses)); // wipe it clean!

	// Add current address to top of the list
	strncpy(addresses[0], server_ip_address, 256);

	// Load the rest of the addresses from the file
	file = fopen(filename, "r");
	if(file != NULL)
	{
		for(counter = 1; counter < 10; counter++)
		{
			if(fgets(temp, 256, file) != NULL)
			{
				if(sscanf(temp, "%s", addresses[counter]) > 0)
				{
					// prevent duplicate entries in data file
					if(strcmp(addresses[counter], server_ip_address) == 0)
					{
						bzero((char *) &addresses[counter], sizeof(addresses[counter])); // wipe it clean!

						// shouldn't ever adjust a for-loop variable this way!
						counter--;
					}
				}
				else
				{
					// Skip blank lines in data file
					// shouldn't ever adjust a for-loop variable this way!
					counter--;
				}
			}
		}
		fclose(file);
	}

	// Write updated list to the file
	file = fopen(filename, "w");
	if(file != NULL)
	{
		for(counter = 0; counter < 10; counter++)
		{
			//	writeln(addresses[counter]);
			if(strcmp(addresses[counter], "") != 0)	// Skip blank entries
				fprintf(file, "%s\n", addresses[counter]);
		}
		fclose(file);
	}
}


void scan_local_network_for_ssm_server(char *server_ip_address)
{
	char machine_ip_address[256];

	/**/

	bzero((char *) &machine_ip_address, sizeof(machine_ip_address)); // wipe it clean!

	get_local_ip_address(machine_ip_address);
	scan_network_for_ssm_server(machine_ip_address, server_ip_address);	
}


void get_local_ip_address(char *ip_address)
{
	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;

	void * tmpAddrPtr=NULL;

	/**/

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		// Skip rest of loop if no valid IP address
		if(!ifa->ifa_addr) continue;

		// Skip rest of loop if "lo" adapter name
		if(strcmp(ifa->ifa_name, "lo") == 0) continue;

		// Currently only care about IP4 addresses
		if (ifa->ifa_addr->sa_family == AF_INET)	// check it is IP4
		{
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, tmpAddrPtr, ip_address, INET_ADDRSTRLEN);
		}
	}

	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
}


void scan_network_for_ssm_server(const char *client_ip_address, char *server_ip_address)
{
	char local_ip_address[256];
	char scan_ip_address[256];
	char *ip[4];

	int counter;
	int ip_value;

	bool server_found;

	/**/

	// Parse incoming ip_address
	// Make a copy of the incoming ip_address so strtok doesn't mess with it
	strcpy(local_ip_address, client_ip_address);
	ip[0] = strtok(local_ip_address, ".");
	ip[1] = strtok(NULL, ".");
	ip[2] = strtok(NULL, ".");
	ip[3] = strtok(NULL, ".");
	
	// optional: guess starting point based on ddd value (eg 104 implies addresses start at 100 instead of 1)
	ip_value = atoi(ip[3]);
	ip_value = ((ip_value / 25) * 25);
	//ip_value++;
	if(ip_value == 0) ip_value++;
	
	// Loop through network IP addresses aaa.bbb.ccc.1-254 until we find valid server response
	server_found = false;
	for(counter = ip_value; counter < 255; counter++)
	{
		snprintf(ip[3], sizeof(ip[3]), "%d", counter);
		snprintf(scan_ip_address, sizeof(scan_ip_address), "%s.%s.%s.%s", ip[0], ip[1], ip[2], ip[3]);
		server_found = verify_ssm_server_address(scan_ip_address);
		if(server_found)
		{
			sprintf(server_ip_address, "%s", scan_ip_address);
			break;
		}
	}

	if(!server_found)
	{
		// Check the rest of the local network
		// (This loop only runs if machine_ip_address > aaa.bbb.ccc.ddd.25)
		for(counter = 1; counter < ip_value; counter++)
		{
			snprintf(ip[3], sizeof(ip[3]), "%d", counter);
			snprintf(scan_ip_address, sizeof(scan_ip_address), "%s.%s.%s.%s", ip[0], ip[1], ip[2], ip[3]);
			server_found = verify_ssm_server_address(scan_ip_address);
			if(server_found)
			{
				sprintf(server_ip_address, "%s", scan_ip_address);
				break;		
			}
		}
	}
}


void display_ip_addresses()
{
	// Code from: http://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine

	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;

	/**/

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if(!ifa->ifa_addr) {
			continue;
		}

		if (ifa->ifa_addr->sa_family == AF_INET)	// check it is IP4
		{
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
		}
		else if (ifa->ifa_addr->sa_family == AF_INET6) // check it is IP6
		{
			// is a valid IP6 Address
			tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
		} 
	}

	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
}