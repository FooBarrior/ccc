PARAMS = -g3 -Wall
CC = gcc -std=gnu99 $(PARAMS)

SRC_WITH_HEADERS = lexer.c parser.c
SRC = $(SRC_WITH_HEADERS)
HEADERS = $(SRC_WITH_HEADERS:%.c=%.h)
OBJS = $(SRC:%.c=.objs/%.o)

T_BIN = $(SRC:%.c=tests/%.run)

all: ccc test_compile

ccc: ccc.c $(OBJS) $(HEADERS)
	$(CC) -occc $< $(OBJS)

.objs/:
	mkdir -p .objs

.objs/%.o: %.c
	$(CC) -c -o$@ $<

test_compile: $(T_BIN) tests/spawntimer

tests/spawntimer: tests/spawntimer.c
	$(CC) -o$@ $<

tests/%.run: %.c
	$(CC) -DCCC_TEST -o$@ $(OBJS:.objs/$*.o=) $<
    
runtests: test_compile
	cd tests && sh runtests.sh $(SRC:%.c=%)

clean:
	rm -rf .objs/*
	rm ccc
	rm tests/*.run
	rm tests/spawntimer
