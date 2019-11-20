
# ACHTUNG unbedingt TABS benutzen beim einrücken

# http://stackoverflow.com/questions/2394609/makefile-header-dependencies
# http://lackof.org/taggart/hacking/make-example/
# http://owen.sj.ca.us/~rk/howto/slides/make/slides/makerecurs.html
# https://gcc.gnu.org/onlinedocs/gcc/Invoking-GCC.html
# http://www.ijon.de/comp/tutorials/makefile.html (Deutsch)

CC = g++
CFLAGS = -Wall -O3 -std=c++14 -pthread -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections -lpthread -static-libgcc -static-libstdc++
DIRS = SocketLib
OBJLIBS = libsocketlib.a

BUILDDIRS = $(DIRS:%=build-%)
CLEANDIRS = $(DIRS:%=clean-%)

INC_PATH = 
LIB_PATH = -L ./SocketLib

OBJ = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
LIB = -l socketlib -l crypto -l ssl

all: Ssl-Example Tcp-Example Dtls-Example

Ssl-Example: $(BUILDDIRS) $(OBJ)
	$(CC) -o Ssl-Example Ssl-Example.o $(LIB_PATH) $(LIB) $(LDFLAGS)

Tcp-Example: $(BUILDDIRS) $(OBJ)
	$(CC) -o Tcp-Example Tcp-Example.o $(LIB_PATH) $(LIB) $(LDFLAGS)

Dtls-Example: $(BUILDDIRS) $(OBJ)
	$(CC) -o Dtls-Example Dtls-Example.o $(LIB_PATH) $(LIB) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC_PATH) -c $<

$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANDIRS)
	rm -f Ssl-Example Tcp-Example Dtls-Example $(OBJ) *~
	rm -f *.o *~

$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: clean


