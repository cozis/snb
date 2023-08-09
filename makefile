ifeq ($(OS),Windows_NT)
	OSNAME = windows
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
    	ARCH = AMD64
    else ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
    	ARCH = AMD64
	else ifeq ($(PROCESSOR_ARCHITECTURE),x86)
        ARCH = IA32
	endif
else
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OSNAME = linux
    else ifeq ($(UNAME_S),Darwin)
        OSNAME = osx
    endif

    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        ARCH = AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        ARCH = IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        ARCH = ARM
    endif
endif

# Auxiliary functions
rwildcard = $(foreach d, $(wildcard $(1:=/*)), $(call rwildcard ,$d, $2) $(filter $(subst *, %, $2), $d))
uppercase = $(shell echo '$1' | tr '[:lower:]' '[:upper:]')

CC = gcc

# release, debug
BUILD = debug

OUTDIR = .
SRCDIR = src

ifeq ($(OSNAME),windows)
	OBJDIR = cache/windows
else ifeq ($(OSNAME),linux)
	OBJDIR = cache/linux
else
	OBJDIR = cache/else
endif

EXE_EDITOR = snb

ifeq ($(OSNAME),windows)
	RAYLIB_DIR = 3p/raylib-4.5.0_win64_mingw-w64
else ifeq ($(OSNAME),linux)
	RAYLIB_DIR = 3p/raylib-4.5.0_linux_amd64
else
	$(error Unknown OS. Was expected windows)
endif
RAYLIB_INC = $(RAYLIB_DIR)/include
RAYLIB_LIB = $(RAYLIB_DIR)/lib

# List of include and library paths
INCDIRS = $(RAYLIB_INC)
LIBDIRS = $(RAYLIB_LIB)

CFLAGS_ALWAYS  = -Wall -Wextra -Wpedantic
CFLAGS_DEBUG   = -g
CFLAGS_LINUX   =
CFLAGS_WINDOWS =

LFLAGS_ALWAYS  = -l:libraylib.a
LFLAGS_DEBUG   = -g
LFLAGS_LINUX   = -lm -lpthread -ldl
LFLAGS_WINDOWS = -lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lws2_32

CFILES = $(call rwildcard, $(SRCDIR) $(SRCDIR)/widget $(SRCDIR)/utils, *.c)
OFILES = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(CFILES))

CFLAGS = $(CFLAGS_CUSTOM) $(CFLAGS_ALWAYS) $(patsubst %, -I%, $(INCDIRS))
LFLAGS = $(LFLAGS_CUSTOM) $(LFLAGS_ALWAYS) $(patsubst %, -L%, $(LIBDIRS))

ifeq ($(BUILD),debug)
	CFLAGS += $(CFLAGS_DEBUG)
	LFLAGS += $(LFLAGS_DEBUG)
endif
ifeq ($(OSNAME),windows)
	CFLAGS += $(CFLAGS_WINDOWS)
	LFLAGS += $(LFLAGS_WINDOWS)
else ifeq ($(OSNAME),linux)
	CFLAGS += $(CFLAGS_LINUX)
	LFLAGS += $(LFLAGS_LINUX)
else
	$(error Unknown OS. Was expected Windows or Linux)
endif

all: $(EXE_EDITOR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE_EDITOR): $(OFILES)
	$(CC) -o $@ $^ $(LFLAGS)

clean:
	rm -fr cache snb snb.exe