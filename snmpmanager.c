#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_IPV4 15

// Global Variables
netsnmp_session session, *ss;
netsnmp_pdu *response;

// Function Declarations
void findDevice();
char **deviceNeighbors(char *device);
void traffic(int timeInterval, int numberOfSamples);
void printTraffic(int interfaces, long currTraffic[], long prevTraffic[]);
int getTableData(char *objectName);

int main(int argc, char ** argv){
	int timeInterval, numberOfSamples;
	char *agentIP, *community;
	// netsnmp_pdu *pdu; // Use locally in separate functions

	if (argc < 4){
		printf("USAGE: timeInterval(Seconds) numberOfSamples agentIP community\n");
		// Default Values?
		timeInterval = 100;
		numberOfSamples = 4;
		agentIP = "127.0.0.1";
		community = "public";
		printf("Default: timeInterval(%d), numberOfSamples(%d), agentIP(%s), community(%s)\n", timeInterval, numberOfSamples, agentIP, community);
	} else {
		timeInterval = atoi(argv[1]);
		numberOfSamples = atoi(argv[2]);
		agentIP = argv[3];
		community = argv[4];
	}

	// oid anOID[MAX_OID_LEN]; // Here only for  testing Use locally
	// size_t anOID_len; // Here only for  testing Use locally

	// netsnmp_variable_list *vars;// Here only for  testing Use locally
	// int status;// Here only for  testing Use locally
	// int count = 1;// Here only for  testing Use locally

	/* Initializes the SNMP library */
	init_snmp("snmpmanager");

	/* Initializes a session */
	snmp_sess_init(&session);
	session.peername = strdup(agentIP);

	/* Set SNMP version number */
	session.version = SNMP_VERSION_1;

	/* Set SNMP community */
	session.community = community;
	session.community_len = strlen(session.community);

	/* Start the session */
	SOCK_STARTUP;
	ss = snmp_open(&session);

	if (!ss)
	{
		snmp_sess_perror("ack", &session);
		SOCK_CLEANUP;
		exit(1);
	}

	// pdu = snmp_pdu_create(SNMP_MSG_GET);
	// anOID_len = MAX_OID_LEN;
	// if (!snmp_parse_oid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len))
	// {
	// 	snmp_perror(".1.3.6.1.2.1.1.1.0");
	// 	SOCK_CLEANUP;
	// 	exit(1);
	// }

	// snmp_add_null_var(pdu, anOID, anOID_len);

	/* Send out a request */
	// status = snmp_synch_response(ss, pdu, &response);

	// //Start test result
	// if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
 //      /*
 //       * SUCCESS: Print the result variables
 //       */

 //      for(vars = response->variables; vars; vars = vars->next_variable)
 //        print_variable(vars->name, vars->name_length, vars);

 //      /* manipuate the information ourselves */
 //      for(vars = response->variables; vars; vars = vars->next_variable) {
 //        if (vars->type == ASN_OCTET_STR) {
	//   char *sp = (char *)malloc(1 + vars->val_len);
	//   memcpy(sp, vars->val.string, vars->val_len);
	//   sp[vars->val_len] = '\0';
 //          printf("value #%d is a string: %s\n", count++, sp);
	//   free(sp);
	// }
 //        else
 //          printf("value #%d is NOT a string! Ack!\n", count++);
 //      }
 //    } else {
 //      /*
 //       * FAILURE: print what went wrong!
 //       */

 //      if (status == STAT_SUCCESS)
 //        fprintf(stderr, "Error in packet\nReason: %s\n",
 //                snmp_errstring(response->errstat));
 //      else if (status == STAT_TIMEOUT)
 //        fprintf(stderr, "Timeout: No response from %s.\n",
 //                session.peername);
 //      else
 //        snmp_sess_perror("snmpdemoapp", ss);

 //    }//END test result

	traffic(timeInterval, numberOfSamples);

	/* Clean up and close connection */
	if (response)
	{
		snmp_free_pdu(response);
	}
	snmp_close(ss);

	SOCK_CLEANUP;

	return 0;
} // END main()

/*********************Find Devices*******************
This function finds a device and returns it to a list
	PRE:
	POST:
*/
void findDevice(){
	
	// return 0;
}

/********************Find Neighbor*******************
This function finds the neighbor for each device
	PRE: device identifier (String?)
	POST: Array of Devices returned
*/
char **deviceNeighbors(char *device){
	char **neighbors;
	// char *neighbors = (char *)malloc(MAX_IPV4 * sizeof(char**))
	//char **neighbors = ( Need to allocate

	return neighbors;
}

