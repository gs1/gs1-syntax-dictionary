#
# GS1 Syntax Dictionary
#
# @author Copyright (c) 2022 GS1 AISBL.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

MAJOR := 1
MINOR := 0
VERSION := $(MAJOR).$(MINOR)

NAME = gs1syntaxdictionary

BUILD_DIR = build


ifeq ($(MAKECMDGOALS),test)
BUILD_DIR = build-test
UNIT_TEST_CFLAGS = -DUNIT_TESTS -DGS1_LINTER_ERR_STR_EN
endif

ifeq ($(MAKECMDGOALS),fuzzer)
SANITIZE = yes
FUZZER_SAN_OPT = ,fuzzer
FUZZER_CORPUS = corpus
endif


ifeq ($(DEBUG),yes)
DEBUG_CFLAGS = -DPRNT
endif

ifeq ($(MAKECMDGOALS),clean-test)
BUILD_DIR = build-test
endif

ifeq ($(SANITIZE),yes)
CC=clang
SAN_LDFLAGS = -fuse-ld=lld
SAN_CFLAGS = -fsanitize=address,leak,undefined$(FUZZER_SAN_OPT) -fno-omit-frame-pointer -fno-optimize-sibling-calls -O1
SAN_ENV = ASAN_OPTIONS="symbolize=1 detect_leaks=1" LSAN_OPTIONS="fast_unwind_on_malloc=0:malloc_context_size=50" ASAN_SYMBOLIZER_PATH="$(shell which llvm-symbolizer)"
endif

# Leak detection is not supported on MacOS builds of LLVM
ifeq ($(SANITIZE),noleak)
CC=clang
SAN_LDFLAGS = -fuse-ld=lld
SAN_CFLAGS = -fsanitize=address,undefined$(FUZZER_SAN_OPT) -fno-omit-frame-pointer -fno-optimize-sibling-calls -O1
SAN_ENV = ASAN_OPTIONS="symbolize=1" LSAN_OPTIONS="fast_unwind_on_malloc=0:malloc_context_size=50" ASAN_SYMBOLIZER_PATH="$(shell which llvm-symbolizer)"
endif

ifeq ($(SLOW_TESTS),yes)
SLOW_TESTS_CFLAGS=-DSLOW_TESTS
endif

ifneq ($(shell uname -s),Darwin)
LDFLAGS = -Wl,--as-needed -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now $(SAN_LDFLAGS)
LDFLAGS_SO = -shared -Wl,-soname,lib$(NAME).so.$(MAJOR)
CFLAGS_FORTIFY = -D_FORTIFY_SOURCE=2
NPROC = nproc
else
LDFLAGS =
LDFLAGS_SO = -shared -Wl,-install_name,lib$(NAME).so.$(MAJOR)
CFLAGS_FORTIFY =
NPROC = sysctl -n hw.ncpu
endif

LDLIBS = -lc
CFLAGS = -g -O2 $(CFLAGS_FORTIFY) -Wall -Wextra -Wconversion -Wformat -Wformat-security -Wdeclaration-after-statement -pedantic -Werror -MMD -fPIC $(SAN_CFLAGS) $(UNIT_TEST_CFLAGS) $(DEBUG_CFLAGS) $(SLOW_TESTS_CFLAGS)

TEST_BIN = $(BUILD_DIR)/$(NAME)-test

LIB_STATIC = $(BUILD_DIR)/lib$(NAME).a
LIB_SHARED = $(BUILD_DIR)/lib$(NAME).so.$(VERSION) $(BUILD_DIR)/lib$(NAME).so $(BUILD_DIR)/lib$(NAME).so.$(MAJOR)

PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib

TEST_SRC = $(NAME)-test.c
TEST_OBJ = $(BUILD_DIR)/$(TEST_SRC:.c=.o)

FUZZER_LINTERS_SRC = $(NAME)-fuzzer-linters.c
#FUZZER_SRCS = $(FUZZER_LINTERS_SRC) $(NAME)-fuzzer-parser.c
FUZZER_SRCS = $(FUZZER_LINTERS_SRC)

FUZZER_LINTERS = $(patsubst %.c,%,$(wildcard lint_*.c))

