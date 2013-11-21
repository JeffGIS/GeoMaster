
# Eventually I'd like a nice Makefile here made with autoconf, but for
# now, we'll use this.

all:
	@echo "You must give a target:"
	@echo "                       Make clean"
	@echo "                       Make install"
	@echo "                       Make tests"

realclean: clean
	rm -f bin/*
	rm -f lib/*.a
	rm -f etc/*

clean: test-clean
	@echo ::: cleaning source directory :::
	${MAKE} -C src   clean
	@echo ::: cleaning tests directory :::
	${MAKE} -C tests   clean

test:
	@echo ::: running all tests :::
	${MAKE} -C tests all

tests: test

test-clean:
	@echo ::: cleaning test directory :::
	${MAKE} -C tests clean

install:
	${MAKE} -C src install

export:
	mv ipw/scratch ipw/scratch-ours
	mkdir ipw/scratch; chmod 777 ipw/scratch; chown ipw ipw/scratch
	rm ipw/VERSION
	date >ipw/VERSION
	/usr/local/gnu/bin/tar -czX ipw/exclude.list -f ipw-dist.tar.gz ipw
	rmdir ipw/scratch
	mv ipw/scratch-ours ipw/scratch
