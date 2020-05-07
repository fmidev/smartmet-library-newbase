PROG = $(patsubst %.cpp,%,$(wildcard *Test.cpp))

-include $(HOME)/.smartmet.mk
GCC_DIAG_COLOR ?= always
CXX_STD ?= c++11

FLAGS = -std=$(CXX_STD) -Wall -W -Wno-unused-parameter

ifeq ($(TSAN), yes)
  FLAGS += -fsanitize=thread
endif
ifeq ($(ASAN), yes)
  FLAGS += -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined -fsanitize-address-use-after-scope
endif

EXTRAFLAGS = -Werror -pedantic -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion -Winline \
	-Wctor-dtor-privacy -Wnon-virtual-dtor -Wno-pmf-conversions \
	-Wsign-promo -Wchar-subscripts -Wold-style-cast \
	-Wshadow -Woverloaded-virtual

DIFFICULTFLAGS = -Weffc++ -Wunreachable-code -Wredundant-decls

CC = g++
CFLAGS = -DUNIX -O0 -g $(FLAGS)

# Boost 1.69

ifneq "$(wildcard /usr/include/boost169)" ""
  INCLUDES += -I/usr/include/boost169
  LIBS += -L/usr/lib64/boost169
endif


INCLUDES += \
	-I../newbase \
	-I/usr/include/smartmet \
	-I.

LIBS +=  ../libsmartmet-newbase.so \
	-lboost_date_time \
	-lboost_filesystem \
	-lboost_iostreams \
	-lboost_thread \
	-lboost_system \
	-lgdal \
	-L/usr/pgsql-9.5/lib -lpq  \
	-lpthread -lz -lbz2 -lrt

all: $(PROG)
clean:
	rm -f $(PROG) *~

test: $(PROG)
	@echo Running tests:
	@rm -f *.err
	@for prog in $(PROG); do \
	  ( ./$$prog || touch $$prog.err ) ; \
	done
	@test `find . -name \*.err | wc -l` = "0" || ( echo ; echo "The following tests have errors:" ; \
		for i in *.err ; do echo `basename $$i .err`; done ; rm -f *.err ; false ) 

$(PROG) : % : %.cpp ../libsmartmet-newbase.so
	$(CC) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)

