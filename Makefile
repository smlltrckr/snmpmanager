#
# Warning: you may need more libraries than are included here on the
# build line.  The agent frequently needs various libraries in order
# to compile pieces of it, but is OS dependent and we can't list all
# the combinations here.  Instead, look at the libraries that were
# used when linking the snmpd master agent and copy those to this
# file.
#

CC=gcc

OBJS4=snmpmanager.o
OBJS5=snmpmanager.o snmpmanager
TARGETS=example-demon snmpdemoapp asyncapp

CFLAGS=-I. `net-snmp-config --cflags`
BUILDLIBS=`net-snmp-config --libs`
BUILDAGENTLIBS=`net-snmp-config --agent-libs`
M=updated
# shared library flags (assumes gcc)
DLFLAGS=-fPIC -shared

all: $(TARGETS)

fixsnmpd:
	sudo apt-get remove --purge snmpd
	sudo rm /usr/local/lib/libnetsnmp*
	sudo apt-get install snmpd
	sudo nano /etc/snmp/snmpd.conf
	sudo service snmpd restart

addlibs:
	cp -a $(HOME)/HW2/lib/. /usr/local/lib/

snmpmanager: $(OBJS4)
	$(CC) -o snmpmanager $(OBJS4) $(BUILDLIBS)

gitmain:
	git add snmpmanager.c Makefile
	git commit -m $(M)
	git push origin main

clean:
	rm $(OBJS5)
