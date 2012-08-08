################################################################################################
# Find a generic dependency, handling debug suffix
# all the parameters are required ; in case of lists or empty parameter, use "" when calling

MACRO(_FIND_3RDPARTY_DEPENDENCY DEP_NAME INCLUDE_FILE INCLUDE_SUBDIRS LIBRARY_NAMES SEARCH_PATH_LIST DEBUG_SUFFIX)

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
    #MESSAGE(STATUS " ${DEP_NAME}_INCLUDE_DIR = '${${DEP_NAME}_INCLUDE_DIR}'")
	
	# Find library files
    SET(MY_PATH_LIB )
    FOREACH(MY_PATH ${SEARCH_PATH_LIST} )
		SET(MY_PATH_LIB ${MY_PATH_LIB} ${MY_PATH}/lib)
    ENDFOREACH(MY_PATH ${SEARCH_PATH_LIST} )
    
	#MESSAGE(STATUS "LIBRARY_NAMES='${LIBRARY_NAMES}', MY_PATH_LIB=${MY_PATH_LIB}")
    FIND_LIBRARY("${DEP_NAME}_LIBRARY" 
      NAMES ${LIBRARY_NAMES}
      PATHS ${MY_PATH_LIB}
      NO_DEFAULT_PATH
    )
    MARK_AS_ADVANCED("${DEP_NAME}_LIBRARY")
    #MESSAGE(STATUS " ${DEP_NAME}_LIBRARY = '${${DEP_NAME}_LIBRARY}'")

	# Whatever happened, done.
    SET(${DEP_NAME}_FOUND "NO" )
    IF(${DEP_NAME}_INCLUDE_DIR AND ${DEP_NAME}_LIBRARY)
      SET( ${DEP_NAME}_FOUND "YES" )
    ENDIF(${DEP_NAME}_INCLUDE_DIR AND ${DEP_NAME}_LIBRARY)

ENDMACRO(_FIND_3RDPARTY_DEPENDENCY DEP_NAME INCLUDE_FILE INCLUDE_SUBDIRS LIBRARY_NAMES SEARCH_PATH_LIST DEBUG_SUFFIX)


MACRO(_FIND_3RDPARTY_DEPENDENCIES ROOT_DIR)

    _FIND_3RDPARTY_DEPENDENCY(SDL sdl.h "SDL;SDL2" sdl "${ROOT_DIR}" "")
    _FIND_3RDPARTY_DEPENDENCY(SDLMAIN sdl_main.h "SDL;SDL2" sdlmain "${ROOT_DIR}" "")
	IF(SDL_FOUND) # Dirty hack to make FindPackage(SDL) work later.
		SET(SDL_LIBRARY_TEMP ${SDL_LIBRARY} CACHE FILEPATH "")
	ENDIF(SDL_FOUND)

    _FIND_3RDPARTY_DEPENDENCY(PLIB plib/sg.h "" sg ${ROOT_DIR} "")
    _FIND_3RDPARTY_DEPENDENCY(PLIB_SSG plib/ssg.h "" ssg ${ROOT_DIR} "")
    _FIND_3RDPARTY_DEPENDENCY(PLIB_SG plib/sg.h "" sg ${ROOT_DIR} "")
    _FIND_3RDPARTY_DEPENDENCY(PLIB_SL plib/sl.h "" sl ${ROOT_DIR} "")
    _FIND_3RDPARTY_DEPENDENCY(PLIB_SSGAUX plib/ssgaux.h "" ssgaux ${ROOT_DIR} "")
    _FIND_3RDPARTY_DEPENDENCY(PLIB_UL plib/ul.h "" ul ${ROOT_DIR} "")
    _FIND_3RDPARTY_DEPENDENCY(PLIB_JS plib/js.h "" js ${ROOT_DIR} "")
	
    # Note: The Open GL includes are automatically added by MSVC 2005.
    # We simply add here the include path for the Open GL extensions headers,
    # and we use OPENGL_INCLUDE_DIR variable for this,
    # as Find_Package(OpenGL) doesn't seem to set it.
	Find_Package(OpenGL)
    FIND_PATH(OPENGL_INCLUDE_DIR GL/glext.h ${ROOT_DIR}/include NO_DEFAULT_PATH)

    _FIND_3RDPARTY_DEPENDENCY(OPENAL AL/al.h "" openal32 ${ROOT_DIR} "")
	
    _FIND_3RDPARTY_DEPENDENCY(ENET enet/enet.h "" enet ${ROOT_DIR} "")
	
	IF(OPTION_3RDPARTY_EXPAT)
		_FIND_3RDPARTY_DEPENDENCY(EXPAT expat.h "" expat ${ROOT_DIR} "")
	ENDIF(OPTION_3RDPARTY_EXPAT)
    
	IF(OPTION_3RDPARTY_SOLID)

		_FIND_3RDPARTY_DEPENDENCY(SOLID SOLID/solid.h "" "solid;broad" ${ROOT_DIR} "")
		_FIND_3RDPARTY_DEPENDENCY(SOLID_SOLID SOLID/solid.h "" "solid" ${ROOT_DIR} "")
		_FIND_3RDPARTY_DEPENDENCY(SOLID_BROAD SOLID/broad.h "" "broad" ${ROOT_DIR} "")

        IF(SOLID_FOUND)
            # Force subsequent FindSOLID stuff not to search for other variables ... kind of a hack 
            SET(SOLID_SOLIDINCLUDE_DIR ${SOLID_INCLUDE_DIR} CACHE FILEPATH "")
            MARK_AS_ADVANCED(SOLID_SOLIDINCLUDE_DIR)
        ENDIF(SOLID_FOUND)

	ENDIF(OPTION_3RDPARTY_SOLID)
    
    _FIND_3RDPARTY_DEPENDENCY(JPEG jpeglib.h "" jpeg_s ${ROOT_DIR} "")
    
    _FIND_3RDPARTY_DEPENDENCY(ZLIB zlib.h "" "z;zlib;zlib1" ${ROOT_DIR} "D")
	
    IF(ZLIB_FOUND)
	
        _FIND_3RDPARTY_DEPENDENCY(PNG png.h "" "libpng;libpng13;libpng14;libpng15;libpng16" ${ROOT_DIR} "D")
		
        IF(PNG_FOUND)
            # Force subsequent FindPNG stuff not to search for other variables ... kind of a hack 
            SET(PNG_PNG_INCLUDE_DIR ${PNG_INCLUDE_DIR} CACHE FILEPATH "")
            MARK_AS_ADVANCED(PNG_PNG_INCLUDE_DIR)
        ENDIF(PNG_FOUND)
		
    ENDIF(ZLIB_FOUND)
	
