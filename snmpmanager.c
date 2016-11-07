#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_IPV4 15
#define BUCKET 5

// Global Variables
netsnmp_session session, *ss;
netsnmp_pdu *pdu;
netsnmp_pdu *response;

// Function Declarations
char **deviceNeighbors(char *device);
void traffic();

netsnmp_variable_list *getBulk(netsnmp_session *ss, oid *firstOid, size_t firstLen);

int main(int argc, char ** argv){
	int timeInterval, numberOfSamples;
	char *agentIP, *community;
	netsnmp_pdu *pdu; // Use locally in separate functions

	if (argc < 4){
		printf("USAGE: timeInterval(Seconds) numberOfSamples agentIP community\n");
		// Default Values?
		timeInterval = 300;
		numberOfSamples = 30;
		agentIP = "127.0.0.1";
		community = "public";
		printf("Default: timeInterval(%d), numberOfSamples(%d), agentIP(%s), community(%s)\n", timeInterval, numberOfSamples, agentIP, community);
	} else {
		timeInterval = atoi(argv[1]);
		numberOfSamples = atoi(argv[2]);
		agentIP = argv[3];
		community = argv[4];
	}

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	netsnmp_variable_list *vars;
	int status;
	int count = 1;

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
	
	anOID_len = MAX_OID_LEN;
	if (!snmp_parse_oid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len))
	{
		snmp_perror(".1.3.6.1.2.1.1.1.0");
		SOCK_CLEANUP;
		exit(1);
	}
	getBulk(ss, anOID, anOID_len);
//	pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
//	anOID_len = MAX_OID_LEN;
//	if (!snmp_parse_oid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len))
//	{
//		snmp_perror(".1.3.6.1.2.1.1.1.0");
//		SOCK_CLEANUP;
//		exit(1);
//	}
//
//	snmp_add_null_var(pdu, anOID, anOID_len);
//
//	/* Send out a request */
//	status = snmp_synch_response(ss, pdu, &response);
//
//	//Start test result
//	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
//      /*
//       * SUCCESS: Print the result variables
//       */
//
//      for(vars = response->variables; vars; vars = vars->next_variable)
//        print_variable(vars->name, vars->name_length, vars);
//
//      /* manipuate the information ourselves */
//      for(vars = response->variables; vars; vars = vars->next_variable) {
//        if (vars->type == ASN_OCTET_STR) {
//	  char *sp = (char *)malloc(1 + vars->val_len);
//	  memcpy(sp, vars->val.string, vars->val_len);
//	  sp[vars->val_len] = '\0';
//          printf("value #%d is a string: %s\n", count++, sp);
//	  free(sp);
//	}
//        else
//          printf("value #%d is NOT a string! Ack!\n", count++);
//      }
//    } else {
//      /*
//       * FAILURE: print what went wrong!
//       */
//
//      if (status == STAT_SUCCESS)
//        fprintf(stderr, "Error in packet\nReason: %s\n",
//                snmp_errstring(response->errstat));
//      else if (status == STAT_TIMEOUT)
//        fprintf(stderr, "Timeout: No response from %s.\n",
//                session.peername);
//      else
//        snmp_sess_perror("snmpdemoapp", ss);
//
//    }//END test result
//
//	/* Clean up and close connection */
//	if (response)
//	{
//		snmp_free_pdu(response);
//	}
//
	snmp_close(ss);

	SOCK_CLEANUP;

	return 0;
} // END main()


/*********************Find Devices*******************
This function finds a device and returns it to a list
	PRE:
	POST:
*/
netsnmp_variable_list *getBulk(netsnmp_session *ss, oid *firstOid, size_t firstLen){
	netsnmp_variable_list *vars;
	int status;
	int count = 1;
	int run = 1;

    oid currOid[MAX_OID_LEN];
    size_t currLen;
    oid lastOid[MAX_OID_LEN];
    size_t lastLen = 0;

	memmove(lastOid, firstOid, firstLen*sizeof(oid));
    lastLen = firstLen;
    lastOid[lastLen-1]++;

    memmove(currOid, firstOid, firstLen * sizeof(oid));
    currLen = firstLen;
	
	do{	
		pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
		snmp_add_null_var(pdu, currOid, currLen);
	
		status = snmp_synch_response(ss, pdu, &response);
	
		if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR){
	    	for(vars = response->variables; vars; vars = vars->next_variable){
                if (snmp_oid_compare(lastOid, lastLen, vars->name, vars->name_length) <= 0) {
                    run = 0;
                    continue;
                }
                print_variable(vars->name, vars->name_length, vars);
                memmove((char *) currOid, (char *) vars->name, vars->name_length * sizeof(oid));
                currLen = vars->name_length;
            }
        }
		if(response){
			snmp_free_pdu(response);
		}
	}while(run);
	return vars;
}


/*********************Find Single Device*****************
This function gets the device information
	PRE:
	POST:
*/
/********************Find Neighbor*******************
This function finds the neighbor for each device
	PRE: device identifier (String?)
	POST: Array of Devices returned
*/
char **deviceNeighbors(char *device){
	char **neighbors;

	neighbors = (char**)malloc(MAX_IPV4 * sizeof(char*));
	*neighbors = (char*) malloc(MAX_IPV4 * sizeof(char));



	return neighbors;
}

/*********************Traffic***********************
This function finds the rate of traffic on each interface
	PRE:
	POST:
*/
void traffic(int interfaceID){

}

/*********************Print Traffic*******************
This function prints Traffic in a human readable format
	PRE:
	POST:
*/

