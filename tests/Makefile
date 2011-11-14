default: build

build:
	cd .. ; make test_compile ; cd tests

t_%: build
	sh runtests.sh $*

stress_%: build
	./spawntimer ./$*.run --no-debug-print $*/stresstest