ENDMACRO(_FIND_3RDPARTY_DEPENDENCIES ROOT_DIR)

################################################################################################
# Handling of optional 3rd party package (usefull only when building under Windows with MSVC)

MACRO(SD_FIND_3RDPARTY)

	OPTION(SDEXT_USE_CUSTOM_3DPARTY "Set to ON to use 3rdParty prebuilt API located in <PROJECT_SOURCE_DIR>/../3rdparty" ON)
	MARK_AS_ADVANCED(SDEXT_USE_CUSTOM_3DPARTY)

	IF(SDEXT_USE_CUSTOM_3DPARTY)

	    GET_FILENAME_COMPONENT(PARENT_DIR ${PROJECT_SOURCE_DIR} PATH)
    	SET(SDEXT_CUSTOM_3DPARTY_DIR "${PARENT_DIR}/3rdparty" CACHE PATH 
        							 "Location of 3rdParty dependencies")
	    IF(EXISTS ${SDEXT_CUSTOM_3DPARTY_DIR})
        	_FIND_3RDPARTY_DEPENDENCIES(${SDEXT_CUSTOM_3DPARTY_DIR})
		ENDIF(EXISTS ${SDEXT_CUSTOM_3DPARTY_DIR})

    	MARK_AS_ADVANCED(SDEXT_CUSTOM_3DPARTY_DIR)

	ENDIF(SDEXT_USE_CUSTOM_3DPARTY)

ENDMACRO(SD_FIND_3RDPARTY)

################################################################################################
# Under Windows, install needed 3rd party DLLs close to Speed Dreams executable
# (but stay compatible with the old 2.0.0 3rd party package which had less DLLs inside)

