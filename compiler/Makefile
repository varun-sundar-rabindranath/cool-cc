
.PHONY: build
build:
	mkdir -p ${COOLCC_BUILD_DIR}
	cmake -S ${COOLCC_SOURCE_DIR} -B  ${COOLCC_BUILD_DIR}
	cmake --build ${COOLCC_BUILD_DIR} -j 8

.PHONY: install
install:
	cmake --install ${COOLCC_BUILD_DIR} --prefix ${COOLCC_INSTALL_DIR}

.PHONY: purge
purge:
	rm -rf ${COOLCC_BUILD_DIR}

## Grammar related builds
.PHONY: build-arith
build-arith:
	make -f ./pl/Makefile build

.PHONY: install-arith
install-arith:
	make -f ./pl/Makefile install

.PHONY: purge-arith
purge-arith:
	make -f ./pl/Makefile purge
