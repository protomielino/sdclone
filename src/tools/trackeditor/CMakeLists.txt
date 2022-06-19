INCLUDE(../../../cmake/macros.cmake)

FIND_PACKAGE(Java)
FIND_PACKAGE(Java COMPONENTS Development)
IF(Java_Development_FOUND AND Java_FOUND)
   INCLUDE(UseJava)


   PROJECT(trackeditor NONE)

   SET(_TARGET_NAME ${SD_TOOLS_EXECPREFIX}trackeditor)

   SET(_JAVA_SRC 
         gui/DeltaPanel.java
         gui/EditorFrame.java
         gui/NewProjectDialog.java
         gui/TrackgenPanel.java
         gui/properties/CameraProperties.java
         gui/properties/EnvMapProperties.java
         gui/properties/GeneralProperties.java
         gui/properties/GraphicProperties.java
         gui/properties/ImageProperties.java
         gui/properties/LocalInfoProperties.java
         gui/properties/ObjectProperties.java
         gui/properties/PitProperties.java
         gui/properties/PropertiesDialog.java
         gui/properties/PropertyPanel.java
         gui/properties/StartingGridProperties.java
         gui/properties/SurfaceProperties.java
         gui/properties/TerrainProperties.java
         gui/properties/TrackLightProperties.java
         gui/properties/TrackProperties.java
         gui/properties/TurnMarksProperties.java
         gui/segment/GroupButton.java
         gui/segment/ProfileButton.java
         gui/segment/SegmentComboBox.java
         gui/segment/SegmentEditorDlg.java
         gui/segment/SegmentSideProperties.java
         gui/segment/SegmentSlider.java
         gui/segment/SliderListener.java
         gui/splash/SplashPanel.java
         gui/splash/SplashScreen.java
         gui/splash/StatusBar.java
         gui/view/CircuitView.java
         gui/view/CircuitViewSelectionEvent.java
         gui/view/CircuitViewSelectionListener.java
         miscel/EPMath.java
         miscel/TorcsTuneFileFilter.java
         plugin/Plugin.java
         plugin/torcs/NoOpEntityResolver.java
         plugin/torcs/TorcsPlugin.java
         plugin/torcs/XMLOutput.java
         plugin/torcs/XmlReader.java
         plugin/torcs/XmlWriter.java
         utils/CustomFileFilter.java
         utils/Editor.java
         utils/EditorPoint.java
         utils/GroupButtonLayout.java
         utils/Project.java
         utils/Properties.java
         utils/SegmentSliderLayout.java
         utils/TrackData.java
         utils/TrackEditor.java
         utils/circuit/Camera.java
         utils/circuit/Curve.java
         utils/circuit/EnvironmentMapping.java
         utils/circuit/Graphic.java
         utils/circuit/Header.java
         utils/circuit/LocalInfo.java
         utils/circuit/MainTrack.java
         utils/circuit/ObjShapeHandle.java
         utils/circuit/ObjShapeTerrain.java
         utils/circuit/ObjectMap.java
         utils/circuit/Pits.java
         utils/circuit/Segment.java
         utils/circuit/SegmentListener.java
         utils/circuit/SegmentSide.java
         utils/circuit/SegmentSideListener.java
         utils/circuit/StartingGrid.java
         utils/circuit/Straight.java
         utils/circuit/Surface.java
         utils/circuit/TerrainGeneration.java
         utils/circuit/TrackLight.java
         utils/circuit/TrackObject.java
         utils/circuit/TurnMarks.java
         utils/circuit/XmlObjPits.java
         utils/undo/Undo.java
         utils/undo/UndoAddSegment.java
         utils/undo/UndoDeleteSegment.java
         utils/undo/UndoInterface.java
         utils/undo/UndoSegmentChange.java)

   #file(GLOB_RECURSE _JAVA_SRC ./*.java)
   #foreach(_jfile ${_JAVA_SRC})
   #   message(STATUS "${_jfile}")
   #endforeach()

   SET(_PNGS gui/splash/Splash.png icon.png)
   #file(GLOB_RECURSE _PNGS *.png)

   SET(_ICONS gui/icons/Calc24.gif
            gui/icons/Cut24.gif
            gui/icons/Export24.gif
            gui/icons/FindAgain24.gif
            gui/icons/Help24.gif
            gui/icons/Movie24.gif
            gui/icons/New24.gif
            gui/icons/Open24.gif
            gui/icons/Redo24.gif
            gui/icons/Save24.gif
            gui/icons/Search24.gif
            gui/icons/Straight24.gif
            gui/icons/Subdivide24.gif
            gui/icons/TurnLeft24.gif
            gui/icons/TurnRight24.gif
            gui/icons/Undo24.gif
            gui/icons/Zoom24.gif
            gui/icons/ZoomIn24.gif
            gui/icons/ZoomOut24.gif
   )

   SET(JAVA_JARS bsh-2.0b4.jar
                  jdom-1.1.3.jar
                  looks-1.2.2.jar )

   # ========= Debug
   #message(STATUS "JAVA_JARS = ${JAVA_JARS}")
   #message(STATUS "_JAVA_SRC = ${_JAVA_SRC}")
   #message(STATUS "_PNGS = ${_PNGS}")
   #message(STATUS "_ICONS = ${_ICONS}")
   # ========= Debug

   SET(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.7" "-target" "1.7")

   SET(_TGT_DIR "${CMAKE_BINARY_DIR}/${SD_BINDIR}")

   ADD_JAR(${_TARGET_NAME} 
            SOURCES ${_JAVA_SRC} gui/splash/Splash.png icon.png ${_ICONS} 
            INCLUDE_JARS ${JAVA_JARS}
            MANIFEST ./META-INF/MANIFEST.MF
            OUTPUT_DIR  ${_TGT_DIR}
            )

   # Copy each jar dependency to the bin folder (for running without installing).
      ADD_CUSTOM_COMMAND(TARGET ${_TARGET_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E make_directory "${_TGT_DIR}"
                        VERBATIM)
      FOREACH(_JAR ${JAVA_JARS})
         ADD_CUSTOM_COMMAND(TARGET ${_TARGET_NAME} POST_BUILD
                              COMMAND ${CMAKE_COMMAND} -E echo Copying "${_JAR}" to "${_TGT_DIR}"
                              COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/${_JAR}" "${_TGT_DIR}"
                              VERBATIM)
      ENDFOREACH()


   INSTALL_JAR(${_TARGET_NAME} DESTINATION ${SD_BINDIR})

   # install the jar dependencies
   SD_INSTALL_FILES(BIN FILES ${JAVA_JARS})

   # ========= Debug
   #get_target_property(_jarFile TrackEditor JAR_FILE)
   #get_target_property(_classDir TrackEditor CLASSDIR)

   #message(STATUS "Jar file ${_jarFile}")
   #message(STATUS "Class compiled to ${_classDir}")
   # ========= Debug


   # add_custom_target() makes the java sources available in the generated IDE project
   ADD_CUSTOM_TARGET(${_TARGET_NAME}-src SOURCES ${_JAVA_SRC} ${_PNGS} ${_ICONS} META-INF/MANIFEST.MF)

   # https://github.com/beanshell/beanshell/releases
   # http://www.jdom.org/dist/binary/archive/jdom-1.1.3.zip
   # http://www.java2s.com/Code/JarDownload/looks/looks-1.2.2.jar.zip

ELSE(Java_Development_FOUND AND Java_FOUND)
   MESSAGE(WARNING "Java_FOUND = ${Java_FOUND}")
   MESSAGE(WARNING "Java_Development_FOUND = ${Java_Development_FOUND}")
   MESSAGE(WARNING "TrackEditor NOT included!")
ENDIF(Java_Development_FOUND AND Java_FOUND)