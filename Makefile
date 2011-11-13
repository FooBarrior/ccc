CC = gcc -std=gnu99 -g3

SRC = lexer.c
OBJS = $(SRC:%.c=.objs/%.o)

T_BIN = $(SRC:%.c=tests/%.run)

ccc: .objs/ $(OBJS)
	$(CC) -occc $(OBJS)

.objs/:
	mkdir -p .objs

.objs/%.o: %.c
	$(CC) -c -o$@ $<


test_compile: $(T_BIN)

tests/%.run: %.c
	$(CC) -DCCC_TEST -o$@ $<
    
runtests: test_compile
	cd tests && sh runtests.sh $(SRC:%.c=%)

