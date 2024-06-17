# Project Name
TARGET ?= Looper

DEBUG = 0
OPT = -Os

# Sources
CPP_SOURCES += \
src/App.cpp \
src/SteppedClock.cpp \
lib/kxmx_bluemchen/src/kxmx_bluemchen.cpp \


USE_FATFS = 0

# Library Locations
LIBDAISY_DIR = ./lib/libDaisy
DAISYSP_DIR = ./lib/DaisySP

C_INCLUDES += \
-I. \



# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

C_INCLUDES += -Ilib/stmlib
C_INCLUDES += -Ilib/eurorack
C_INCLUDES += -Ilib/kxmx_bluemchen/src
C_INCLUDES += -Wno-unused-local-typedefs