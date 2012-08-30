INCLUDE(../../../cmake/macros.cmake)

SET(TXML_SOURCES 
	asciitab.h  
	latin1tab.h  
	xml.h       
	xmlrole.h      
	xmltok_impl.h
	codepage.h  
	nametab.h    
	xmlparse.c  
	xmltok.c
	utf8tab.h    
	xmlparse.h  
	xmltok.h
	filemap.h   
	iasciitab.h  
	xmldef.h     
	xmlrole.c   
	xml.cpp
)

IF(WIN32)
	SET(TXML_SOURCES ${TXML_SOURCES} dllmain.c)
ENDIF(WIN32)

ADD_SDLIB_INCLUDEDIR(portability)

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DTXML_DLL)
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(txml SHARED ${TXML_SOURCES})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(txml PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

IF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)
	ADD_LIBRARY(txml_static STATIC ${TXML_SOURCES})
ENDIF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)

ADD_SDLIB_LIBRARY(txml portability)

ADD_SUBDIRECTORY(gennmtab)

IF(WIN32)
	SD_INSTALL_FILES(BIN TARGETS txml)
ELSE(WIN32)
	SD_INSTALL_FILES(LIB lib TARGETS txml)
ENDIF(WIN32)

SD_INSTALL_FILES(INCLUDE FILES xml.h xmlparse.h)

