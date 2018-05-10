first: all

TOPDIR	=	.
include config.mk

# Sub directories to build
SRCDIRS		=	src
PLUGINSDIR	=	plugins

SUBDIRS		=	$(SRCDIRS) $(MODULESDIR)

#LIBS		=	$(addsuffix /out.a,$(SUBDIRS))
MODULESLIBS	= $(addsuffix .a,$(MODULES))

.PHONY: all clean
.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
	@echo " [ LD ] $(BINFILE)"
	@$(CC) -o $(BINFILE) $(CFLAGS) -Wl,--whole-archive $(addsuffix /out.a,$(SRCDIRS)) -Wl,--no-whole-archive $(MODULESLIBS) $(LDFLAGS)
	
	@darnit-tmxconv res-input/map.tmx res/map.ldmz
	
	@echo "Build complete."
	@echo 

clean: $(SUBDIRS)
	@echo " [ RM ] $(BINFILE)"
	@$(RM) $(BINFILE)
	
	@echo
	@echo "Source tree cleaned."
	@echo

$(SUBDIRS):
	@echo " [ CD ] $(CURRENTPATH)$@/"
	@+make -C "$@" "CURRENTPATH=$(CURRENTPATH)$@/" $(MAKECMDGOALS)
