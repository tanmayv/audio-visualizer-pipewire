# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tanmay/recording/audio-visualizer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tanmay/recording/audio-visualizer/build

# Include any dependencies generated for this target.
include CMakeFiles/AudioVisualizer.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/AudioVisualizer.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/AudioVisualizer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/AudioVisualizer.dir/flags.make

CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o: CMakeFiles/AudioVisualizer.dir/flags.make
CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o: /home/tanmay/recording/audio-visualizer/src/audio-stream.cc
CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o: CMakeFiles/AudioVisualizer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tanmay/recording/audio-visualizer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o -MF CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o.d -o CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o -c /home/tanmay/recording/audio-visualizer/src/audio-stream.cc

CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tanmay/recording/audio-visualizer/src/audio-stream.cc > CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.i

CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tanmay/recording/audio-visualizer/src/audio-stream.cc -o CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.s

CMakeFiles/AudioVisualizer.dir/src/main.cc.o: CMakeFiles/AudioVisualizer.dir/flags.make
CMakeFiles/AudioVisualizer.dir/src/main.cc.o: /home/tanmay/recording/audio-visualizer/src/main.cc
CMakeFiles/AudioVisualizer.dir/src/main.cc.o: CMakeFiles/AudioVisualizer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tanmay/recording/audio-visualizer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/AudioVisualizer.dir/src/main.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/AudioVisualizer.dir/src/main.cc.o -MF CMakeFiles/AudioVisualizer.dir/src/main.cc.o.d -o CMakeFiles/AudioVisualizer.dir/src/main.cc.o -c /home/tanmay/recording/audio-visualizer/src/main.cc

CMakeFiles/AudioVisualizer.dir/src/main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/AudioVisualizer.dir/src/main.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tanmay/recording/audio-visualizer/src/main.cc > CMakeFiles/AudioVisualizer.dir/src/main.cc.i

CMakeFiles/AudioVisualizer.dir/src/main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/AudioVisualizer.dir/src/main.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tanmay/recording/audio-visualizer/src/main.cc -o CMakeFiles/AudioVisualizer.dir/src/main.cc.s

# Object files for target AudioVisualizer
AudioVisualizer_OBJECTS = \
"CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o" \
"CMakeFiles/AudioVisualizer.dir/src/main.cc.o"

# External object files for target AudioVisualizer
AudioVisualizer_EXTERNAL_OBJECTS =

AudioVisualizer: CMakeFiles/AudioVisualizer.dir/src/audio-stream.cc.o
AudioVisualizer: CMakeFiles/AudioVisualizer.dir/src/main.cc.o
AudioVisualizer: CMakeFiles/AudioVisualizer.dir/build.make
AudioVisualizer: CMakeFiles/AudioVisualizer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/tanmay/recording/audio-visualizer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable AudioVisualizer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/AudioVisualizer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/AudioVisualizer.dir/build: AudioVisualizer
.PHONY : CMakeFiles/AudioVisualizer.dir/build

CMakeFiles/AudioVisualizer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/AudioVisualizer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/AudioVisualizer.dir/clean

CMakeFiles/AudioVisualizer.dir/depend:
	cd /home/tanmay/recording/audio-visualizer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tanmay/recording/audio-visualizer /home/tanmay/recording/audio-visualizer /home/tanmay/recording/audio-visualizer/build /home/tanmay/recording/audio-visualizer/build /home/tanmay/recording/audio-visualizer/build/CMakeFiles/AudioVisualizer.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/AudioVisualizer.dir/depend

