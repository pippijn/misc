#.SILENT:

CC = gcc-4.5

OPTFLAGS	= -O3 -DTIMING
PROFILE_BEGIN	= -fprofile-generate
PROFILE_MID	= -fprofile-use
PROFILE_END	= 

CPPFLAGS	= -Iinclude
CFLAGS		= -MD --coverage -ggdb3

LINT		= splint -checks $(CPPFLAGS)

SOURCES	:= src/xmalloc.c src/raycasting.c src/main.c
OBJECTS	:= $(SOURCES:.c=.o)

TESTIN	:= $(wildcard testsuite/*.txt)
TESTOUT	:= $(TESTIN:.txt=.out)

all-nobench: clean lint coverage

all: clean lint coverage timing-small

hoff-test: rc_test
	test `./rc_test 0 0 testsuite/hoff.txt` = "outside"
	test `./rc_test 100 100 testsuite/hoff.txt` = "outside"
	test `./rc_test 1 3 testsuite/hoff.txt` = "inside"
	test `./rc_test 4 5 testsuite/hoff.txt` = "inside"
	test `./rc_test 5 7 testsuite/hoff.txt` = "inside"
	test `./rc_test 6 7 testsuite/hoff.txt` = "outside"
	test `./rc_test 7 7 testsuite/hoff.txt` = "outside"
	test `./rc_test 8 7 testsuite/hoff.txt` = "inside"
	test `./rc_test 9 5 testsuite/hoff.txt` = "outside"
	test `./rc_test 10 5 testsuite/hoff.txt` = "outside"
	test `./rc_test 11 5 testsuite/hoff.txt` = "outside"
	test `./rc_test 12 5 testsuite/hoff.txt` = "inside"
	test `./rc_test 12 10 testsuite/hoff.txt` = "inside"
	test `./rc_test 13 10 testsuite/hoff.txt` = "outside"
	test `./rc_test 12 11 testsuite/hoff.txt` = "outside"
	test `./rc_test 17 3 testsuite/hoff.txt` = "inside"

our-test: rc_test
	# diamond
	test `./rc_test 4 1 testsuite/test0001.txt` = "inside"
	test `./rc_test 6 1 testsuite/test0001.txt` = "inside"
	test `./rc_test 4 9 testsuite/test0001.txt` = "inside"
	test `./rc_test 6 9 testsuite/test0001.txt` = "inside"
	test `./rc_test 7 9 testsuite/test0001.txt` = "outside"
	test `./rc_test 3 9 testsuite/test0001.txt` = "outside"
	test `./rc_test 7 1 testsuite/test0001.txt` = "outside"
	test `./rc_test 3 1 testsuite/test0001.txt` = "outside"
	# M
	test `./rc_test 6 5 testsuite/test0002.txt` = "inside"
	test `./rc_test 5 5 testsuite/test0002.txt` = "inside"
	test `./rc_test 4 5 testsuite/test0002.txt` = "inside"
	test `./rc_test 5 6 testsuite/test0002.txt` = "outside"
	test `./rc_test 11 5 testsuite/test0002.txt` = "outside"
	# weird polygon
	test `./rc_test 0 1 testsuite/test0004.txt` = "inside"
	test `./rc_test 1 0 testsuite/test0004.txt` = "outside"
	test `./rc_test 6 5 testsuite/test0004.txt` = "outside"
	test `./rc_test 7 5 testsuite/test0004.txt` = "inside"
	test `./rc_test 8 5 testsuite/test0004.txt` = "inside"
	test `./rc_test 10 5 testsuite/test0004.txt` = "inside"
	test `./rc_test 11 5 testsuite/test0004.txt` = "outside"
	# square
	test `./rc_test 0 5 testsuite/test0005.txt` = "inside"
	test `./rc_test 10 5 testsuite/test0005.txt` = "inside"
	test `./rc_test 5 0 testsuite/test0005.txt` = "inside"
	test `./rc_test 5 10 testsuite/test0005.txt` = "inside"
	# reverse square
	test `./rc_test 0 5 testsuite/test0006.txt` = "inside"
	test `./rc_test 10 5 testsuite/test0006.txt` = "inside"
	test `./rc_test 5 0 testsuite/test0006.txt` = "inside"
	test `./rc_test 5 10 testsuite/test0006.txt` = "inside"
	# weird polygon 2
	test `./rc_test 3 5 testsuite/test0007.txt` = "outside"
	test `./rc_test 7 5 testsuite/test0007.txt` = "inside"
	test `./rc_test 8 5 testsuite/test0007.txt` = "inside"
	test `./rc_test 6 3 testsuite/test0007.txt` = "outside"
	test `./rc_test 7 3 testsuite/test0007.txt` = "inside"
	test `./rc_test 8 3 testsuite/test0007.txt` = "inside"

coverage: check hoff-test our-test
	./rc_test
	-./rc_test "" 5 nonexisting.txt
	-./rc_test 5 5 nonexisting.txt
	-./rc_test a 5 testsuite/test0001.txt
	./rc_test 9223372036854 5 testsuite/test0001.txt
	-./rc_test 9223372036855 5 testsuite/test0001.txt
	-sh -c 'ulimit -v 8192 && ./rc_test 5 5 testsuite/big.txt'
	gcov $(SOURCES) -o src

check: $(TESTOUT)

%.out: %.txt rc_test
	-./rc_test 5 5 $<

combined.c: src/main-bench.c src/raycasting.c src/xmalloc.c
	$(RM) $@
	for i in $+; do			\
		echo "// $$i" >> $@;	\
		cat $$i >> $@;		\
	done

timing-small: combined.c
	$(CC) $(CPPFLAGS) $(OPTFLAGS) $(PROFILE_BEGIN) -o rc_test combined.c
	./rc_test 99850 20000 testsuite/generated.txt
	$(CC) $(CPPFLAGS) $(OPTFLAGS) $(PROFILE_MID) -o rc_test combined.c
	./rc_test 99850 20000 testsuite/generated.txt

timing-noprofile: combined.c
	$(CC) $(CPPFLAGS) $(OPTFLAGS) -o rc_test combined.c
	./rc_test 99850 20000 testsuite/generated.txt

timing: rc_test.combined
	bash -c 'time ./$< 5 5 testsuite/big.txt'

leaks: rc_test
	valgrind --leak-check=full ./$< 5 5 testsuite/small.txt

rc_test.combined: $(SOURCES)
	$(CC) $(CPPFLAGS) $(OPTFLAGS) -o $@ $(SOURCES)

rc_test: $(OBJECTS)
	$(LINK.c) -o $@ $(OBJECTS)

lint: $(SOURCES)
	$(LINT) $(SOURCES) 2> rc_test.lint || cat rc_test.lint

clean:
	$(RM) rc_test $(OBJECTS)
	$(RM) rc_test.lint
	$(RM) $(SOURCES:.c=.gcda)
	$(RM) $(SOURCES:.c=.gcno)
	$(RM) $(SOURCES:.c=.d)

-include $(wildcard src/*.d)
