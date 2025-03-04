cmake_minimum_required(VERSION 2.6)

SET(PeonyNet ./)
SET(P_INCLDE ./include)
SET(P_HEADER ./header)
SET(BOOST_INCLUDE ../../../XDepend/boost_1_46_1)

SET(BOOST_LIB ../../../XDepend/boost_1_46_1/stage/lib)

AUX_SOURCE_DIRECTORY(./source P_SRC)

INCLUDE_DIRECTORIES(. ${P_INCLDE} 
	${P_HEADER}
	${BOOST_INCLUDE}
	 )

LINK_DIRECTORIES( ${BOOST_LIB} )
LINK_LIBRARIES( boost_thread-mt boost_random-mt boost_regex-mt boost_filesystem-mt boost_system-mt boost_date_time-mt ) 

ADD_DEFINITIONS(-O2 -g -Wall -fPIC) 

SET(EXECUTABLE_OUTPUT_PATH bin)
SET(LIBRARY_OUTPUT_PATH bin)

ADD_LIBRARY(PeyoneNetLIb_d STATIC ${P_SRC})