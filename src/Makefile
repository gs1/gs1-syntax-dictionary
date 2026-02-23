#
# GS1 Barcode Syntax Dictionary
#
# @author Copyright (c) 2022-2026 GS1 AISBL.
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
UNIT_TEST_CFLAGS = -DUNIT_TESTS -DGS1_LINTER_ERR_STR_EN -DGS1_LINTER_CUSTOM_GCP_LOOKUP_H=test-gcp-lookup.h
endif

ifeq ($(MAKECMDGOALS),fuzzer)
SANITIZE = yes
FUZZER_SAN_OPT = ,fuzzer
FUZZER_CORPUS = corpus
FUZZER_MAX_LEN = 125
endif


ifeq ($(DEBUG),yes)
DEBUG_CFLAGS = -DPRNT
endif

CFLAGS_V = -fvisibility=hidden

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

# MemorySanitizer: detects reads of uninitialised memory.
# Mutually exclusive with ASAN so uses its own build directory.
#
#   make MSAN=yes test
#
ifeq ($(MSAN),yes)
CC=clang
BUILD_DIR = build-msan
SAN_LDFLAGS = -fuse-ld=lld
SAN_CFLAGS = -fsanitize=memory -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -fno-optimize-sibling-calls -O1
SAN_ENV = MSAN_SYMBOLIZER_PATH="$(shell which llvm-symbolizer)"
endif


# Code coverage: builds with gcov instrumentation, runs tests and
# generates an HTML report via lcov/genhtml.
#
#   make coverage
#
ifeq ($(COVERAGE),yes)
BUILD_DIR = build-coverage
COV_CFLAGS = --coverage
endif


ifeq ($(SLOW_TESTS),yes)
SLOW_TESTS_CFLAGS=-DSLOW_TESTS
endif

ifeq ($(ANALYZER),yes)
ANALYZER_CFLAGS = -fanalyzer
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
CFLAGS = -g -O2 $(CFLAGS_FORTIFY) $(CFLAGS_V) -Wall -Wextra -Wconversion -Wformat=2 -Wshadow -Wdeclaration-after-statement -pedantic -Wundef -Wnull-dereference -Wstrict-prototypes -Werror -fstack-protector-strong -MMD -fPIC $(SAN_CFLAGS) $(COV_CFLAGS) $(ANALYZER_CFLAGS) $(UNIT_TEST_CFLAGS) $(DEBUG_CFLAGS) $(SLOW_TESTS_CFLAGS)

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

FUZZER_LINTERS = $(patsubst %.c,%,$(filter-out lint__stubs.c,$(wildcard lint_*.c)))

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


default: lib

.PHONY: all
all: lib

.PHONY: lib
lib: libshared libstatic

.PHONY: libshared
libshared: $(LIB_SHARED)

.PHONY: libstatic
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
.PHONY: test
test: $(TEST_BIN)
	$(SAN_ENV) ./$(TEST_BIN) $(TEST)

.PHONY: coverage
coverage:
	$(MAKE) COVERAGE=yes SLOW_TESTS=yes test
	lcov --capture --directory build-coverage \
		--output-file build-coverage/coverage.info
	lcov --ignore-errors unused \
		--remove build-coverage/coverage.info \
		'*/acutest.h' '*/gs1syntaxdictionary-test.c' '/usr/include/*' \
		--output-file build-coverage/coverage.info
	genhtml build-coverage/coverage.info \
		--output-directory build-coverage/coverage-report
	@echo
	@echo "Coverage report: build-coverage/coverage-report/index.html"

