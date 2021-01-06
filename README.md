# An Openvdb to openexr converter for sliced otextures to be imported in Unreal Engine.

![Preview](https://olir/vdb2exr/blob/master/Testdata/Preview-sixteenth-eighth-quarter.JPG?raw=true)

## === TESTCASE ===

Convert vdb from disney to exr and import it as exr texture into Unreal Engine.
  Testdata: Walt Disney Animation Studios Samples from Cloud Data Set
Source: https://www.disneyanimation.com/data-sets/?drawer=/resources/clouds/
  UE4 Import and Setup Example: https://www.youtube.com/watch?v=eOrTudg7P1E

## === BUILDING / Content ===

Primary Dependencies:

- https://github.com/AcademySoftwareFoundation/openvdb
- https://github.com/syoyo/tinyexr
- https://github.com/AcademySoftwareFoundation/openexr

Secondary Dependencies:
- https://github.com/microsoft/vcpkg
- https://developer.microsoft.com/de-de/windows/downloads/windows-10-sdk

## === IMPLEMENTATION ===

Tutorials this implementation is based on:

- https://www.openvdb.org/documentation/doxygen/codeExamples.html#sModifyingGrids

- https://www.openexr.com/documentation/ReadingAndWritingImageFiles.pdf

  ## === CONFIGURATION ===

  Setup: Windows 10, VS2019, Release/x64 Configuration:
  C++: Multithreaded (/MT)
  Addional Include Paths:
      ...\openvdb\openvdb
      ...\vcpkg\installed\x64 - windows\include
  Addional Library Paths:
      ...\openvdb\build\openvdb\openvdb\Release
      ...\vcpkg\installed\x64 - windows\lib
  Linker Input:
      ...\openvdb\build\openvdb\openvdb\Release\openvdb.lib
      ...\vcpkg\installed\x64 - windows\lib\Half - 2_5.lib
      ...\vcpkg\installed\x64-windows\lib\tbb.lib
  

## LICENSING:

The contents of this package are Copyright 2021 Oliver Rode and are licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License. A copy of this license is available at http://creativecommons.org/licenses/by-sa/3.0/. 

Binary Content in Testdata and 3RDPARTY folder are redistributed under it's own conditions as described in license and readme files contained in these folders.