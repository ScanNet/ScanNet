# Executable name
EXE_NAME = calibration_explorer

# Useful Directories
BIN_DIR = ../../../bin/
LIB_DIR = /usr/local/lib/
OUT_DIR = ../out/
BSC_DIR  = ../../basics/
IMGUI_DIR = $(BSC_DIR)extern/imgui/

UNAME = $(shell uname -s)
# List of source files
SRCS = calibration_explorer.cpp $(IMGUI_DIR)imgui.cpp $(IMGUI_DIR)imgui_draw.cpp
OBJS = $(SRCS:%.cpp=$(OUT_DIR)%.o)

# Compile and link options
CC = g++
WARNINGS = -Wall -Wno-unused-result
CPPFLAGS = -std=c++11 -O -I. -I/usr/local/include  -I$(IMGUI_DIR) -I$(BSC_DIR) 

# Libraries
LIBS = -L$(LIB_DIR) 

# Frameworks
FRAMEWORKS = -framework OpenGL -framework GLUT

ifeq ($(UNAME),Linux)
	LIBS += -lGL -lGLEW -lglfw
endif

ifeq ($(UNAME),Darwin)
	LIBS += $(FRAMEWORKS) -lglew -lglfw
	CPPFLAGS += -DGL_SILENCE_DEPRECATION
endif

# Make targets
all: clean calibration_explorer

calibration_explorer: $(OBJS)
		$(CC) $(CPPFLAGS) $(OBJS) -o ${BIN_DIR}${EXE_NAME} $(LIBS)

clean:
		rm -f $(OUT_DIR)*.a $(OUT_DIR)*.o ${BIN_DIR}${EXE_NAME} ${BIN_DIR}${EXE_NAME}.exe

make_directories:
		mkdir -p ../out; mkdir -p ../../bin	

# Compile command
$(OUT_DIR)%.o: %.cpp
		$(CC) $(WARNINGS) $(CPPFLAGS) -c $< -o $@


# GNU Make: targets that don't build files
.PHONY: all clean
