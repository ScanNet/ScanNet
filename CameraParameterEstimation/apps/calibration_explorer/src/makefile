# Executable name
EXE_NAME = calibration_explorer

# Useful Directories
BIN_DIR = ../../../bin/
LIB_DIR = /usr/local/lib/
OUT_DIR = ../out/

BSC_DIR  = ../../basics/
IMGUI_DIR = $(BSC_DIR)extern/imgui/

# List of source files
SRCS = calibration_explorer.cpp $(IMGUI_DIR)imgui.cpp $(IMGUI_DIR)imgui_draw.cpp
OBJS = $(SRCS:%.cpp=$(OUT_DIR)%.o)

# Compile and link options
CC = g++
WARNINGS = -Wall
CPPFLAGS = -std=c++11 -O -I. -I/usr/local/include  -I$(IMGUI_DIR) -I$(BSC_DIR) 

# Libraries
LIBS = -L$(LIB_DIR) -lglew -lglfw3 

# Frameworks
FRAMEWORKS = -framework opengl 

# Make targets
all: clean calibration_explorer

calibration_explorer: $(OBJS)
		$(CC) $(FRAMEWORKS) $(CPPFLAGS) $(OBJS) $(LIBS) -o ${BIN_DIR}${EXE_NAME}

clean:
		rm -f $(OUT_DIR)*.a $(OUT_DIR)*.o ${BIN_DIR}${EXE_NAME} ${BIN_DIR}${EXE_NAME}.exe


# Compile command
$(OUT_DIR)%.o: %.cpp
		$(CC) $(WARNINGS) $(CPPFLAGS) -c $< -o $@


# GNU Make: targets that don't build files
.PHONY: all clean
