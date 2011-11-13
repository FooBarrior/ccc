CC = gcc -std=gnu99 -g3

SRC = lexer.c
OBJS = $(SRC:%.c=.objs/%.o)

T_BIN = $(SRC:%.c=%.run)

ccc: .objs/ $(OBJS)
	$(CC) -occc $(OBJS)

.objs/:
	mkdir -p .objs

.objs/%.o: %.c
	$(CC) -c -o$@ $<


test_compile: $(T_BIN)

%.run: %.c
	$(CC) -DCCC_TEST -otests/$@ $<
    
runtests: test_compile
	cd tests
	sh runtests.sh
