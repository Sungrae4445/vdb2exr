// vdb2exr.cpp : read openvd volume texture, convert to to slices, and write as openexr.
//
// === TESTCASE ===
// Convert vdb from disney to exr and import it as exr texture into Unreal Engine.
//   Testdata: https://www.disneyanimation.com/data-sets/?drawer=/resources/clouds/
//   UE4 Import and Setup Example: https://www.youtube.com/watch?v=eOrTudg7P1E
// 
// 
// === BUILDING ===
// Primary Dependencies:
// - https://github.com/AcademySoftwareFoundation/openvdb
// - https://github.com/AcademySoftwareFoundation/openexr

// Secondary Dependencies:
// - https://github.com/microsoft/vcpkg
// - https://developer.microsoft.com/de-de/windows/downloads/windows-10-sdk
// 
// === IMPLEMENTATION ===
// Tutorials this implementation is based on:
// - https://www.openvdb.org/documentation/doxygen/codeExamples.html#sModifyingGrids
// - https://www.openexr.com/documentation/ReadingAndWritingImageFiles.pdf
//
// === CONFIGURATION ===
// Setup: Windows 10, VS2019, Release/x64 Configuration:
//   C++: Multithreaded (/MT)
//   Addional Include Paths:
//       ...\openvdb\openvdb
//       ...\vcpkg\installed\x64 - windows\include
//   Addional Library Paths:
//       ...\openvdb\build\openvdb\openvdb\Release
//       ...\vcpkg\installed\x64 - windows\lib
//   Linker Input:
//       ...\openvdb\build\openvdb\openvdb\Release\openvdb.lib
//       ...\vcpkg\installed\x64 - windows\lib\Half - 2_5.lib
//       ...\vcpkg\installed\x64-windows\lib\tbb.lib
//

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>

#include <openvdb/openvdb.h>
#include <openvdb/tools/ChangeBackground.h>




int main()
{
    openvdb::initialize();
    // Create a shared pointer to a newly-allocated grid of a built-in type:
    // in this case, a FloatGrid, which stores one single-precision floating point
    // value per voxel.  Other built-in grid types include BoolGrid, DoubleGrid,
    // Int32Grid and Vec3SGrid (see openvdb.h for the complete list).
    // The grid comprises a sparse tree representation of voxel data,
    // user-supplied metadata and a voxel space to world space transform,
    // which defaults to the identity transform.


    // Create a VDB file object.
    std::cout << "Reading from "<< "..\\Testdata\\wdas_cloud_sixteenth.vdb" << "\n";
    openvdb::io::File file("..\\Testdata\\wdas_cloud_sixteenth.vdb");
    // Open the file.  This reads the file header, but not any grids.
    file.open();
    // Loop over all grids in the file and retrieve a shared pointer
    // to the one named "LevelSetSphere".  (This can also be done
    // more simply by calling file.readGrid("LevelSetSphere").)
    openvdb::GridBase::Ptr baseGrid;
    for (openvdb::io::File::NameIterator nameIter = file.beginName();
        nameIter != file.endName(); ++nameIter)
    {
        std::cout << " reading grid " << nameIter.gridName() << "\n";

        // Read in only the grid we are interested in.
        if (nameIter.gridName() == "density") {
            baseGrid = file.readGrid(nameIter.gridName());
        }
        else {
            std::cout << "skipping grid " << nameIter.gridName() << std::endl;
        }
    }
    file.close();

    std::cout << "Reading completed." << "\n";

    // From the example above, "LevelSetSphere" is known to be a FloatGrid,
    // so cast the generic grid pointer to a FloatGrid pointer.
    openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
    // Convert the level set sphere to a narrow-band fog volume, in which
    // interior voxels have value 1, exterior voxels have value 0, and
    // narrow-band voxels have values varying linearly from 0 to 1.
    const float outside = grid->background();
    const float width = 2.0 * outside;

    // Visit and update all of the grid's active values, which correspond to
    // voxels on the narrow band.
    for (openvdb::FloatGrid::ValueOnIter iter = grid->beginValueOn(); iter; ++iter) {
        float dist = iter.getValue();
        iter.setValue((outside - dist) / width);
        std::cout << "  dist: " << dist << " -> " << (outside - dist) / width << "\n";
    }

    // Visit all of the grid's inactive tile and voxel values and update the values
    // that correspond to the interior region.
    for (openvdb::FloatGrid::ValueOffIter iter = grid->beginValueOff(); iter; ++iter) {
        if (iter.getValue() < 0.0) {
            iter.setValue(1.0);
            iter.setValueOff();
        }
    }
    // Set exterior voxels to 0.
    openvdb::tools::changeBackground(grid->tree(), 0.0);
    std::cout << "  done." << "\n";

}
