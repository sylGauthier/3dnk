NAME := 3dnk
DEPS := 3dmr
DEBUG :=

PREFIX ?= /usr/local
INCLUDEDIR ?= include
LIBDIR ?= lib

CFLAGS ?= -std=c89 -pedantic -march=native -Wall $(if $(DEBUG),-g -DDEBUG,-O3)

CFLAGS += $(shell pkg-config --cflags $(DEPS)) -I.
LDFLAGS += $(shell pkg-config --libs $(DEPS)) -lm

OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c))
TESTS := $(patsubst %.c,%,$(wildcard test/*.c))

LIB := lib$(NAME).a

.PHONY: all test
all: $(LIB)

$(LIB): $(OBJECTS)
	$(AR) rcs $@ $^

test/simple: test/simple.c $(LIB)
	$(CC) -o $@ $< -I. $(CFLAGS) $(shell pkg-config --cflags lib3dasset) -L. -l3dnk $(LDFLAGS) $(shell pkg-config --libs lib3dasset)

test: $(TESTS)

clean:
	rm -rf $(OBJECTS) $(LIB) $(TESTS)

install: $(LIB) $(NAME).pc
	mkdir -p $(PREFIX)/$(INCLUDEDIR) $(PREFIX)/$(LIBDIR)/pkgconfig $(PREFIX)/bin
	cp $(NAME).h nuklear.h $(PREFIX)/$(INCLUDEDIR)
	cp $(LIB) $(PREFIX)/$(LIBDIR)
	cp $(NAME).pc $(PREFIX)/$(LIBDIR)/pkgconfig

.PHONY: $(NAME).pc
$(NAME).pc:
	printf 'prefix=%s\nincludedir=%s\nlibdir=%s\n\nName: %s\nDescription: %s\nVersion: %s\nCflags: %s\nLibs: %s\nRequires: %s' \
		'$(PREFIX)' \
		'$${prefix}/$(INCLUDEDIR)' \
		'$${prefix}/$(LIBDIR)' \
		'lib$(NAME)' \
		'An implementation of the Nuklear lib for 3dmr' \
		'0.1' \
		'-I$${includedir}' \
		'-L$${libdir} -l$(NAME)' \
		'$(DEPS)' \
		> $@

