# CMakeList.txt : CMake project for FastTargaTest, include source and define
# project specific logic here.
#

# Add FastTarga Library
if(NOT TARGET FastTarga)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../FastTarga FastTarga)
endif()

# Add source to this project's executable.
add_executable (FastTargaTest "Main.cpp")

# Find SDL2
find_package(SDL2 CONFIG REQUIRED)

# Link FastTarga Library
target_link_libraries(FastTargaTest PRIVATE FastTarga)

# Link SDL2
target_link_libraries(FastTargaTest PRIVATE SDL2::SDL2 SDL2::SDL2main)

# TODO: Add tests and install targets if needed.

# Copy FastTarga Library to output directory
add_custom_command(TARGET FastTargaTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:FastTarga>
		$<TARGET_FILE_DIR:FastTargaTest>
)