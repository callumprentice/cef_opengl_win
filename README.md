Description:
============
A simple example for Windows that uses the Chroimium Embedded Framework (CEF) (https://bitbucket.org/chromiumembedded/cef) directly to render web content to an OpenGL buffer and display it. This example is very rudimentary and designed as a place to test ideas and explore blocking issues.

For a more complex implementation including a standalone wrapper for CEF as well as more interesting examples, you might want to check out a project called Dullahan (https://bitbucket.org/lindenlab/dullahan) 

Prerequisites:
==============
* Visual Studio 12 2013 Update 5
* CMake 3.1 or later (3.4.3 or later is better)
* A tool to uncompreess .bz2 files like 7-Zip via http://www.7-zip.org/download.html

Building libcef_dll_wrapper
============================
* Visit Spotify open source page of automated builds at http://opensource.spotify.com/cefbuilds/index.html 
* Download the "Standard Distribution" version of the Windows 64 build you want to use.
* Uncompress it
* Open a Visual Studio 12 2013 Command Prompt
* change to directory you uncompressed
* `mkdir build` #important - use this name as it's referenced in the CMake file for the application
* `cd build`
* `cmake -G "Visual Studio 12 2013 Win64" ..`
* `msbuild cef.sln /t:libcef_dll_wrapper /p:Configuration=Debug /p:Platform=x64`
* `msbuild cef.sln /t:libcef_dll_wrapper /p:Configuration=Release /p:Platform=x64`

Building the cef_opengl_win application 
========================================
* Open a Visual Studio 12 2013 Command Prompt
* change to directory where this file is located
* `mkdir build`
* `cd build`
* `cmake -G "Visual Studio 12 2013 Win64" -DCEF_BUILD_DIR="C:\work\cef_builds\cef_binary_3.3239.1706.gcd33baa_windows64" ..`
  (replacing the directory name with the CEF directory you just built)
* `start cef_opengl_win.sln`
* If you have a recent (>3.4.3) version of CMake, the cef_opengl_win project is already selected as the Startup Project. If not, select it manually yourself
* Build and run the application

Notes
=====
* Instructions are for the 64bit version. Make some simple changes to use the 32 bit version instead (Grab a 32 bit CEF build from the Spotify site, remove `Win64` tag on CMake generator and use `/p:Platform=Win32` for the msbuild parameter instead of `/p:Platform=x64`)