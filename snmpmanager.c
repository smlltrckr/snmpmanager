#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_IPV4 15

int main(int argc, char ** argv){
	return 0;
}

/*********************Find Devices*******************
This function finds a device and returns it to a list
	PRE:
	POST:
*/
void findDevice(){
	
	return 0;
}

/********************Find Neighbor*******************
This function finds the neighbor for each device
	PRE: device identifier (String?)
	POST: Array of Devices returned
*/
char **deviceNeighbors(char *device){
	char **neighbors;
	char *neighbors = (char *)malloc(MAX_IPV4 * sizeof(char**))
	//char **neighbors = ( Need to allocate

	return neighbors;
}
