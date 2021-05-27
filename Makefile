#------------------------------------------------------------------------------#
.PHONY = 

#------------------------------------------------------------------------------#
LIBRARY_NAME = cz~
LIBRARY_AUTHOR = Mike Moser Booth / MikeMoreno / Nioelumijke
LIBRARY_DESCRIPTION = Emulates oscillator Casio CZ series
LIBRARY_LICENSE = Standard Improved BSD License
LIBRARY_VERSION = 0.1
META_FILE = $(LIBRARY_NAME)-meta.pd

#------------------------------------------------------------------------------#
SOURCES_DIR = src
SOURCES = \
cz~.c
EXTRA_DIST = README.md LICENSE.txt
HELPPATCHES = $(SOURCES:.c=-help.pd) $(PDOBJECTS:.pd=-help.pd)

#------------------------------------------------------------------------------#
UNAME := $(shell uname -s)
#------------------------------------------------------------------------------#
ifeq ($(UNAME),Linux)
  CPU := $(shell uname -m)
  EXTENSION = pd_linux
  SHARED_EXTENSION = so
  OS = linux
  PD_PATH ?= /usr
  PD_INCLUDE = $(PD_PATH)/include/pd
  CFLAGS = -I"$(PD_INCLUDE)" -Wall -W
  CFLAGS += -DPD -DVERSION='"$(LIBRARY_VERSION)"'
  CFLAGS += -fPIC
  CFLAGS += -O6 -funroll-loops -fomit-frame-pointer
  LDFLAGS = -rdynamic -shared -fPIC -Wl,-rpath,"\$$ORIGIN",--enable-new-dtags
  LIBS_linux =
  LIBS = -lc $(LIBS_linux)
endif

#------------------------------------------------------------------------------#
ifeq (CYGWIN,$(findstring CYGWIN,$(UNAME)))
  CPU := $(shell uname -m)
  EXTENSION = dll
  SHARED_EXTENSION = dll
  OS = cygwin
  PD_PATH ?= $(shell cygpath $$PROGRAMFILES)/pd
  CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  LDFLAGS = -rdynamic -shared -L"$(PD_PATH)/src" -L"$(PD_PATH)/bin"
  LIBS_cygwin =
  LIBS = -lc -lpd $(LIBS_cygwin)
endif

#------------------------------------------------------------------------------#
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
  CPU := $(shell uname -m)
  EXTENSION = dll
  SHARED_EXTENSION = dll
  OS = windows
  PD_PATH ?= $(shell cd "$$PROGRAMFILES/pd" && pwd)
  CC=gcc
  CFLAGS = -O3 -funroll-loops -fomit-frame-pointer
  CFLAGS += -mms-bitfields
  LDFLAGS = -s -shared -Wl,--enable-auto-import
  LIBS_windows =
  LIBS = -L"$(PD_PATH)/src" -L"$(PD_PATH)/bin" -L"$(PD_PATH)/obj" \
	-lpd -lwsock32 -lkernel32 -luser32 -lgdi32 -liberty $(LIBS_windows)
endif

#------------------------------------------------------------------------------#
all: $(SOURCES:.c=.$(EXTENSION))
	@echo "done."

%.o: $(SOURCES_DIR)/%.c
	$(CC) $(CFLAGS) -o "$(SOURCES_DIR)/$*.o" -c "$(SOURCES_DIR)/$*.c"

%.$(EXTENSION): $(SOURCES_DIR)/%.o
	$(CC) $(LDFLAGS) -o "$*.$(EXTENSION)" "$(SOURCES_DIR)/$*.o"  $(LIBS)
	chmod a-x "$*.$(EXTENSION)"

#------------------------------------------------------------------------------#
clean:
	-rm -f -- $(SOURCES_DIR)/$(SOURCES:.c=.o)
	-rm -f -- $(SOURCES:.c=.$(EXTENSION))

#------------------------------------------------------------------------------#
meta:
	@echo "#N canvas 100 100 360 360 10;" > $(META_FILE)
	@echo "#X text 10 10 META this is prototype of a libdir meta file;" >> $(META_FILE)
	@echo "#X text 10 30 NAME" $(LIBRARY_NAME) ";" >> $(META_FILE)
	@echo "#X text 10 50 AUTHOR" $(LIBRARY_AUTHOR) ";" >> $(META_FILE)
	@echo "#X text 10 70 DESCRIPTION" $(LIBRARY_DESCRIPTION) ";" >> $(META_FILE)
	@echo "#X text 10 90 LICENSE" $(LIBRARY_LICENSE) ";" >> $(META_FILE)
	@echo "#X text 10 110 VERSION" $(LIBRARY_VERSION) ";" >> $(META_FILE)
	@echo "meta done"

#------------------------------------------------------------------------------#
showsetup:
	@echo "UNAME               : $(UNAME)"
	@echo "CPU                 : $(CPU)"
	@echo "OS                  : $(OS)"
	@echo "EXTENSION           : $(EXTENSION)"
	@echo "SHARED_EXTENSION    : $(SHARED_EXTENSION)"
	@echo "PD_PATH             : $(PD_PATH)"
	@echo "PD_INCLUDE          : $(PD_INCLUDE)"
	@echo "CFLAGS              : $(CFLAGS)"
	@echo "LDFLAGS             : $(LDFLAGS)"
	@echo "LIBS                : $(LIBS)"
	@echo "LIBRARY_NAME        : $(LIBRARY_NAME)"
	@echo "LIBRARY_AUTHOR      : $(LIBRARY_AUTHOR)"
	@echo "LIBRARY_DESCRIPTION : $(LIBRARY_DESCRIPTION)"
	@echo "LIBRARY_LICENSE     : $(LIBRARY_LICENSE)"
	@echo "LIBRARY_VERSION     : $(LIBRARY_VERSION)"
	@echo "SOURCES             : $(SOURCES)"
	@echo "EXTRA_DIST          : $(EXTRA_DIST)"
	@echo "HELPPATCHES         : $(HELPPATCHES)"