MACRO(_FIND_3RDPARTY_DLL PACKAGE_NAME LINK_LIBRARY NAME_HINTS DLL_PATHNAME_VAR)

    FIND_PACKAGE(${PACKAGE_NAME})

	FOREACH(_LIB_NAME ${NAME_HINTS})

		# Must handle the case of multiple libs listed in ${LINK_LIBRARY}
		SET(_LIB_PATHNAME ${LINK_LIBRARY})
		FOREACH(_LIB_PATHNAME_ ${LINK_LIBRARY})
			IF(${_LIB_PATHNAME_} MATCHES "${_LIB_NAME}\\.")
				SET(_LIB_PATHNAME ${_LIB_PATHNAME_})
				BREAK()
			ENDIF(${_LIB_PATHNAME_} MATCHES "${_LIB_NAME}\\.")
		ENDFOREACH(_LIB_PATHNAME_ ${LINK_LIBRARY})

		# Got 1 link library pathname : check if any corresponding DLL around.
    	GET_FILENAME_COMPONENT(_LIB_PATH "${_LIB_PATHNAME}" PATH)
		SET(${DLL_PATHNAME_VAR} "${_LIB_PATH}/../bin/${_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		#MESSAGE(STATUS "Trying 3rdParty DLL ${${DLL_PATHNAME_VAR}} for ${PACKAGE_NAME}")
		IF(NOT EXISTS "${${DLL_PATHNAME_VAR}}")
			SET(_LIB_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}${_LIB_NAME}")
			SET(${DLL_PATHNAME_VAR} "${_LIB_PATH}/../bin/${_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
			#MESSAGE(STATUS "Trying 3rdParty DLL ${${DLL_PATHNAME_VAR}} for ${PACKAGE_NAME}")
		ENDIF(NOT EXISTS "${${DLL_PATHNAME_VAR}}")
		#MESSAGE(STATUS "XX ${${DLL_PATHNAME_VAR}} <= ${LINK_LIBRARY} : ${_LIB_NAME} in ${_LIB_PATH}")
		IF(EXISTS "${${DLL_PATHNAME_VAR}}")
			#MESSAGE(STATUS "Found 3rdParty DLL ${${DLL_PATHNAME_VAR}} for ${PACKAGE_NAME}")
			BREAK()
		ELSE(EXISTS "${${DLL_PATHNAME_VAR}}")
			UNSET(${DLL_PATHNAME_VAR})
		ENDIF(EXISTS "${${DLL_PATHNAME_VAR}}")

	ENDFOREACH(_LIB_NAME ${NAME_HINTS})

	IF(NOT ${_DLL_PATHNAME_VAR})
		#MESSAGE(STATUS "Could not find 3rdParty DLL in ${NAME_HINTS} for ${PACKAGE_NAME}")
	ENDIF(NOT ${_DLL_PATHNAME_VAR})

ENDMACRO(_FIND_3RDPARTY_DLL DLL_PATHNAME)

MACRO(SD_INSTALL_3RDPARTY)

    SET(_THIRDPARTY_DLL_PATHNAMES)

    _FIND_3RDPARTY_DLL("OpenAL" "${OPENAL_LIBRARY}" "OpenAL32" _DLL_PATHNAME)
	LIST(APPEND _THIRDPARTY_DLL_PATHNAMES "${_DLL_PATHNAME}")

    _FIND_3RDPARTY_DLL("SDL" "${SDL_LIBRARY}" "SDL" _DLL_PATHNAME)
	LIST(APPEND _THIRDPARTY_DLL_PATHNAMES "${_DLL_PATHNAME}")

	IF(OPTION_3RDPARTY_EXPAT)

		_FIND_3RDPARTY_DLL("EXPAT" "${EXPAT_LIBRARY}" "expat;expat-1" _DLL_PATHNAME)
		LIST(APPEND _THIRDPARTY_DLL_PATHNAMES "${_DLL_PATHNAME}")

	ENDIF(OPTION_3RDPARTY_EXPAT)

	IF(OPTION_3RDPARTY_SOLID)

		_FIND_3RDPARTY_DLL("SOLID" "${SOLID_SOLID_LIBRARY}" "solid" _DLL_PATHNAME)
		LIST(APPEND _THIRDPARTY_DLL_PATHNAMES "${_DLL_PATHNAME}")

		_FIND_3RDPARTY_DLL("SOLID" "${SOLID_BROAD_LIBRARY}" "broad" _DLL_PATHNAME)
		LIST(APPEND _THIRDPARTY_DLL_PATHNAMES "${_DLL_PATHNAME}")

	ENDIF(OPTION_3RDPARTY_SOLID)

    _FIND_3RDPARTY_DLL("ZLIB" "${ZLIB_LIBRARY}" "zlib" _DLL_PATHNAME)
	LIST(APPEND _THIRDPARTY_DLL_PATHNAMES "${_DLL_PATHNAME}")

    _FIND_3RDPARTY_DLL("PNG" "${PNG_LIBRARY}" "png" _DLL_PATHNAME)
	LIST(APPEND _THIRDPARTY_DLL_PATHNAMES "${_DLL_PATHNAME}")

    _FIND_3RDPARTY_DLL("JPEG" "${JPEG_LIBRARY}" "jpeg-8" _DLL_PATHNAME)
	LIST(APPEND _THIRDPARTY_DLL_PATHNAMES "${_DLL_PATHNAME}")

	#MESSAGE(STATUS "3rdParty dependencies : Will install ${_THIRDPARTY_DLL_PATHNAMES}")
	SD_INSTALL_FILES(BIN FILES ${_THIRDPARTY_DLL_PATHNAMES})

	# Make sure Windows compilers run-time libs are also installed.
	IF(MSVC)
		# We do it ourselves, but use InstallRequiredSystemLibraries to figure out what they are
		SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
		INCLUDE(InstallRequiredSystemLibraries)
		IF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
			SD_INSTALL_FILES(BIN FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
		ENDIF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
	ELSEIF(MINGW)
		# Works with MinGW 4.4 and 4.7.
		GET_FILENAME_COMPONENT(_MINGW_BINDIR "${CMAKE_CXX_COMPILER}" PATH)
		SD_INSTALL_FILES(BIN FILES "${_MINGW_BINDIR}/libstdc++-6.dll" "${_MINGW_BINDIR}/libgcc_s_dw2-1.dll")
	ENDIF(MSVC)

ENDMACRO(SD_INSTALL_3RDPARTY)
