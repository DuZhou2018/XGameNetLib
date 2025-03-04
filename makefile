include ../XGSRock/xzhtmake.head

#build for python exlib
#NETBTYPE   = ForPyExLib
NETBTYPE    = ForCppNet

NETINCLUDE  = -I $(ZBOOST)
NETINCLUDE += -I $(ZPYTHON)/Include/python2.7


CFLAG   = -c -std=c++14 -Wno-unused-variable -DOS_LINUX -fPIC -g -DDEBUG
ifeq ($(zver), debug)
	override CFLAG  = -c -std=c++14 -Wno-unused-variable -DOS_LINUX -fPIC -g -DDEBUG
else
	override CFLAG  = -c -std=c++14 -Wno-unused-variable -DOS_LINUX -fPIC -O3
endif

#LIBS     = -lpthread -ldl -lutil
LIBS      = -lpthread -ldl -lutil -lnsl -lreadline -ltermcap -lieee -lm -lrt
LIBS     += -L$(XDEPEND)/lib-win-centos/centos_lib
LIBS     += -L$(ZBOOSTLB) -lboost_thread -lboost_random -lboost_filesystem -lboost_system -lboost_date_time -lboost_exception -lboost_python27


INC	 = -I ./header/ -I ./include/ -I ./source/ -I ./httpcc/

 
SRC  = $(wildcard ./source/*.cpp)
SRC += $(wildcard ./base64/*.cpp)
SRC += $(wildcard ./httpcc/*.cpp)
SRC += $(wildcard ./json/*.cpp)
SRC += $(wildcard ./sha1/*.cpp)

ifeq ($(NETBTYPE), ForPyExLib)
    SRC += $(wildcard ./PyWrap/*.cpp)
endif


.c.o:
	$(CC) $(CFLAGS) $(CFLAG) -o $*.o $(INC) $(NETINCLUDE) $*.c 
 
.cpp.o:
	$(CPLUS) $(CFLAGS) $(CFLAG) -o $*.o $(INC) $(NETINCLUDE) $*.cpp 

OBJ  = $(SRC:.cpp=.o)


TARGET  = 
ifeq ($(NETBTYPE), ForPyExLib)
	ifeq ($(zver), debug)
		override TARGET = peyonenetd.so
	else
		override TARGET = peyonenetr.so
	endif

lib:$(OBJ)
	g++ -o $(TARGET) $(OBJ) -shared -fPIC $(LIBS)
else
	ifeq ($(zver), debug)
		override TARGET = libPeyoneNetLibd.a
	else
		override TARGET = libPeyoneNetLibr.a
	endif
	
lib:$(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ)
	cp -rf $(TARGET) $(XLIBOUTPATH)
endif


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
	
#cp peyonenetd.so  /usr/lib64/python2.7/site-packages/peyone/peyonenet.so
