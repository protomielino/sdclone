;
;        Windows NSIS configuration file for the "HQ Cars & Tracks (2)" installer
;        (HQ tracks and cars with full AI support)
;
;        copyright  : (C) 2011 onwards Jean-Philippe Meuret
;        $Id: speed-dreams-cars-and-tracks.nsi 3306 2011-02-03 23:07:43Z pouillot $
;
;        This program is free software; you can redistribute it and/or modify
;        it under the terms of the GNU General Public License as published by
;        the Free Software Foundation; either version 2 of the License, or
;        (at your option) any later version.

; Common definitions.
!include "speed-dreams.nsh"

;SetCompressor /SOLID lzma ; 1-block compression, smaller but longer (generation, extraction)

; MUI Settings
!define MUI_WELCOMEPAGE_TITLE "${GAME_NAME}\n     ${GAME_SUBTITLE}\nHQ (2) tracks, cars and AI drivers"
!define MUI_WELCOMEPAGE_TITLE_3LINES

; Installer pages
;Page custom EnterXXXPage LeaveXXXPage
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES

; Language files (Warning: Can't be upper in the script)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "SimpChinese"

; Global installer settings.
Name "${GAME_VERSIONNED_NAME}"
Caption "Install more HQ tracks, cars & AI drivers for ${GAME_VERSIONNED_NAME}"
OutFile "speed-dreams-${MOREHQCARSTRACKS_SETUP_KEY}-${GAME_LONG_VERSION}-win32-setup.exe"
InstallDir "$PROGRAMFILES\${INST_INST_DIR_SUFFIX}"
InstallDirRegKey ${GAME_ROOT_REGKEY} "${GAME_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section -CoreData

  SetOverwrite on

  ; Game core data files (racemanagers, ...)
  ; Warning: Must be there too in case "HQ Cars & Tracks (1)" not installed.
  SetOutPath "$INSTDIR\data\config\raceman"
  File "${BUILD_INST_DIR}\data\config\raceman\career*.*"
  File "${BUILD_INST_DIR}\data\config\raceman\championship*.*"

SectionEnd

Section -Drivers

  SetOverwrite on

  ; Robots binaries
  SetOutPath "$INSTDIR\lib\drivers\simplix_trb1"
  File /r /x *.lib "${BUILD_INST_DIR}\lib\drivers\simplix_trb1\*.*"

  SetOutPath "$INSTDIR\lib\drivers\usr_trb1"
  File /r /x *.lib "${BUILD_INST_DIR}\lib\drivers\usr_trb1\*.*"

  ; Warning: Must be here because kilo2008 also drivers the TRB1 cars.
  SetOutPath "$INSTDIR\lib\drivers\kilo2008"
  File /r /x *.lib "${BUILD_INST_DIR}\lib\drivers\kilo2008\*.*"

  ; Robots data files (car and track setups)
  SetOutPath "$INSTDIR\data\drivers\simplix_trb1"
  File /r "${BUILD_INST_DIR}\data\drivers\simplix_trb1\*.*"

  SetOutPath "$INSTDIR\data\drivers\usr_trb1"
  File /r "${BUILD_INST_DIR}\data\drivers\usr_trb1\*.*"

  ; Warning: Must be here because kilo2008 also drivers the TRB1 cars.
  SetOutPath "$INSTDIR\data\drivers\kilo2008"
  File /r "${BUILD_INST_DIR}\data\drivers\kilo2008\*.*"

SectionEnd

Section -Cars

  SetOverwrite on

  ; HQ / AI driven TORCS Racing Board car models
  SetOutPath "$INSTDIR\data\cars\trb1-boxer-trbrs"
  File /r "${BUILD_INST_DIR}\data\cars\trb1-boxer-trbrs\*.*"

  SetOutPath "$INSTDIR\data\cars\trb1-cavallo-360rb"
  File /r "${BUILD_INST_DIR}\data\cars\trb1-cavallo-360rb\*.*"

  SetOutPath "$INSTDIR\data\cars\trb1-sector-rb1"
  File /r "${BUILD_INST_DIR}\data\cars\trb1-sector-rb1\*.*"

  SetOutPath "$INSTDIR\data\cars\trb1-silber-rblk"
  File /r "${BUILD_INST_DIR}\data\cars\trb1-silber-rblk\*.*"

  SetOutPath "$INSTDIR\data\cars\trb1-spirit-rb1lt"
  File /r "${BUILD_INST_DIR}\data\cars\trb1-spirit-rb1lt\*.*"

  SetOutPath "$INSTDIR\data\cars\trb1-taipan-rb1"
  File /r "${BUILD_INST_DIR}\data\cars\trb1-taipan-rb1\*.*"

  SetOutPath "$INSTDIR\data\cars\trb1-vieringe-5rb"
  File /r "${BUILD_INST_DIR}\data\cars\trb1-vieringe-5rb\*.*"

  SetOutPath "$INSTDIR\data\cars\trb1-zaxxon-rb"
  File /r "${BUILD_INST_DIR}\data\cars\trb1-zaxxon-rb\*.*"

SectionEnd

Section -Tracks

  SetOverwrite on

  ; HQ (2) circuits 
  SetOutPath "$INSTDIR\data\tracks\circuit\aalborg"
  File /r "${BUILD_INST_DIR}\data\tracks\circuit\aalborg\*.*"

  SetOutPath "$INSTDIR\data\tracks\circuit\migrants"
  File /r "${BUILD_INST_DIR}\data\tracks\circuit\migrants\*.*"

  SetOutPath "$INSTDIR\data\tracks\circuit\ruudskogen"
  File /r "${BUILD_INST_DIR}\data\tracks\circuit\ruudskogen\*.*"

  ; HQ (2) dirt tracks 
  SetOutPath "$INSTDIR\data\tracks\dirt\garguree"
  File /r "${BUILD_INST_DIR}\data\tracks\dirt\garguree\*.*"

  ; HQ (2) road tracks 
  SetOutPath "$INSTDIR\data\tracks\road\inuksuk"
  File /r "${BUILD_INST_DIR}\data\tracks\road\inuksuk\*.*"

  SetOutPath "$INSTDIR\data\tracks\road\lombaro"
  File /r "${BUILD_INST_DIR}\data\tracks\road\lombaro\*.*"

  SetOutPath "$INSTDIR\data\tracks\road\pinabashi"
  File /r "${BUILD_INST_DIR}\data\tracks\road\pinabashi\*.*"

  SetOutPath "$INSTDIR\data\tracks\road\ole-road-1"
  File /r "${BUILD_INST_DIR}\data\tracks\road\ole-road-1\*.*"

  SetOutPath "$INSTDIR\data\tracks\road\prenzlau"
  File /r "${BUILD_INST_DIR}\data\tracks\road\prenzlau\*.*"

  SetOutPath "$INSTDIR\data\tracks\road\salty"
  File /r "${BUILD_INST_DIR}\data\tracks\road\salty\*.*"

  ; HQ (2) speedways 
  SetOutPath "$INSTDIR\data\tracks\speedway\newton"
  File /r "${BUILD_INST_DIR}\data\tracks\speedway\newton\*.*"

SectionEnd

; This function is called at installer startup.
Function .onInit

  ; Check if the base package is installed and OK, and abort if not.
  !insertmacro CheckIfBasePackageInstalled

FunctionEnd

