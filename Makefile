CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls \
-Wmissing-declarations -Wold-style-definition -Wmissing-prototypes \
-Wdeclaration-after-statement -Wno-return-local-addr \
-Wuninitialized -Wextra -Wunused
#CFLAGS += -Werror # for treating warnings as compilation errors


PROG = psush
AOUT = psush 
TAR_FILE = ${LOGNAME}_$(PROG).tar.gz

all: $(PROG)

$(PROG): $(PROG).o cmd_parse.o
	$(CC) $(CFLAGS) -o $(AOUT) $(PROG).o cmd_parse.o
	chmod og-rx $(AOUT)

$(PROG).o: $(PROG).c cmd_parse.h
	$(CC) $(CFLAGS) -c $(PROG).c

cmd_parse.o: cmd_parse.c cmd_parse.h
	$(CC) $(CFLAGS) -c cmd_parse.c

opt: clean
	make DEBUG=-O3

tar: clean
	tar cvfa $(PROG).tar.gz *.[ch] ?akefile

clean:
	rm -f $(PROG) *.o *~ \#*

