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

# apps - related commands
.PHONY: apps-build
apps-build: pl-build pl-install
	make -f ./apps/Makefile build

.PHONY: apps-install
apps-install:
	make -f ./apps/Makefile install

.PHONY: apps-purge
apps-purge:
	make -f ./apps/Makefile purge

# all builds
.PHONY: build
build: compiler-build pl-build apps-build

.PHONY: install
install: compiler-install pl-install apps-install

.PHONY: purge
purge: compiler-purge pl-purge apps-purge
