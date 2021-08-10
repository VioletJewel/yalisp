CC = gcc
CFLAGS = -lreadline -Ilibs -Wno-abi
OBJDIR = .out
OUT = clisp
TESTOUT = testing/tests


# clisp
# =====

# primary target
clisp: $(OBJDIR)/lisp.o $(OBJDIR)/vector.o $(OBJDIR)/node.o
	$(CC) -o $(OUT) $(CFLAGS) $(OBJDIR)/lisp.o $(OBJDIR)/vector.o $(OBJDIR)/node.o


# object files
# ============

# build lisp object
$(OBJDIR)/lisp.o: lisp.c
	$(CC) $(CFLAGS) -c lisp.c -o $(OBJDIR)/lisp.o

# build vector library object
$(OBJDIR)/vector.o: libs/vector.c libs/vector.h
	$(CC) $(CFLAGS) -c libs/vector.c -o $(OBJDIR)/vector.o

# build node library object
$(OBJDIR)/node.o: libs/node.c libs/node.h
	$(CC) $(CFLAGS) -c libs/node.c -o $(OBJDIR)/node.o


# debugging
# =========

.PHONY: debug
debug: CFLAGS += -Wall -DDEBUG -g
debug: clisp


# testing
# =======

tests: CFLAGS += -Wall -DDEBUG -g
tests: $(OBJDIR)/tests.o $(OBJDIR)/vector.o $(OBJDIR)/node.o
	$(CC) $(CFLAGS) $(OBJDIR)/tests.o $(OBJDIR)/vector.o $(OBJDIR)/node.o -o $(TESTOUT)

$(OBJDIR)/tests.o: testing/tests.c
	$(CC) $(CFLAGS) -c testing/tests.c -o $(OBJDIR)/tests.o

.PHONY: debugtests
debugtests: CFLAGS += -Wall -DDEBUG -g
debugtests: tests


# cleanup
# =======

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(OUT) $(TESTOUT)
