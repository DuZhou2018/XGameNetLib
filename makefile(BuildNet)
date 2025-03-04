include ../XGSRock/xzhtmake.head

NETINCLUDE  = -I $(ZBOOST)
#NETINCLUDE+= -I $(ZPYTHON)


INC	 = -I ./header/ -I ./include/ -I ./source/ -I ./httpcc/
LIBS = -lpthread -ldl -lutil

 
SRC  = $(wildcard ./source/*.cpp)
SRC += $(wildcard ./base64/*.cpp)
SRC += $(wildcard ./httpcc/*.cpp)
SRC += $(wildcard ./json/*.cpp)
SRC += $(wildcard ./sha1/*.cpp)


.c.o:
	$(CC) $(CFLAGS) $(CFLAG) -o $*.o $(INC) $(NETINCLUDE) $*.c 
 
.cpp.o:
	$(CPLUS) $(CFLAGS) $(CFLAG) -o $*.o $(INC) $(NETINCLUDE) $*.cpp 

OBJ  = $(SRC:.cpp=.o)

TARGET  = libPeyoneNetLibd.a
CFLAG   = -c -std=c++14 -Wno-unused-variable -DOS_LINUX -fPIC -g -DDEBUG
ifeq ($(zver), debug)
	override CFLAG  = -c -std=c++14 -Wno-unused-variable -DOS_LINUX -fPIC -g -DDEBUG
	override TARGET = libPeyoneNetLibd.a
else
	override CFLAG  = -c -std=c++14 -Wno-unused-variable -DOS_LINUX -fPIC -O3
	override TARGET = libPeyoneNetLibr.a
endif

lib:$(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ)
	cp -rf $(TARGET) $(XLIBOUTPATH)

all: lib

test:
	@echo "PRJ_HOME =" ${PRJ_HOME}
	@echo "ZBOOST   =" ${ZBOOST}
	@echo "ZPYTHON  =" ${ZPYTHON}
	@echo "ZMGBSON  =" ${ZMGBSON}

clean:
	find ./ -name "*.o" -exec rm {} \;
	rm $(TARGET) -fr
	rm $(XLIBOUTPATH)/$(TARGET) -fr

cleanoo:
	find ./ -name "*.o" -exec rm {} \;
