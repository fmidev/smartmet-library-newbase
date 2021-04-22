SUBNAME = newbase
LIB = smartmet-$(SUBNAME)
SPEC = smartmet-library-$(SUBNAME)
INCDIR = smartmet/$(SUBNAME)


# Say 'yes' to disable Gdal
DISABLED_GDAL ?=
REQUIRES :=
ifneq ($(DISABLED_GDAL),yes)
REQUIRES += gdal
endif

REQUIRES += fmt

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

DEFINES = -DUNIX -D_REENTRANT -DBOOST -DFMI_COMPRESSION


CFLAGS0        = $(DEFINES) $(FLAGS) $(FLAGS_RELEASE) -DNDEBUG -O0 -g

LIBS += -lsmartmet-gis \
	-lboost_regex \
	-lboost_date_time \
	-lboost_filesystem \
	-lboost_iostreams \
	-lboost_thread \
	$(REQUIRED_LIBS)

# What to install

LIBFILE = libsmartmet-$(SUBNAME).so
ALIBFILE = libsmartmet-$(SUBNAME).a

# How to install
ARFLAGS = -r

# Compilation directories

vpath %.cpp $(SUBNAME)
vpath %.h $(SUBNAME)

# The files to be compiled

SRCS = $(wildcard $(SUBNAME)/*.cpp)
HDRS = $(wildcard $(SUBNAME)/*.h)
OBJS = $(patsubst %.cpp, obj/%.o, $(notdir $(SRCS)))

INCLUDES := -Iinclude $(INCLUDES)

.PHONY: test rpm

# The rules

all: objdir $(LIBFILE) $(ALIBFILE)
debug: all
release: all
profile: all

$(LIBFILE): $(OBJS)
	$(CXX) $(CFLAGS) -shared -rdynamic -o $(LIBFILE) $(OBJS) $(LIBS)
	@echo Checking $(LIBFILE) for unresolved references
	@if ldd -r $(LIBFILE) 2>&1 | c++filt | grep ^undefined\ symbol; \
		then rm -v $(LIBFILE); \
		exit 1; \
	fi

$(ALIBFILE): $(OBJS)
	$(AR) $(ARFLAGS) $(ALIBFILE) $(OBJS)

clean:
	rm -f $(LIBFILE) $(ALIBFILE) *~ $(SUBNAME)/*~
	rm -rf $(objdir)
	rm -f test/*Test

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp test/*.cpp

install:
	@mkdir -p $(includedir)/$(INCDIR)
	@list='$(HDRS)'; \
	for hdr in $$list; do \
	  HDR=$$(basename $$hdr); \
	  echo $(INSTALL_DATA) $$hdr $(includedir)/$(INCDIR)/$$HDR; \
	  $(INSTALL_DATA) $$hdr $(includedir)/$(INCDIR)/$$HDR; \
	done
	@mkdir -p $(libdir)
	echo $(INSTALL_PROG) $(LIBFILE) $(libdir)/$(LIBFILE)
	$(INSTALL_PROG) $(LIBFILE) $(libdir)/$(LIBFILE)
	echo $(INSTALL_DATA) $(ALIBFILE) $(libdir)/$(ALIBFILE)
	$(INSTALL_DATA) $(ALIBFILE) $(libdir)/$(ALIBFILE)

test:
	+cd test && make test

objdir:
	@mkdir -p $(objdir)

rpm: clean $(SPEC).spec
	rm -f $(SPEC).tar.gz # Clean a possible leftover from previous attempt
	tar -czvf $(SPEC).tar.gz --exclude test --exclude-vcs --transform "s,^,$(SPEC)/," *
	rpmbuild -tb $(SPEC).tar.gz
	rm -f $(SPEC).tar.gz

.SUFFIXES: $(SUFFIXES) .cpp

modernize:
	for F in newbase/*.cpp; do echo $$F; clang-tidy $$F -fix -checks=-*,modernize-* -- $(CFLAGS) $(DEFINES) $(INCLUDES); done

obj/%.o: %.cpp
	@mkdir -p obj
	$(CXX) $(CFLAGS) $(INCLUDES) -c  -MD -MF $(patsubst obj/%.o, obj/%.d.new, $@) -o $@ $<
	@sed -e "s|^$(notdir $@):|$@:|" $(patsubst obj/%.o, obj/%.d.new, $@) >$(patsubst obj/%.o, obj/%.d, $@)
	@rm -f $(patsubst obj/%.o, obj/%.d.new, $@)

ifneq ($(USE_CLANG), yes)
obj/NFmiEnumConverterInit.o: CFLAGS += -O0
endif

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif

-include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile-abicheck.inc
