.PHONY: config build install host-config host-build host-install

build:
	@echo "Running bind-test build"
	cmake --build --preset=$(BIND_TEST_BUILD_PRESET)

install:
	@echo "Running bind-test install"
	cmake --install .

host-build:
	@echo "Running bind-test build"
	cmake --build --preset=$(BIND_TEST_BUILD_PRESET)

host-install:
	@echo "Running bind-test install"
	cmake --install .
