# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jiangwenlong/Desktop/biyesheji/loti-examples

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jiangwenlong/Desktop/biyesheji/loti-examples/build

# Include any dependencies generated for this target.
include CMakeFiles/provide_buffers.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/provide_buffers.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/provide_buffers.dir/flags.make

CMakeFiles/provide_buffers.dir/provide_buffers.c.o: CMakeFiles/provide_buffers.dir/flags.make
CMakeFiles/provide_buffers.dir/provide_buffers.c.o: ../provide_buffers.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jiangwenlong/Desktop/biyesheji/loti-examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/provide_buffers.dir/provide_buffers.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/provide_buffers.dir/provide_buffers.c.o   -c /home/jiangwenlong/Desktop/biyesheji/loti-examples/provide_buffers.c

CMakeFiles/provide_buffers.dir/provide_buffers.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/provide_buffers.dir/provide_buffers.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jiangwenlong/Desktop/biyesheji/loti-examples/provide_buffers.c > CMakeFiles/provide_buffers.dir/provide_buffers.c.i

CMakeFiles/provide_buffers.dir/provide_buffers.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/provide_buffers.dir/provide_buffers.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jiangwenlong/Desktop/biyesheji/loti-examples/provide_buffers.c -o CMakeFiles/provide_buffers.dir/provide_buffers.c.s

# Object files for target provide_buffers
provide_buffers_OBJECTS = \
"CMakeFiles/provide_buffers.dir/provide_buffers.c.o"

# External object files for target provide_buffers
provide_buffers_EXTERNAL_OBJECTS =

provide_buffers: CMakeFiles/provide_buffers.dir/provide_buffers.c.o
provide_buffers: CMakeFiles/provide_buffers.dir/build.make
provide_buffers: CMakeFiles/provide_buffers.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jiangwenlong/Desktop/biyesheji/loti-examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable provide_buffers"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/provide_buffers.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/provide_buffers.dir/build: provide_buffers

.PHONY : CMakeFiles/provide_buffers.dir/build

CMakeFiles/provide_buffers.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/provide_buffers.dir/cmake_clean.cmake
.PHONY : CMakeFiles/provide_buffers.dir/clean

CMakeFiles/provide_buffers.dir/depend:
	cd /home/jiangwenlong/Desktop/biyesheji/loti-examples/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jiangwenlong/Desktop/biyesheji/loti-examples /home/jiangwenlong/Desktop/biyesheji/loti-examples /home/jiangwenlong/Desktop/biyesheji/loti-examples/build /home/jiangwenlong/Desktop/biyesheji/loti-examples/build /home/jiangwenlong/Desktop/biyesheji/loti-examples/build/CMakeFiles/provide_buffers.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/provide_buffers.dir/depend
