.PHONY: build
build:
	mkdir -p ${COMPILER_BUILD_DIR}
	cmake -S ${COMPILER_SOURCE_DIR} -B  ${COMPILER_BUILD_DIR}
	cmake --build ${COMPILER_BUILD_DIR} -j 8

.PHONY: install
install:
	cmake --install ${COMPILER_BUILD_DIR} --prefix ${COMPILER_INSTALL_DIR}

.PHONY: purge
purge:
	rm -rf ${COMPILER_BUILD_DIR}
