Building the dependencies for Speed-Dreams makes use of CMake's ExternalProject
 module. The source for each is downloaded from each project's site, patched if
 necessary, and built. This can take considerable time and accesses several
 different sites. See the thirdpartydefinitions.cmake file for the exact sites.
 The installation also contains a folder 3rdParty/source_info in which you
 will find the source link for each project.


 =============================================================================
 Windows
 As of version 2.3, this will download approximately 35MB of compressed source 
 files.
 You will need more than 1GB of free disk space for the build.

 Prerequisites:
 DirectX SDK (June 2010) - needed by SDL and possibly OpenAL
 http://www.microsoft.com/en-us/download/details.aspx?id=6812

 CMake version 3.4 or greater.

 Building the 3rd Party dependencies for Windows

 Get the code

    Using Subversion:
        svn co https://svn.code.sf.net/p/speed-dreams/code/trunk/packaging/3rdParty-devel C:\src\3rdParty-devel

    If you already have the speed-dreams code:
        copy <Path-to sd-code>\packaging\3rdParty-devel\*.* C:\src\3rdParty-devel

    Important - Keep the path short ie: C:\src\3rdParty-devel
    The building of OpenSceneGraph with CMake's ExternalProject_Add creates quite a deep tree and the build may fail. 

 CMake CLI Build

    Make a directory under the source directory (C:\src\3rdParty-devel)
        mkdir build-vs2015-release
    
    Change to the new directory:
        cd build-vs2015-release

    Generate the build system:
        cmake -G "Visual Studio 14 2015" .. -D CMAKE_BUILD_TYPE=Release -A Win32

    Build the package:
        cmake --build . --target PACKAGE --config Release

 Note:
   To build with VS2019 on Windows 7 or 8x, you may need to add -D CMAKE_SYSTEM_VERSION=10.0 to the command line:
         cmake -G "Visual Studio 16 2019" .. -D CMAKE_BUILD_TYPE=Release -D CMAKE_SYSTEM_VERSION=10.0 -A Win32

  =============================================================================
 OS X
 TODO

   =============================================================================
 Linux
 TODO



 