.PHONY: fuzzer
fuzzer: $(FUZZER_BINS) | $(FUZZER_CORPUSES)
	@echo
	@echo Start fuzzing as follows:
	@echo
	@for sym in $^ ; do \
		echo '$(SAN_ENV)' $$sym -jobs=`$(NPROC)` -workers=`$(NPROC)` $(FUZZER_CORPUS_PREFIX)$${sym##*-} -max_len=$(FUZZER_MAX_LEN) ; echo ; \
	done
	@echo
	@for sym in $^ ; do \
		echo '$(SAN_ENV)' nice $$sym $(FUZZER_CORPUS_PREFIX)$${sym##*-} -max_len=$(FUZZER_MAX_LEN) \& ; \
	done

	@echo

.PHONY: clean
clean:
	$(RM) -r build build-test build-msan build-coverage
	$(RM) *.gcov


.PHONY: install
install: install-static install-shared

.PHONY: install-headers
install-headers:
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 0644 $(NAME).h $(DESTDIR)$(PREFIX)/include

.PHONY: install-static
install-static: libstatic install-headers
	install -d $(DESTDIR)$(LIBDIR)
	install -m 0644 $(BUILD_DIR)/lib$(NAME).a $(DESTDIR)$(LIBDIR)

.PHONY: install-shared
install-shared: libshared install-headers
	install -d $(DESTDIR)$(LIBDIR)
	install -m 0644 $(BUILD_DIR)/lib$(NAME).so.$(VERSION) $(DESTDIR)$(LIBDIR)
	cd $(DESTDIR)$(LIBDIR) && ln -sf lib$(NAME).so.$(VERSION) lib$(NAME).so
	cd $(DESTDIR)$(LIBDIR) && ln -sf lib$(NAME).so.$(VERSION) lib$(NAME).so.$(MAJOR)
	-ldconfig

.PHONY: uninstall
uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/include/$(NAME).h
	$(RM) $(DESTDIR)$(PREFIX)/lib/lib$(NAME).so.$(VERSION)
	$(RM) $(DESTDIR)$(PREFIX)/lib/lib$(NAME).so.$(MAJOR)
	$(RM) $(DESTDIR)$(PREFIX)/lib/lib$(NAME).so
	$(RM) $(DESTDIR)$(PREFIX)/lib/lib$(NAME).a
	-ldconfig

.PHONY: docs
docs:
	doxygen
	perl -0777 -ne 'print "$$&\n" if m#<div class="directory">.*</div><!-- directory -->#s' docstmp/files.html | \
	  perl -0pi -e 'BEGIN{$$x=<STDIN>;}s#!!!REFERENCE!!!#$$x#sm' docstmp/index.html
	perl -pi -e 's#lint_([A-Za-z0-9_]+)\.c#\1#' docstmp/index.html
	perl -0pi -e 's#<h2 class="groupheader">Functional Description</h2>.*</div><!-- fragment -->.</div>.</div>.##ms' docstmp/gs1syntaxdictionary_8h.html
	perl -0pi -e 's#<div class="fragment"><div class="line"><span class="lineno">.*?<!-- fragment -->.##msg' docstmp/gs1syntaxdictionary-utils_8h.html
	cp docstmp/index.html ../docs
	cp docstmp/gs1syntaxdictionary_8h.html ../docs
	cp docstmp/gs1syntaxdictionary-utils_8h.html ../docs
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

.PHONY: copyright
copyright:
	sed -i -e "s/Copyright (c) \([[:digit:]]\{4\}\)\(-[[:digit:]]\{4\}\)\{0,1\} GS1 AISBL/Copyright (c) \1-$$(date +'%Y') GS1 AISBL/" $(ALL_SRCS) $(NAME).h unittest.h Makefile Doxyfile

.PHONY: superlinter
superlinter:
	docker run --rm \
		-e RUN_LOCAL=true \
		-e VALIDATE_ALL_CODEBASE=true \
		-e DEFAULT_BRANCH=main \
		-e IGNORE_GITIGNORED_FILES=true \
		-e LINTER_RULES_PATH=.github/linters \
		-e 'FILTER_REGEX_EXCLUDE=(docs/|src/build|src/acutest\.h|corpus-|src/slow-unit-)' \
		$(shell grep -v '^\#' ../.github/linters/superlinter-disabled.env | grep '=' | sed 's/^/-e /') \
		-v `pwd`/..:/tmp/lint \
		ghcr.io/super-linter/super-linter:v8


-include $(DEPS)
