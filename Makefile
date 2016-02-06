# The MIT License (MIT)
# Copyright (c) 2016 Andrea Bernardo Ciddio (Code::Chunks) and Marek Miller
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

MAJOR := 0
MINOR := 0
VERSION := $(MAJOR).$(MINOR)

NAME        := bitcoinrpc
SRCDIR      := src
DOCDIR      := doc
TESTNAME    := $(NAME)_test
TESTSRCDIR  := $(SRCDIR)/test
LIBDIR      := .lib
BINDIR      := bin
LDFLAGS     := -luuid -ljansson -lcurl
TESTLDFLAGS := -ljansson

CFLAGS := -fPIC -O3 -g -Wall -Werror -Wextra -std=c99
CC := gcc

SHELL := /bin/sh

INSTALL_PREFIX   := /usr/local
INSTALL_DOCSPATH := /usr/share/doc

INSTALL_LIBPATH    := $(INSTALL_PREFIX)/lib
INSTALL_HEADERPATH := $(INSTALL_PREFIX)/include
INSTALL_MANPATH    := $(shell manpath | cut -f 1 -d ":")

# Programs for installation
INSTALL_PROGRAM = $(INSTALL)
INSTALL = install
INSTALL_DATA = $(INSTALL) -m 644

RM = rm -fv
# -----------------------------------------------------------------------------
CFLAGS += -D VERSION=\"$(VERSION)\"

SRCFILES = $(shell find $(SRCDIR) -maxdepth 1 -iname '*.c')
OBJFILES = $(shell echo $(SRCFILES) | sed 's/\.c/\.o/g')

.PHONY: all
all: prep lib build-test

.PHONY: prep
prep:
	@echo
	@mkdir -p $(LIBDIR) $(BINDIR)


.PHONY: lib
lib: $(LIBDIR)/lib$(NAME).so

$(LIBDIR)/lib$(NAME).so: $(LIBDIR)/lib$(NAME).so.$(VERSION)
	@echo
	ldconfig -v -n $(LIBDIR)
	ln -fs lib$(NAME).so.$(MAJOR) $(LIBDIR)/lib$(NAME).so

$(LIBDIR)/lib$(NAME).so.$(VERSION): $(OBJFILES)
	@echo
	$(CC) $(CFLAGS) -shared -Wl,-soname,lib$(NAME).so.$(MAJOR) \
	$(OBJFILES) \
	-o $@ \
	-Wl,--copy-dt-needed-entries $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<


# --------- test -----------------
.PHONY: build-test
build-test: lib $(BINDIR)/$(TESTNAME)

$(BINDIR)/$(TESTNAME): $(TESTSRCDIR)/$(TESTNAME).o
	@echo
	$(CC) $(CFLAGS) $(TESTSRCDIR)/$(TESTNAME).o -o $@ \
		-l$(NAME) $(TESTLDFLAGS) -L$(LIBDIR) -I $(SRCDIR) -Wl,-rpath=$(LIBDIR)

$(TESTSRCDIR)/$(TESTNAME).o: $(TESTSRCDIR)/$(TESTNAME).c
	$(CC) $(CFLAGS) -c $< -o $@ \
		-l$(NAME) -L$(LIBDIR) -I $(SRCDIR) -Wl,-rpath=$(LIBDIR)



BITCOINDATADIR := $(shell mktemp -d)
BITCOINPARAMS   = -regtest -datadir=$(BITCOINDATADIR)

.PHONY: prep-test
prep-test:
	@echo
	@echo "Testing library routines in regtest mode (bitcoind and bitcoin-cli needed!)"
	@if ! which bitcoind ; then echo "Can't find bitcoind executable." ; exit 1; fi
	@if ! which bitcoin-cli ; then echo "Can't find bitcoin-cli executable." ; exit 1; fi
	@echo "Prepare regtest blockchain"
	@echo "datadir: $(BITCOINDATADIR)"
	@mkdir -p $(BITCOINDATADIR)
	echo 'rpcpassword=test' > $(BITCOINDATADIR)/bitcoin.conf
	bitcoind -daemon $(BITCOINPARAMS)
	@sleep 3s;
	@echo "Generate 50 blocks"
  # Wait for the server to start
	while ! bitcoin-cli $(BITCOINPARAMS) generate 50 > /dev/null; do sleep 3s; done


.PHONY: preform-test
perform-test:
	@echo
	@echo "Start $(TESTNAME)"
	$(BINDIR)/$(TESTNAME) --rpc-password=test --rpc-port=18332
	@bitcoin-cli $(BITCOINPARAMS) stop 2> /dev/null || true # server is probably alredy stoped by test programm
	sleep 5s;

.PHONY: clean-test
clean-test:
	@echo
	@echo "Bitcoin Core logs:"
	cat $(BITCOINDATADIR)/regtest/debug.log
	@echo "Remove datadir"
	rm -rf $(BITCOINDATADIR)


.PHONY: test
test: prep-test perform-test clean-test


# ---------- clean ----------------
.PHONY: clean
clean:
	$(RM) $(SRCDIR)/*.o $(SRCDIR)/test/*.o $(LIBDIR)/*.so* $(BINDIR)/$(NAME)_test
	$(RM) -d $(LIBDIR) $(BINDIR)

# ---------- install --------------
.PHONY: install
install:
	@echo "Installing to $(INSTALL_PREFIX)/"
	$(INSTALL) $(LIBDIR)/lib$(NAME).so.$(VERSION) $(INSTALL_LIBPATH)
	ldconfig  -n $(INSTALL_LIBPATH)
	ln -fs lib$(NAME).so.$(MAJOR) $(INSTALL_LIBPATH)/lib$(NAME).so
	ldconfig
	$(INSTALL_DATA) $(SRCDIR)/$(NAME).h $(INSTALL_HEADERPATH)
	@echo "Installing docs to $(INSTALL_DOCSPATH)/$(NAME)"
	mkdir -p $(INSTALL_DOCSPATH)/$(NAME)
	$(INSTALL_DATA) $(DOCDIR)/*.md $(INSTALL_DOCSPATH)/$(NAME)
	$(INSTALL_DATA) CREDITS $(INSTALL_DOCSPATH)/$(NAME)
	$(INSTALL_DATA) LICENSE $(INSTALL_DOCSPATH)/$(NAME)
	$(INSTALL_DATA) Changelog.md $(INSTALL_DOCSPATH)/$(NAME)
	@echo "Installing man pages"
	@mkdir -p $(INSTALL_MANPATH)/man3
	$(INSTALL_DATA) $(DOCDIR)/man3/$(NAME)*.gz $(INSTALL_MANPATH)/man3

.PHONY: uninstall
uninstall:
	@echo "Removing from $(INSTALL_LIBPATH)/"
	@$(RM) $(INSTALL_LIBPATH)/lib$(NAME).so*
	@$(RM) $(INSTALL_HEADERPATH)/$(NAME).h
	@$(RM) $(INSTALL_MANPATH)/man3/$(NAME)*.gz
