################################################################################################
# Find a generic dependency, handling debug suffix
# all the parameters are required ; in case of lists or empty parameter, use "" when calling
################################################################################################

MACRO(FIND_DEPENDENCY DEP_NAME INCLUDE_FILE INCLUDE_SUBDIRS LIBRARY_NAMES SEARCH_PATH_LIST DEBUG_SUFFIX)

    #MESSAGE(STATUS "Searching for 3rd party dependency DEP_NAME='${DEP_NAME}' INCLUDE_FILE='${INCLUDE_FILE}' INCLUDE_SUBDIRS='${INCLUDE_SUBDIRS}' LIBRARY_NAMES='${LIBRARY_NAMES}' SEARCH_PATH_LIST='${SEARCH_PATH_LIST}' DEBUG_SUFFIX='${DEBUG_SUFFIX}' ...")

	# Convert possibly a simple string to a real list.
	SET(_INCLUDE_SUBDIRS)
	LIST(APPEND _INCLUDE_SUBDIRS ${INCLUDE_SUBDIRS})
	LIST(LENGTH _INCLUDE_SUBDIRS _NB_DIRS)
	#MESSAGE(STATUS "_INCLUDE_SUBDIRS=${_INCLUDE_SUBDIRS}, _NB_DIRS=${_NB_DIRS}")
	
	# Find include dirs
    SET(MY_PATH_INCLUDE )
    FOREACH(MY_PATH ${SEARCH_PATH_LIST} )
		IF(${_NB_DIRS} GREATER 0)
			FOREACH(MY_SUBDIR ${_INCLUDE_SUBDIRS} )
				#MESSAGE(STATUS "MY_PATH='${MY_PATH}', MY_SUBDIR='${MY_SUBDIR}'")
				IF(NOT "${MY_SUBDIR}" STREQUAL "")
					SET(MY_SUBDIR "/${MY_SUBDIR}")
				ENDIF(NOT "${MY_SUBDIR}" STREQUAL "")
				SET(MY_PATH_INCLUDE ${MY_PATH_INCLUDE} ${MY_PATH}/include${MY_SUBDIR})
			ENDFOREACH(MY_SUBDIR ${_INCLUDE_SUBDIRS} )
		ELSE(${_NB_DIRS} GREATER 0)
			SET(MY_PATH_INCLUDE ${MY_PATH_INCLUDE} ${MY_PATH}/include)
		ENDIF(${_NB_DIRS} GREATER 0)
    ENDFOREACH(MY_PATH ${SEARCH_PATH_LIST} )
    
	#MESSAGE(STATUS "MY_PATH_INCLUDE='${MY_PATH_INCLUDE}'")
    FIND_PATH("${DEP_NAME}_INCLUDE_DIR" ${INCLUDE_FILE}
      ${MY_PATH_INCLUDE}
      NO_DEFAULT_PATH
    )
    MARK_AS_ADVANCED("${DEP_NAME}_INCLUDE_DIR")
    #MESSAGE(" ${DEP_NAME}_INCLUDE_DIR = '${${DEP_NAME}_INCLUDE_DIR}'")
	
	# Find library files
    SET(MY_PATH_LIB )
    FOREACH(MY_PATH ${SEARCH_PATH_LIST} )
		SET(MY_PATH_LIB ${MY_PATH_LIB} ${MY_PATH}/lib)
    ENDFOREACH(MY_PATH ${SEARCH_PATH_LIST} )
    
    FIND_LIBRARY("${DEP_NAME}_LIBRARY" 
      NAMES ${LIBRARY_NAMES}
      PATHS ${MY_PATH_LIB}
      NO_DEFAULT_PATH
    )
    MARK_AS_ADVANCED("${DEP_NAME}_LIBRARY")
    #MESSAGE(" ${DEP_NAME}_LIBRARY = '${${DEP_NAME}_LIBRARY}'")

	# Whatever happened, done.
    SET(${DEP_NAME}_FOUND "NO" )
    IF(${DEP_NAME}_INCLUDE_DIR AND ${DEP_NAME}_LIBRARY)
      SET( ${DEP_NAME}_FOUND "YES" )
    ENDIF(${DEP_NAME}_INCLUDE_DIR AND ${DEP_NAME}_LIBRARY)

