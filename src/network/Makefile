first: all

TOPDIR	=	$(shell DIR=.; while [ ! "`readlink -f \"$${DIR}\"`" = "/" -a ! -f "$${DIR}/config.mk" ]; do DIR="../$${DIR}"; done; echo "$${DIR}")
ifeq ($(shell readlink -f "$(TOPDIR)"),/)
	$(error Could not find the project top directory with config.mk)
endif
include $(TOPDIR)/config.mk
-include extra.mk

# Files to build
ASMFILES	=	$(wildcard *.S)
SRCFILES	=	$(wildcard *.c)
OBJFILES	=	$(SRCFILES:.c=.c.o)
OBJFILES	+=	$(ASMFILES:.S=.S.o)

OUTFILE		=	out.a

# Flags
CFLAGS		+=	
ASFLAGS		+=	
LDFLAGS		+=	

# Sub directories to build
SUBDIRS		=	$(foreach dir,$(wildcard */Makefile),$(dir $(dir)))

LIBS		=	$(addsuffix $(OUTFILE),$(SUBDIRS))

DEPDIR = .deps
df = $(DEPDIR)/$(*F)

.PHONY: all clean release install
.PHONY: $(SUBDIRS)

all: $(OUTFILE)
	@echo
	@echo "Done."
	@echo

clean: $(SUBDIRS)
	@echo
	@echo " [ RM ] $(OBJFILES) $(OUTFILE)"
	@$(RM) $(OBJFILES) $(OUTFILE)
	@$(RM) -R $(DEPDIR)
	@echo "Done."
	@echo 

$(OUTFILE): $(OBJFILES) $(SUBDIRS) $(TARGET)
	@echo " [ AR ] $(CURRENTPATH)$(OUTFILE)"
	@$(RM) $(OUTFILE)
	@$(AR) -cmT $(OUTFILE) $(OBJFILES) $(LIBS)

$(SUBDIRS):
	@echo " [ CD ] $(CURRENTPATH)$@"
	@+make -C "$@" "CURRENTPATH=$(CURRENTPATH)$@" $(MAKECMDGOALS)

$(DEPDIR):
	@mkdir -p $@

%.c.o: %.c | $(DEPDIR)
	@echo " [ CC ] $(CURRENTPATH)$<"
	@$(CC) -c $(CFLAGS) -MD -o $@ $<
	@cp $*.c.d $(df).c.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.c.d >> $(df).c.P; $(RM) $*.c.d
	
%.S.o: %.S | $(DEPDIR)
	@echo " [ AS ] $(CURRENTPATH)$<"
	@$(CC) -c $(ASFLAGS) -MD -o $@ $<
	@cp $*.S.d $(df).S.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.S.d >> $(df).S.P; $(RM) $*.S.d

-include $(SRCFILES:%.c=$(DEPDIR)/%.c.P)
-include $(ASMFILES:%.S=$(DEPDIR)/%.S.P)
