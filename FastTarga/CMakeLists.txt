# CMakeList.txt : CMake project for FastTarga, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (FastTarga SHARED "TargaImage.cpp" "TargaImage.h" "TargaImageImpl.h")

# Add include directories
target_include_directories(FastTarga PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add compile definitions
target_compile_definitions(FastTarga PRIVATE FASTTARGA_EXPORTS)

# TODO: Add tests and install targets if needed.
