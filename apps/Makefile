.PHONY: build
build:
	mkdir -p ${APPS_BUILD_DIR}
	cmake -S ${APPS_SOURCE_DIR} -B  ${APPS_BUILD_DIR}
	cmake --build ${APPS_BUILD_DIR} -j 8

.PHONY: install
install:
	cmake --install ${APPS_BUILD_DIR} --prefix ${APPS_INSTALL_DIR}

.PHONY: build
purge:
	rm -rf ${APPS_BUILD_DIR}
