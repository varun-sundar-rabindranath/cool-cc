# compiler related commands
.PHONY: compiler-build
compiler-build:
	make -f ./compiler/Makefile build

.PHONY: compiler-install
compiler-install:
	make -f ./compiler/Makefile install

.PHONY: compiler-purge
compiler-purge:
	make -f ./compiler/Makefile purge

# pl - related commands
.PHONY: pl-build
pl-build: compiler-build compiler-install
	make -f ./pl/Makefile build

.PHONY: pl-install
pl-install:
	make -f ./pl/Makefile install

.PHONY: pl-purge
pl-purge:
	make -f ./pl/Makefile purge

# all builds
.PHONY: build
build: compiler-build pl-build

.PHONY: install
install: compiler-install pl-install

.PHONY: purge
purge: compiler-purge pl-purge
