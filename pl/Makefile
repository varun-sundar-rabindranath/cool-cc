.PHONY: build
build:
	mkdir -p ${PL_BUILD_DIR}
	cmake -S ${PL_SOURCE_DIR} -B  ${PL_BUILD_DIR}
	cmake --build ${PL_BUILD_DIR} -j 8

.PHONY: install
install:
	cmake --install ${PL_BUILD_DIR} --prefix ${PL_INSTALL_DIR}

.PHONY: build
purge:
	rm -rf ${PL_BUILD_DIR}
