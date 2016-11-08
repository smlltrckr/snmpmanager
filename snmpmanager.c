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
void findDevice();
char **deviceNeighbors(char *device);
void trafficV3(int timeInterval, int numberOfSamples);
int getTableData(char *objectName);
struct trafficData *getTableDataTest(int interfaces);

int main(int argc, char ** argv){
	int timeInterval, numberOfSamples;
	char *agentIP, *community;

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

	trafficV3(timeInterval, numberOfSamples);

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
			printf("Sample\tInterface\tIn (Mbps)\tOut (Mbps)\n");
			printf("**************************************************\n");
		}
		start = getTableData("system.sysUpTime.0");
		prevTraffic = getTableDataTest(interfaces);
		sleep(timeBetweenPolls);
		currTraffic = getTableDataTest(interfaces);
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

struct trafficData *getTableDataTest(int interfaces){
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
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		sprintf(interfaceName, "ifInOctets.%d", i + 1);
		get_node(interfaceName, anOID, &anOID_len);
		snmp_add_null_var(pdu, anOID, anOID_len);

		status = snmp_synch_response(ss, pdu, &response);

		if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR){
			// SUCCESS
			for (vars = response->variables; vars; vars = vars->next_variable){
				if (vars->type == ASN_COUNTER){
					tData->ifInOctets[i] = *vars->val.integer;
				} else {
					vars = vars->next_variable;
				}
			}
			
		} else {
			// FAILURE
			if (status == STAT_SUCCESS){
				fprintf(stderr, "Error in packet\n Reason: %s\n",
					snmp_errstring(response->errstat));
				exit (-1);
			} else if (status == STAT_TIMEOUT){
				fprintf(stderr, "Timeout: No response from %s.\n",
					session.peername);
				exit (-2);
			} else {
				snmp_sess_perror("snmpmanager", ss);
			}
		}

	
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		sprintf(interfaceName, "ifOutOctets.%d", i + 1);
		get_node(interfaceName, anOID, &anOID_len);
		snmp_add_null_var(pdu, anOID, anOID_len);
		status = snmp_synch_response(ss, pdu, &response);

		if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR){
			// SUCCESS
			for (vars = response->variables; vars; vars = vars->next_variable){
				if (vars->type == ASN_COUNTER){
					tData->ifOutOctets[i] = *vars->val.integer;
				} else {
					vars = vars->next_variable;
				}
			}
			
		} else {
			// FAILURE
			if (status == STAT_SUCCESS){
				fprintf(stderr, "Error in packet\n Reason: %s\n",
					snmp_errstring(response->errstat));
				exit (-1);
			} else if (status == STAT_TIMEOUT){
				fprintf(stderr, "Timeout: No response from %s.\n",
					session.peername);
				exit (-2);
			} else {
				snmp_sess_perror("snmpmanager", ss);
			}
		}
	}
	return tData;
}