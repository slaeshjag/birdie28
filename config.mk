# Makefile configurations
MAKEFLAGS	+=	--no-print-directory

#Binaries
BINFILE		=	birdie28

# Paths
MODULESDIR      = $(TOPDIR)/submodules
MODULES		= $(patsubst %/,%,$(foreach dir,$(wildcard $(MODULESDIR)/*/Makefile),$(dir $(dir))))
MODULESINCLUDE	= $(addsuffix /include,$(MODULES))

INCLUDEDIRS	= $(TOPDIR)/include $(MODULESINCLUDE)

# Compiler and linker flags
INCLUDES	= $(addprefix -I,$(INCLUDEDIRS))

#General flags
LDFLAGS		+=	-ldarnit_s -lm -lmodplug -lbz2
CFLAGS		+=	-Wall -std=c99 -g -O0 -DDEBUG -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=600 $(INCLUDES) -pthread
ASFLAGS		+=

#Extra install targets
INSTARG		=	

#Makefile tools
MKDIR		=	mkdir -p
RMDIR		=	rmdir --ignore-fail-on-non-empty
CP		=	cp

ifeq ($(TARGET),i586-mingw32msvc-)
LDFLAGS	+=	-static -lws2_32 -lopengl32 -lwinmm -lgdi32 -lshlwapi -lstdc++ -mwindows res.res
BINFILE	:= $(BINFILE).exe
endif

i586-mingw32msvc-:
	@$(TARGET)windres $(TOPDIR)/platform/windows/res.rc -O coff -o $(TOPDIR)/res.res

ifeq ($(TARGET),)
LDFLAGS	+= -lSDL -lasound -lGL
endif