/*********************Traffic***********************
This function finds the rate of traffic on each interface
	PRE:
	POST:
*/
void traffic(int timeInterval, int numberOfSamples){
	int interfaces = getTableData("ifNumber.0"); // Number of interfaces
	long prevInTraffic[interfaces];
	long currInTraffic[interfaces];
	long prevOutTraffic[interfaces];
	long currOutTraffic[interfaces];
	time_t startTime, endTime;
	double timeElapsed;

	memset(currInTraffic, 0, sizeof(currInTraffic) / sizeof(long));
	memset(prevInTraffic, 0, sizeof(prevInTraffic) / sizeof(long));
	memset(currOutTraffic, 0, sizeof(currOutTraffic) / sizeof(long));
	memset(prevOutTraffic, 0, sizeof(prevOutTraffic) / sizeof(long));

	// Inbound Traffic
	for (int a = 0; a < numberOfSamples; a++)
	{
		printf("interfaces: %d\n", interfaces);
		for (int b = 0; b < interfaces; b++)
			{
				// Inbound
				// printf("MADE IT HERE 1\n");
				// int size = 100;// log10(b + 1) + 1; // For int to string
				// char buffer[100]; // For int to string
				// snprintf(buffer, size, "%d", b + 1); // For int to string

				// printf("MADE IT HERE 2\n");
				// puts(buffer);
				// char octetID[50];
				// strcpy(octetID, buffer);
				// char *pollInOctect = strcat("ifInOctets.", buffer);
				// printf("MADE IT HERE 3\n");
				char pollInOctect[] = "ifInOctets.1";
				pollInOctect[11] += b;
				printf("pollOctect: %s\n", pollInOctect);

				currInTraffic[a] = getTableData(pollInOctect);
				printf("currInTraffic %d: %ld\n", a, currInTraffic[a]);
				prevInTraffic[a] = getTableData(pollInOctect);
				printf("prevInTraffic %d: %ld\n", a, prevInTraffic[a]);

				// Outbound
				// char *pollOutOctect = strcat("ifOutOctets.", buffer);
				char pollOutOctect[] = "ifOutOctets.1";
				pollOutOctect[12] += b;
				printf("pollOutOctect: %s\n", pollOutOctect);

				currOutTraffic[a] = getTableData(pollOutOctect);
				printf("currOutTraffic %d: %ld\n", a, currOutTraffic[a]);
				prevOutTraffic[a] = getTableData(pollOutOctect);
				printf("prevOutTraffic %d: %ld\n", a, prevOutTraffic[a]);

				// free(buffer);
			}
		printTraffic(interfaces, currInTraffic, currOutTraffic);
		printTraffic(interfaces, currOutTraffic, prevOutTraffic);

		time(&startTime);
		do
		{
			time(&endTime);
			timeElapsed = difftime(endTime, startTime);
		} while (timeElapsed < timeInterval);
	}
}
/*********************Get Table Data*******************
This function gets int data from the table.
	PRE:
	POST:
*/
int getTableData(char *objectName){
	netsnmp_pdu *pdu;
	oid anOID[MAX_OID_LEN];
	netsnmp_variable_list *vars;
	size_t anOID_len = MAX_OID_LEN;
	int status;

	pdu = snmp_pdu_create(SNMP_MSG_GET);
	get_node(objectName, anOID, &anOID_len);
	snmp_add_null_var(pdu, anOID, anOID_len);

	status = snmp_synch_response(ss, pdu, &response);

	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR){
		// SUCCESS
		for (vars = response->variables; vars; vars = vars->next_variable){
			if (vars->type == ASN_INTEGER || vars->type == ASN_COUNTER){
				return *vars->val.integer;
			} else {
				vars = vars->next_variable;
			}
		}
	} else {
		// FAILURE
		if (status == STAT_SUCCESS){
			fprintf(stderr, "Error in packet\n Reason: %s\n",
				snmp_errstring(response->errstat));
			return (-1);
		} else if (status == STAT_TIMEOUT){
			fprintf(stderr, "Timeout: No response from %s.\n",
				session.peername);
			return (-2);
		} else {
			snmp_sess_perror("snmpmanager", ss);
		}
	}
	return 0;
}

/*********************Print Traffic*******************
This function prints Traffic in a human readable format
	PRE:
	POST:
*/
void printTraffic(int interfaces, long currTraffic[], long prevTraffic[]){

}
