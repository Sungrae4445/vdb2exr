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
// - https://github.com/syoyo/tinyexr

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

#include <cstdio>
#include <cstdlib>
#include <vector>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

//#define STB_IMAGE_RESIZE_IMPLEMENTATION
//#include "stb_image_resize.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "tinyexr.h"


#include <openvdb/openvdb.h>
#include <openvdb/tools/ChangeBackground.h>


int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: vdb2exr input.vdb output.exr\n");

        exit(-1);
    }

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


    int width, height;


    // From the example above, "LevelSetSphere" is known to be a FloatGrid,
    // so cast the generic grid pointer to a FloatGrid pointer.
    openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
    // Convert the level set sphere to a narrow-band fog volume, in which
    // interior voxels have value 1, exterior voxels have value 0, and
    // narrow-band voxels have values varying linearly from 0 to 1.
    const float outside = grid->background();
    const float gwidth = 2.0 * outside;



    width = 1;
    height = 1;

    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 1;

    std::vector<float> images[1];
    images[0].resize(width * height);




    // Visit and update all of the grid's active values, which correspond to
    // voxels on the narrow band.
    for (openvdb::FloatGrid::ValueOnIter iter = grid->beginValueOn(); iter; ++iter) {
        float dist = iter.getValue();
        iter.setValue((outside - dist) / gwidth);
        // std::cout << "  dist: " << dist << " -> " << (outside - dist) / gwidth << "\n";
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

    std::cout << "  constructing image ..." << "\n";

    for (int i = 0; i < width * height; i++) {
        images[0][i] = 0.0f;
    }

    float* image_ptr[1];
    image_ptr[0] = &(images[0].at(0)); // V

    image.images = (unsigned char**)image_ptr;
    image.width = width;
    image.height = height;

    header.num_channels = 1;
    header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be BGR(A) order, since most of EXR viewers expect this channel order.
    strncpy_s(header.channels[0].name, "V", 255); header.channels[0].name[strlen("V")] = '\0';

    header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    std::cout << "  writing image ..." << "\n";

    const char* err;
    int ret = SaveEXRImageToFile(&image, &header, argv[2], &err);
    if (ret != TINYEXR_SUCCESS) {
        fprintf(stderr, "Save EXR err: %s\n", err);
        return ret;
    }
    printf("Saved exr file. [ %s ] \n", argv[2]);

    //free(rgb);

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);

    std::cout << "  done." << "\n";

    return 0;
}
