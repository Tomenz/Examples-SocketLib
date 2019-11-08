
# ACHTUNG unbedingt TABS benutzen beim einrücken

# http://stackoverflow.com/questions/2394609/makefile-header-dependencies
# http://lackof.org/taggart/hacking/make-example/
# http://owen.sj.ca.us/~rk/howto/slides/make/slides/makerecurs.html
# https://gcc.gnu.org/onlinedocs/gcc/Invoking-GCC.html
# http://www.ijon.de/comp/tutorials/makefile.html (Deutsch)

CC = g++
CFLAGS = -Wall -O3 -std=c++14 -pthread -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections -lpthread -static-libgcc -static-libstdc++
TARGET = Http2Serv
DIRS = SocketLib
OBJLIBS = libsocketlib.a

BUILDDIRS = $(DIRS:%=build-%)
CLEANDIRS = $(DIRS:%=clean-%)

INC_PATH = -I ./brotli/include
LIB_PATH = -L ./zlib -L ./socketlib -L ./brotli -L ./CommonLib

OBJ = Http2Serv.o HttpServ.o ConfFile.o LogFile.o Trace.o TempFile.o SpawnProcess.o HPack.o #OBJ = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
LIB = -l socketlib -l crypto -l ssl

all: $(TARGET)

#mDnsServ: $(BUILDDIRS) mDnsServ.o DnsProtokol.o
#	$(CC) -o mDnsServ mDnsServ.o DnsProtokol.o $(LIB_PATH) $(LIB) $(LDFLAGS)

$(TARGET): $(BUILDDIRS) $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LIB_PATH) $(LIB) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC_PATH) -c $<

$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANDIRS)
	rm -f $(TARGET) $(OBJ) *~
	rm -f mDnsServ mDnsServ.o DnsProtokol.o *~

$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: clean