FUZZER_PREFIX = $(NAME)-fuzzer-
#FUZZER_BINS = $(addprefix $(BUILD_DIR)/$(FUZZER_PREFIX),$(FUZZER_LINTERS)) $(BUILD_DIR)/$(FUZZER_PREFIX)parser
FUZZER_BINS = $(addprefix $(BUILD_DIR)/$(FUZZER_PREFIX),$(FUZZER_LINTERS))
FUZZER_OBJS = $(addsuffix .o, $(FUZZER_BINS))

FUZZER_CORPUS_PREFIX = corpus-
#FUZZER_CORPUSES = $(addsuffix /, $(addprefix $(FUZZER_CORPUS_PREFIX),$(FUZZER_LINTERS))) $(FUZZER_CORPUS_PREFIX)parser/
FUZZER_CORPUSES = $(addsuffix /, $(addprefix $(FUZZER_CORPUS_PREFIX),$(FUZZER_LINTERS)))

ALL_SRCS = $(wildcard *.c)
SRCS = $(filter-out $(TEST_SRC) $(FUZZER_SRCS), $(ALL_SRCS))
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))
DEPS = $(addprefix $(BUILD_DIR)/, $(ALL_SRCS:.c=.d)) $(FUZZER_OBJS:.o=.d)


.PHONY: all clean libshared libstatic install install-static install-shared uninstall test clean-test fuzzer docs copyright

default: lib
all: lib
lib: libshared libstatic
libshared: $(LIB_SHARED)
libstatic: $(LIB_STATIC)

$(BUILD_DIR)/:
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)/
	$(CC) $(CFLAGS) -c $< -o $@

#
#  Shared library
#
$(BUILD_DIR)/lib$(NAME).so: $(BUILD_DIR)/lib$(NAME).so.$(VERSION) $(BUILD_DIR)/lib$(NAME).so.$(MAJOR)
	ln -sf $(notdir $<) $@

$(BUILD_DIR)/lib$(NAME).so.$(MAJOR): $(BUILD_DIR)/lib$(NAME).so.$(VERSION)
	ln -sf $(notdir $<) $@

$(BUILD_DIR)/lib$(NAME).so.$(VERSION): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDFLAGS_SO) $(LDLIBS) $^ -o $@


#
#  Static library
#
$(BUILD_DIR)/lib$(NAME).a: $(OBJS)
	$(AR) cr $@ $^
	ranlib $@


#
#  Test binary
#
$(TEST_BIN): $(OBJS) $(TEST_OBJ)
	$(CC) $(CFLAGS) $(OBJS) $(TEST_OBJ) -o $(TEST_BIN)


#
#  Fuzzer binaries
#

define gen-fuzzer-target
$$(FUZZER_CORPUS_PREFIX)$1/:
	mkdir -p $$(FUZZER_CORPUS_PREFIX)$1

$$(BUILD_DIR)/$$(FUZZER_PREFIX)$1.o : $$(FUZZER_LINTERS_SRC)
	$$(CC) $$(CFLAGS) -DLINTER=gs1_$1 -c $$< -o $$@

$$(BUILD_DIR)/$$(FUZZER_PREFIX)$1: $$(OBJS) $$(BUILD_DIR)/$$(FUZZER_PREFIX)$1.o
	$$(CC) $$(CFLAGS) $$(OBJS) $$(BUILD_DIR)/$$(FUZZER_PREFIX)$1.o -o $$(BUILD_DIR)/$$(FUZZER_PREFIX)$1
endef

$(foreach linter,$(FUZZER_LINTERS),$(eval $(call gen-fuzzer-target,$(linter))))


#$(FUZZER_CORPUS_PREFIX)parser/:
#	mkdir -p $@
#
#$(BUILD_DIR)/$(FUZZER_PREFIX)parser: $(OBJS) $(BUILD_DIR)/$(FUZZER_PREFIX)parser.o
#	$(CC) $(CFLAGS) $(OBJS) $(BUILD_DIR)/$(FUZZER_PREFIX)parser.o -o $(BUILD_DIR)/$(FUZZER_PREFIX)parser


