# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/vladimirkovesnikov/Study/tftds/hw1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/vladimirkovesnikov/Study/tftds/hw1

# Include any dependencies generated for this target.
include CMakeFiles/worker.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/worker.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/worker.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/worker.dir/flags.make

CMakeFiles/worker.dir/codegen:
.PHONY : CMakeFiles/worker.dir/codegen

CMakeFiles/worker.dir/worker.cpp.o: CMakeFiles/worker.dir/flags.make
CMakeFiles/worker.dir/worker.cpp.o: worker.cpp
CMakeFiles/worker.dir/worker.cpp.o: CMakeFiles/worker.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/vladimirkovesnikov/Study/tftds/hw1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/worker.dir/worker.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/worker.dir/worker.cpp.o -MF CMakeFiles/worker.dir/worker.cpp.o.d -o CMakeFiles/worker.dir/worker.cpp.o -c /Users/vladimirkovesnikov/Study/tftds/hw1/worker.cpp

CMakeFiles/worker.dir/worker.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/worker.dir/worker.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vladimirkovesnikov/Study/tftds/hw1/worker.cpp > CMakeFiles/worker.dir/worker.cpp.i

CMakeFiles/worker.dir/worker.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/worker.dir/worker.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vladimirkovesnikov/Study/tftds/hw1/worker.cpp -o CMakeFiles/worker.dir/worker.cpp.s

# Object files for target worker
worker_OBJECTS = \
"CMakeFiles/worker.dir/worker.cpp.o"

# External object files for target worker
worker_EXTERNAL_OBJECTS =

worker: CMakeFiles/worker.dir/worker.cpp.o
worker: CMakeFiles/worker.dir/build.make
worker: CMakeFiles/worker.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/vladimirkovesnikov/Study/tftds/hw1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable worker"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/worker.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/worker.dir/build: worker
.PHONY : CMakeFiles/worker.dir/build

CMakeFiles/worker.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/worker.dir/cmake_clean.cmake
.PHONY : CMakeFiles/worker.dir/clean

CMakeFiles/worker.dir/depend:
	cd /Users/vladimirkovesnikov/Study/tftds/hw1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/vladimirkovesnikov/Study/tftds/hw1 /Users/vladimirkovesnikov/Study/tftds/hw1 /Users/vladimirkovesnikov/Study/tftds/hw1 /Users/vladimirkovesnikov/Study/tftds/hw1 /Users/vladimirkovesnikov/Study/tftds/hw1/CMakeFiles/worker.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/worker.dir/depend
