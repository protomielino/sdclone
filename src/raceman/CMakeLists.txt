INCLUDE(../../cmake/macros.cmake)

SET(RACEMAN_FILES practice.xml quickrace.xml
                  singleevent-36gp.xml
                  singleevent-challenge.xml singleevent-endurance.xml
                  singleevent-ls-gt1.xml singleevent-mp5.xml
                  championship.xml championship-supercars.xml championship-trb1.xml
                  championship-36gp.xml championship-ls-gt1.xml championship-mp5.xml
                  career.xml 
                  career_supercars.xmls career_ls1.xmls)

# Work-in-progress / unofficial race modes.
IF(NOT OPTION_OFFICIAL_ONLY)

  SET(RACEMAN_FILES ${RACEMAN_FILES} networkrace.xml championship-mpa1.xml)

ENDIF()

SD_INSTALL_FILES(DATA config/raceman USER config/raceman FILES ${RACEMAN_FILES})

SET(EXTRA_FILES extra/skill.xml)

SD_INSTALL_FILES(DATA config/raceman/extra USER config/raceman/extra 
                 FILES ${EXTRA_FILES})

# Fictive = no-output target, for having source files available in IDEs.
ADD_CUSTOM_TARGET(raceman SOURCES ${RACEMAN_FILES} ${EXTRA_FILES})

