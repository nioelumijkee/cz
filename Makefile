################################################################################
lib.name = cz~
cflags = 
class.sources = cz~.c
sources = 
datafiles = \
cz~-help.pd \
cz~-help.conf \
README.md \
LICENSE.txt

################################################################################
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
