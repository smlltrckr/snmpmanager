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

// trafficData struct
struct trafficData
{
	long *ifOutOctets;
	long *ifInOctets;
};


// Function Declarations
void trafficV3(int timeInterval, int numberOfSamples);
int getTableData(char *objectName);
int getNext(oid *anOID, size_t anOID_len, int interfc, oid *endOID);
struct trafficData *getOctets(int interfaces);
oid *getEndOID(char *anOID);
void printAssignmentHeader();

int main(int argc, char ** argv){
	int timeInterval, numberOfSamples;
	char *agentIP, *community;
	oid anOID[MAX_OID_LEN], endOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;
	int interfaces; 
	// netsnmp_session session, *ss;
	// netsnmp_pdu *response;

	if (argc < 4){
		printf("USAGE: timeInterval(Seconds) numberOfSamples agentIP community\n");
		// Default Values
		timeInterval = 10;
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

	// Function Calls
	printAssignmentHeader();
	printf("Interface\tIpAddress\n");
	printf("**************************************************\n");
	interfaces = getTableData("ifNumber.0");
	if (!snmp_parse_oid("ipAdEntAddr", anOID, &anOID_len)) { 
      snmp_perror("ipAdEntAddr");
      exit(1); 
	}

	getNext(anOID, anOID_len, 1, getEndOID("ipAdEntIfIndex"));

	printf("\nInterface\tNeghbors\n");
	printf("**************************************************\n");
	if (!snmp_parse_oid("ipNetToMediaNetAddress", anOID, &anOID_len)) { 
      snmp_perror("ipNetToMediaNetAddress");
      exit(1); 
	}

	getNext(anOID, anOID_len, 1, getEndOID("ipNetToMediaType"));

	trafficV3(timeInterval, numberOfSamples);
	// END Function Calls


	/* Clean up and close connection */
	if (response)
	{
		snmp_free_pdu(response);
	}
	snmp_close(ss);

	SOCK_CLEANUP;

	return 0;
} // END main()

/*********************Traffic***********************
This function finds the rate of traffic on each interface and prints the results
	PRE:
	POST:
*/
void trafficV3(int timeInterval, int numberOfSamples){
	int interfaces = getTableData("ifNumber.0"); // Number of interfaces
	long start, end;
	double elapsedTime;
	struct trafficData *prevTraffic, *currTraffic;
	char interfaceName[20];
	int timeBetweenPolls = 5;

	for (int a = 0; a < numberOfSamples; a++)
	{
		if(a == 0){
			printf("\nSample\tInterface\tIn (Mbps)\tOut (Mbps)\n");
			printf("**************************************************\n");
		}
		start = getTableData("system.sysUpTime.0");
		prevTraffic = getOctets(interfaces);
		sleep(timeBetweenPolls);
		currTraffic = getOctets(interfaces);
		end = getTableData("system.sysUpTime.0");
		elapsedTime = ((end - start) / 100.0) + timeBetweenPolls;
		for (int b = 0; b < interfaces; b++){
			double mbpsIn = (currTraffic->ifInOctets[b] - prevTraffic->ifInOctets[b]) * 8.0 / 1000000.0 / elapsedTime;
			double mbpsOut = (currTraffic->ifOutOctets[b] - prevTraffic->ifOutOctets[b]) * 8.0 / 1000000.0 / elapsedTime;
			if(b == 0){
				printf("%d\t%d\t\t%-9.2lf\t%-9.2lf\n",a + 1, b + 1, mbpsIn, mbpsOut);
			} else {
				printf("\t%d\t\t%-9.2lf\t%-9.2lf\n", b + 1, mbpsIn, mbpsOut);
			}
		}
		if(a != numberOfSamples - 1){
			sleep(timeInterval);
		}
	}
	free(prevTraffic->ifInOctets);
	free(prevTraffic->ifOutOctets);
	free(prevTraffic);
	free(currTraffic->ifInOctets);
	free(currTraffic->ifOutOctets);
	free(currTraffic);
	return;
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
			if (vars->type == ASN_INTEGER || vars->type == ASN_COUNTER || vars->type == ASN_TIMETICKS){
				return *vars->val.integer;
			} else{
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

/*********************Get Octets*******************
This function gathers up the In/Out octets of 
each interface.
	PRE: 
	POST:
*/
struct trafficData *getOctets(int interfaces){
	netsnmp_pdu *pdu;
	oid anOID[MAX_OID_LEN];
	netsnmp_variable_list *vars;
	size_t anOID_len = MAX_OID_LEN;
	int status;
	struct trafficData *tData;

	char interfaceName[20];

	tData = (struct trafficData *) malloc(sizeof(struct trafficData) + (sizeof(long) * interfaces)+ (sizeof(long) * interfaces));
	tData->ifOutOctets = (long *) malloc(sizeof(long) * interfaces);
	tData->ifInOctets = (long *) malloc(sizeof(long) * interfaces);
	
	for (int i = 0; i < interfaces; i++)
	{
		sprintf(interfaceName, "ifInOctets.%d", i + 1);
		tData->ifInOctets[i] = getTableData(interfaceName);

		sprintf(interfaceName, "ifOutOctets.%d", i + 1);
		tData->ifOutOctets[i] = getTableData(interfaceName);
	}
	return tData;
}


/*********************Get Next*******************
This function recursively gets the next node and prints results
	PRE: 
	POST:
*/
int getNext(oid *anOID, size_t anOID_len, int interfc, oid *endOID){
	netsnmp_pdu *pdu, *nextPdu;
	netsnmp_variable_list *vars;
	int status;
	char ipAddress[50];
	char *ip;
	ip = strtok(ipAddress, "IpAddress: ");
	oid anOID2[MAX_OID_LEN];
	size_t anOID_len2 = MAX_OID_LEN;

	if (snmp_oid_compare(anOID, anOID_len, endOID, anOID_len) < 0)
	{
		pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
		snmp_add_null_var(pdu, anOID, anOID_len);

		status = snmp_synch_response(ss, pdu, &response);

		if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR){
		// SUCCESS: Results are in response
			vars = response->variables;
			if (vars->type == ASN_IPADDRESS){
				// print_variable(vars->name, vars->name_length, vars);
				
				snprint_ipaddress (ipAddress, sizeof(ipAddress), vars, NULL, NULL, NULL);
				ip = strtok(ipAddress, "IpAddress: ");
				// printf("%ld\n", strlen(ipAddress));
				printf("%d\t\t%s\n", interfc, ip);
			}
			// TODO need to have some kind of check for Interface neighbors
			if (!snmp_parse_oid("ipNetToMediaNetAddress", anOID2, &anOID_len2)) { 
			    snmp_perror("ipNetToMediaNetAddress");
			    exit(1); 
			}
			if (snmp_oid_compare(anOID, anOID_len, anOID2, anOID_len2) == 1) {
				getNext(vars->name, vars->name_length, 1, endOID);
			} else {
				getNext(vars->name, vars->name_length, interfc + 1, endOID);
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
	}
	return 0;	
}

oid *getEndOID(char *anOID){
	netsnmp_pdu *pdu;
	oid endOID[MAX_OID_LEN];
	size_t endOID_len = MAX_OID_LEN;
	netsnmp_variable_list *vars;
	int status;

	pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);

	if (!snmp_parse_oid(anOID, endOID, &endOID_len)) { 
    	snmp_perror(anOID);
    	exit(1); 
	}

	snmp_add_null_var(pdu, endOID, endOID_len);
	status = snmp_synch_response(ss, pdu, &response);

	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) { 
      // Success
		// print_variable(response->variables->name, response->variables->name_length, response->variables);
		return response->variables->name;

	} else {
       // FAILURE: print what went wrong!

		if (status == STAT_SUCCESS){
			fprintf(stderr, "DEBUG:: Error in packet\nReason: %s\n", 
				snmp_errstring(response->errstat));
			exit (-1);
		}
		else if (status == STAT_TIMEOUT){
			fprintf(stderr, "Timeout: No response from %s.\n", session.peername);
			exit (-2);
		}
		else
			snmp_sess_perror("snmpdemoapp", ss);
		exit (-3);
	}
	return NULL;
}

void printAssignmentHeader(){
	printf("**************************************************\n");
	printf("*                  Assignment 2                  *\n");
	printf("*                     CS158B                     *\n");
	printf("*           By Sam Rucker, Dustin Tran           *\n");
	printf("**************************************************\n\n");
}