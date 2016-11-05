#
# Warning: you may need more libraries than are included here on the
# build line.  The agent frequently needs various libraries in order
# to compile pieces of it, but is OS dependent and we can't list all
# the combinations here.  Instead, look at the libraries that were
# used when linking the snmpd master agent and copy those to this
# file.
#

CC=gcc

OBJS1=snmpdemoapp.o
OBJS2=example-demon.o nstAgentSubagentObject.o
OBJS3=asyncapp.o
OBJS4=snmpmanager.o
OBJS5=snmpmanager.o snmpmanager
TARGETS=example-demon snmpdemoapp asyncapp

CFLAGS=-I. `net-snmp-config --cflags`
BUILDLIBS=`net-snmp-config --libs`
BUILDAGENTLIBS=`net-snmp-config --agent-libs`

# shared library flags (assumes gcc)
DLFLAGS=-fPIC -shared

all: $(TARGETS)

fixsnmpd:
	sudo apt-get remove --purge snmpd
	sudo rm /usr/local/lib/libnetsnmp*
	sudo apt-get install snmpd
	sudo nano /etc/snmp/snmpd.conf

addlibs:
	cp -a $(HOME)/HW2/lib/. /usr/local/lib/

snmpmanager: $(OBJS4)
	$(CC) -o snmpmanager $(OBJS4) $(BUILDLIBS)

snmpdemoapp: $(OBJS1)
	$(CC) -o snmpdemoapp $(OBJS1) $(BUILDLIBS)

asyncapp: $(OBJS3)
	$(CC) -o asyncapp $(OBJS3) $(BUILDLIBS)

example-demon: $(OBJS2)
	$(CC) -o example-demon $(OBJS2)  $(BUILDAGENTLIBS)

clean:
	rm $(OBJS5)

nstAgentPluginObject.so: nstAgentPluginObject.c Makefile
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o nstAgentPluginObject.o nstAgentPluginObject.c
	$(CC) $(CFLAGS) $(DLFLAGS) -o nstAgentPluginObject.so nstAgentPluginObject.o
