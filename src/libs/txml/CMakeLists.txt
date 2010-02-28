INCLUDE(../../../cmake/macros.cmake)

IF(WIN32)
	SET(TXML_SOURCES 
	asciitab.h  
	latin1tab.h  
	xml.h       
	xmlrole.h      
	xmltok_impl.h
	codepage.h  
	hashtable.c  
	nametab.h    
	xmlparse.c  
	xmltok.c
	hashtable.h  
	utf8tab.h    
	xmlparse.h  
	xmltok.h
	filemap.h   
	iasciitab.h  
	xmldef.h     
	xmlrole.c   
	txml.def
	dllmain.c
	xml.cpp
	)
ENDIF(WIN32)

IF(UNIX)
	SET(TXML_SOURCES 
	asciitab.h  
	latin1tab.h  
	xml.h       
	xmlrole.h      
	xmltok_impl.h
	codepage.h  
	hashtable.c  
	nametab.h    
	xmlparse.c  
	xmltok.c
	hashtable.h  
	utf8tab.h    
	xmlparse.h  
	xmltok.h
	filemap.h   
	iasciitab.h  
	xmldef.h     
	xmlrole.c   
	xml.cpp
)
ENDIF(UNIX)

ADD_TORCSLIB_INCLUDEDIR(portability)

IF(UNIX)
	ADD_LIBRARY(txml SHARED ${TXML_SOURCES})
ENDIF(UNIX)

IF(WIN32)
	ADD_LIBRARY(txml SHARED ${TXML_SOURCES} txml.def dllmain.c)
ENDIF(WIN32)

IF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)
	ADD_LIBRARY(txml_static STATIC ${TXML_SOURCES})
ENDIF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)

ADD_SUBDIRECTORY(gennmtab)

IF(WIN32)
	TORCS_INSTALL_FILES(BIN TARGETS txml)
ELSE(WIN32)
	TORCS_INSTALL_FILES(LIB lib TARGETS txml)
ENDIF(WIN32)
TORCS_INSTALL_FILES(INCLUDE FILES xml.h xmlparse.h)

