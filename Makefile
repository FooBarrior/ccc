PARAMS = -g3
CC = gcc -std=gnu99 $(PARAMS)

SRC = lexer.c
OBJS = $(SRC:%.c=.objs/%.o)

T_BIN = $(SRC:%.c=tests/%.run)

all: ccc test_compile

ccc: ccc.c $(OBJS)
	$(CC) -occc $< $(OBJS)

.objs/:
	mkdir -p .objs

.objs/%.o: %.c
	$(CC) -c -o$@ $<

test_compile: $(T_BIN) tests/spawntimer

tests/spawntimer: tests/spawntimer.c
	$(CC) -o$@ $<

tests/%.run: %.c
	$(CC) -DCCC_TEST -o$@ $<
    
runtests: test_compile
	cd tests && sh runtests.sh $(SRC:%.c=%)

clean:
	rm -rf .objs/*
	rm ccc
	rm tests/*.run
	rm tests/spawntimer
