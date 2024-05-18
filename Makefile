# Project Name
TARGET ?= Looper

DEBUG = 1
OPT = -O0


# Sources

# example
#CPP_SOURCES = src/kxmx_bluemchen.cpp examples/${TARGET}.cpp
CPP_SOURCES += \
bluemchen/kxmx_bluemchen.cpp \
src/App.cpp \
src/SteppedClock.cpp \

USE_FATFS = 0

# Library Locations
LIBDAISY_DIR = libDaisy
DAISYSP_DIR = DaisySP
STMLIB_DIR = stmlib
CXXFLAGS += \
-DBLUEMCHEN \

C_INCLUDES += \
-I. \

C_INCLUDES += -Wno-unused-local-typedefs

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile