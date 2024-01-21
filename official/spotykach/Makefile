# Project Name
TARGET = spotykach

# Sources
C_INCLUDES += -Icore
CPP_SOURCES = spotykach.cpp $(wildcard core/*.cpp) $(wildcard control/*.cpp) $(wildcard fx/mi/*.cpp)

# Library Locations
LIBDAISY_DIR = lib/libdaisy/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

CPP_STANDARD = -std=gnu++17
