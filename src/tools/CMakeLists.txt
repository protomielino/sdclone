INCLUDE(../../cmake/macros.cmake)

ADD_SUBDIRECTORY(accc)
ADD_SUBDIRECTORY(nfs2ac)
ADD_SUBDIRECTORY(nfsperf)
#texmapper is not a cross platform application
IF(UNIX)
	ADD_SUBDIRECTORY(texmapper)
ENDIF(UNIX)
ADD_SUBDIRECTORY(trackgen)
ADD_SUBDIRECTORY(xmlversion)
ADD_SUBDIRECTORY(menuview)
