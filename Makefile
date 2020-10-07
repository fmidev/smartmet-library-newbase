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

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

DEFINES = -DUNIX -D_REENTRANT -DBOOST -DFMI_COMPRESSION


CFLAGS0        = $(DEFINES) $(FLAGS) $(FLAGS_RELEASE) -DNDEBUG -O0 -g

LIBS += -L$(libdir) \
	-lfmt \
	-lboost_regex \
	-lboost_date_time \
	-lboost_filesystem \
	-lboost_iostreams \
	-lboost_thread

ifneq ($(DISABLED_GDAL),yes)
ifeq ($(shell pkg-config --exists $(gdal_version) && echo 0),0)
LIBS += -L$(PREFIX)/$(gdal_version)/lib `pkg-config --libs $(gdal_version)`
else
LIBS += -lgdal
endif
endif

# What to install

LIBFILE = libsmartmet-$(SUBNAME).so
ALIBFILE = libsmartmet-$(SUBNAME).a

# How to install
ARFLAGS = -r

# Compile option overrides

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_DEBUG)
  CFLAGS0 = $(CFLAGS_DEBUG)
endif

ifneq (,$(findstring profile,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_PROFILE)
  CFLAGS0 = $(CFLAGS_PROFILE)
endif

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
	rpmbuild -ta $(SPEC).tar.gz
	rm -f $(SPEC).tar.gz

.SUFFIXES: $(SUFFIXES) .cpp

modernize:
	for F in newbase/*.cpp; do echo $$F; clang-tidy $$F -fix -checks=-*,modernize-* -- $(CFLAGS) $(DEFINES) $(INCLUDES); done

obj/%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

obj/NFmiEnumConverter.o: NFmiEnumConverter.cpp
	$(CXX) $(CFLAGS0) $(INCLUDES) -c -o $@ $<

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