#
#  Utility targets
#
test: $(TEST_BIN)
	$(SAN_ENV) ./$(TEST_BIN) $(TEST)

fuzzer: $(FUZZER_BINS) | $(FUZZER_CORPUSES)
	@echo
	@echo Start fuzzing as follows:
	@echo
	@for sym in $^ ; do \
		echo '$(SAN_ENV)' $$sym -jobs=`$(NPROC)` -workers=`$(NPROC)` $(FUZZER_CORPUS_PREFIX)$${sym##*-} -max_len=99 ; echo ; \
	done
	@echo
	@for sym in $^ ; do \
		echo '$(SAN_ENV)' nice $$sym $(FUZZER_CORPUS_PREFIX)$${sym##*-} -max_len=99 \& ; \
	done

	@echo

clean:
	$(RM) $(OBJS) $(TEST_BIN) $(TEST_OBJ) $(FUZZER_BINS) $(FUZZER_OBJS) $(LIB_STATIC) $(LIB_SHARED) $(DEPS)

clean-test:
	$(RM) $(OBJS) $(TEST_BIN) $(TEST_OBJ) $(FUZZER_BINS) $(FUZZER_OBJS) $(LIB_STATIC) $(LIB_SHARED) $(DEPS)


install: install-static install-shared

install-headers:
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 0644 $(NAME).h $(DESTDIR)$(PREFIX)/include

install-static: libstatic install-headers
	install -d $(DESTDIR)$(LIBDIR)
	install -m 0644 $(BUILD_DIR)/lib$(NAME).a $(DESTDIR)$(LIBDIR)

install-shared: libshared install-headers
	install -d $(DESTDIR)$(LIBDIR)
	install -m 0644 $(BUILD_DIR)/lib$(NAME).so.$(VERSION) $(DESTDIR)$(LIBDIR)
	cd $(DESTDIR)$(LIBDIR) && ln -sf lib$(NAME).so.$(VERSION) lib$(NAME).so
	cd $(DESTDIR)$(LIBDIR) && ln -sf lib$(NAME).so.$(VERSION) lib$(NAME).so.$(MAJOR)
	-ldconfig

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/include/$(NAME).h
	$(RM) $(DESTDIR)$(PREFIX)/lib/lib$(NAME).so.$(VERSION)
	$(RM) $(DESTDIR)$(PREFIX)/lib/lib$(NAME).so.$(MAJOR)
	$(RM) $(DESTDIR)$(PREFIX)/lib/lib$(NAME).so
	$(RM) $(DESTDIR)$(PREFIX)/lib/lib$(NAME).a
	-ldconfig

docs:
	doxygen
	perl -0777 -ne 'print "$$&\n" if m#<div class="directory">.*</div><!-- directory -->#s' docstmp/files.html | \
	  perl -0pi -e 'BEGIN{$$x=<STDIN>;}s#!!!REFERENCE!!!#$$x#sm' docstmp/index.html
	perl -pi -e 's#lint_([A-Za-z0-9_]+)\.c#\1#' docstmp/index.html
	perl -0pi -e 's#<h2 class="groupheader">Functional Description</h2>.*</div><!-- fragment -->.</div>.</div>.##ms' docstmp/gs1syntaxdictionary_8h.html
	cp docstmp/index.html ../docs
	cp docstmp/gs1syntaxdictionary_8h.html ../docs
	cp docstmp/lint__*.html ../docs
	cp docstmp/doxygen.css ../docs
	cp docstmp/doxygen.svg ../docs
	cp docstmp/dynsections.js ../docs
	cp docstmp/jquery.js ../docs
	cp docstmp/menu.js ../docs
	cp docstmp/menudata.js ../docs
	cp docstmp/doc.svg ../docs
	cp docstmp/nav_*.png ../docs
	cp docstmp/tabs.css ../docs
	cp docstmp/tab_*.png ../docs

copyright:
	sed -i -e "s/Copyright (c) \([[:digit:]]\{4\}\)\(-[[:digit:]]\{4\}\)\{0,1\} GS1 AISBL/Copyright (c) \1-$$(date +'%Y') GS1 AISBL/" $(ALL_SRCS) $(NAME).h unittest.h Makefile


-include $(DEPS)