ENDMACRO(FIND_DEPENDENCY DEP_NAME INCLUDE_FILE INCLUDE_SUBDIRS LIBRARY_NAMES SEARCH_PATH_LIST DEBUG_SUFFIX)


################################################################################################
# this Macro is tailored to Mike dependencies
################################################################################################

MACRO(SEARCH_3RDPARTY ROOT_DIR)

    FIND_DEPENDENCY(JPEG jpeglib.h "" jpeg_s ${ROOT_DIR} "")
    
    FIND_DEPENDENCY(OPENAL AL/al.h "" openal32 ${ROOT_DIR} "")
	
    FIND_DEPENDENCY(ENET enet/enet.h "" enet ${ROOT_DIR} "")
	
    FIND_DEPENDENCY(SDL sdl.h "SDL;SDL2" sdl "${ROOT_DIR}" "")
    FIND_DEPENDENCY(SDLMAIN sdl_main.h "SDL;SDL2" sdlmain "${ROOT_DIR}" "")
	IF(SDL_FOUND) # Dirty hack to make FindPackage(SDL) work later.
		SET(SDL_LIBRARY_TEMP ${SDL_LIBRARY} CACHE FILEPATH "")
	ENDIF(SDL_FOUND)

    FIND_DEPENDENCY(PLIB plib/sg.h "" sg ${ROOT_DIR} "")
    FIND_DEPENDENCY(PLIB_SSG plib/ssg.h "" ssg ${ROOT_DIR} "")
    FIND_DEPENDENCY(PLIB_SG plib/sg.h "" sg ${ROOT_DIR} "")
    FIND_DEPENDENCY(PLIB_SL plib/sl.h "" sl ${ROOT_DIR} "")
    FIND_DEPENDENCY(PLIB_SSGAUX plib/ssgaux.h "" ssgaux ${ROOT_DIR} "")
    FIND_DEPENDENCY(PLIB_UL plib/ul.h "" ul ${ROOT_DIR} "")
    FIND_DEPENDENCY(PLIB_JS plib/js.h "" js ${ROOT_DIR} "")
	
    FIND_DEPENDENCY(ZLIB zlib.h "" "z;zlib;zlib1" ${ROOT_DIR} "D")
	
    IF(ZLIB_FOUND)
	
        FIND_DEPENDENCY(PNG png.h "" "libpng;libpng13;libpng14;libpng15;libpng16" ${ROOT_DIR} "D")
		
        IF(PNG_FOUND)
            #force subsequent FindPNG stuff not to search for other variables ... kind of a hack 
            SET(PNG_PNG_INCLUDE_DIR ${PNG_INCLUDE_DIR} CACHE FILEPATH "")
            MARK_AS_ADVANCED(PNG_PNG_INCLUDE_DIR)
        ENDIF(PNG_FOUND)
		
    ENDIF(ZLIB_FOUND)
	
ENDMACRO(SEARCH_3RDPARTY ROOT_DIR)

################################################################################################
# this is code for handling optional 3DPARTY usage (mainly under Windows)
################################################################################################

OPTION(SDEXT_USE_CUSTOM_3DPARTY "Set to ON to use 3rdParty prebuilt API located in <PROJECT_SOURCE_DIR>/../3rdparty" ON)
MARK_AS_ADVANCED(SDEXT_USE_CUSTOM_3DPARTY)

IF(SDEXT_USE_CUSTOM_3DPARTY)

    GET_FILENAME_COMPONENT(PARENT_DIR ${PROJECT_SOURCE_DIR} PATH)
    SET(SDEXT_CUSTOM_3DPARTY_DIR "${PARENT_DIR}/3rdparty" CACHE PATH 
        "Location of 3rdParty dependencies")
    IF(EXISTS ${SDEXT_CUSTOM_3DPARTY_DIR})
        SEARCH_3RDPARTY(${SDEXT_CUSTOM_3DPARTY_DIR})
    ENDIF(EXISTS ${SDEXT_CUSTOM_3DPARTY_DIR})

    # Not very useful if not Windows.
    IF(NOT WIN32)
        MARK_AS_ADVANCED(SDEXT_CUSTOM_3DPARTY_DIR)
    ENDIF(NOT WIN32)
ENDIF(SDEXT_USE_CUSTOM_3DPARTY)
