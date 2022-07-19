
.PHONY: build
build:
	mkdir -p ${COOLCC_BUILD_DIR}
	cmake -S ${COOLCC_SOURCE_DIR} -B  ${COOLCC_BUILD_DIR}
	cmake --build ${COOLCC_BUILD_DIR}

.PHONY: install
install:
	cmake --install ${COOLCC_BUILD_DIR} --prefix ${COOLCC_INSTALL_DIR}

.PHONY: build
purge:
	rm -rf ${COOLCC_BUILD_DIR}
