# The MIT License (MIT)
# Copyright (c) 2016 Marek Miller and Andrea Bernardo Ciddio (Code::Chunks)
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

VMAJOR := 0
VMINOR := 0
VPATCH := 0
VERSION := $(VMAJOR).$(VMINOR).$(VPATCH)

NAME := bitcoinrpc
SRCDIR := src
LIBDIR := .lib
BINDIR := bin

LDFLAGS := -luuid -ljansson -lcurl

CFLAGS := -fPIC -O3 -g -Wall -Werror -std=c99
CC := gcc

all: prep lib test

prep:
	@mkdir -p $(LIBDIR) $(BINDIR)

lib: $(LIBDIR)/lib$(NAME).so

$(LIBDIR)/lib$(NAME).so: $(LIBDIR)/lib$(NAME).so.$(VERSION)
	ldconfig -v -n $(LIBDIR)
	ln -s lib$(NAME).so.$(MAJOR) $(LIBDIR)/lib$(NAME).so

$(LIBDIR)/lib$(NAME).so.$(VERSION): $(SRCDIR)/$(NAME).o
	$(CC) -shared -Wl,-soname,lib$(NAME).so.$(MAJOR) \
	$^ -o $@ \
	-Wl,--copy-dt-needed-entries $(LDFLAGS)


test: $(NAME)_test

$(NAME)_test: lib
	$(CC) $(SRCDIR)/test/$(NAME)_test.c -o $(BINDIR)/$@ \
		-l$(NAME) -L$(LIBDIR) -I $(SRCDIR) \
		-Wl,-rpath=$(LIBDIR)

.PHONY: clean
clean:
	$(RM) $(SRCDIR)/*.o $(SRCDIR)/test/*.o $(LIBDIR)/*.so* $(BINDIR)/$(NAME)_test
	$(RM) -d $(LIBDIR) $(BINDIR)
