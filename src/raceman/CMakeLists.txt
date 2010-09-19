INCLUDE(../../cmake/macros.cmake)

FILE(GLOB files_xml *.xml)
FILE(GLOB files_xmls *.xmls)

SD_INSTALL_FILES(DATA config/raceman USER config/raceman FILES ${files_xml}
	${files_xmls})
SD_INSTALL_FILES(DATA config/raceman/extra USER config/raceman/extra FILES
	extra/skill.xml)

