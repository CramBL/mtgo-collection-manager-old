# Default flags
# MTGOPARSER_GENERATOR := "Ninja Multi-Config"
MTGOPARSER_IPO := Off
MTGOPARSER_BUILD_MODE := Release

# Set generator to "Ninja Multi-Config" for unix-like systems.
ifeq ($(shell uname -s),Linux)
    OS_TYPE := Linux
	MTGOPARSER_GENERATOR := "Ninja Multi-Config"
else ifeq ($(shell uname -s),Darwin)
    OS_TYPE := macOS
	MTGOPARSER_GENERATOR := "Ninja Multi-Config"
else ifeq ($(shell uname -o),Msys)
    OS_TYPE := Windows
	MTGOPARSER_GENERATOR := "Visual Studio 17 2022"
else
    OS_TYPE := Unknown
	MTGOPARSER_GENERATOR := "Ninja Multi-Config"
endif

# Minimum supported versions
RUST_MIN_VERSION := 1.70.0
GO_MIN_VERSION := 1.20
CMAKE_MIN_VERSION := 3.20

# Get version from a unix-like terminal
RUST_VERSION := $(shell rustc --version | cut -d' ' -f2)
GO_VERSION := $(shell go version | cut -d' ' -f3 | sed 's/go//')
CMAKE_VERSION := $(shell cmake --version | cut -d' ' -f3 | head -n 1)

.PHONY: all
all:\
	show-versions \
	build-mtgogetter \
	build-mtgoparser \
	build-mtgoupdater \

.PHONY: test
test:\
	show-versions \
	test-mtgogetter \
	test-mtgoparser \
	test-mtgoupdater \

.PHONY: show-versions
show-versions:
	@echo "Operating System: $(OS_TYPE)"
	@echo "Rust : $(RUST_VERSION) (min. $(RUST_MIN_VERSION))"
	@echo "Go   : $(GO_VERSION) (min. $(GO_MIN_VERSION))"
	@echo "CMake: $(CMAKE_VERSION) (min. $(CMAKE_MIN_VERSION))"

.PHONY: build-mtgogetter
build-mtgogetter:
	@echo "==> Building MTGO Getter..."
	go build -C mtgogetter -v
	@echo "=== Done building MTGO Getter ==="

.PHONY: test-mtgogetter
test-mtgogetter:
	@echo "==> Testing MTGO Getter..."
	go test -C mtgogetter -v ./...
	@echo "=== Done testing MTGO Getter ==="

.PHONY: build-mtgoparser
build-mtgoparser:
	@echo "==> Building MTGO Parser..."
	cd mtgoparser && cmake -S . -B build -G "$(MTGOPARSER_GENERATOR)" -Dmtgoparser_ENABLE_IPO=$(MTGOPARSER_IPO) -DCMAKE_BUILD_TYPE:STRING=$(MTGOPARSER_BUILD_MODE)
	cd mtgoparser && cmake --build build --config $(MTGOPARSER_BUILD_MODE)
	@echo "=== Done building MTGO Parser ==="

.PHONY: test-mtgoparser
test-mtgoparser:
	@echo "==> Testing MTGO Parser..."
	cd mtgoparser/build && ctest
	@echo "=== Done testing MTGO Parser ==="


.PHONY: build-mtgoupdater
build-mtgoupdater:
	@echo "==> Building MTGO Updater..."
	cd mtgoupdater && cargo build
	@echo "=== Done building MTGO Updater ==="

.PHONY: build-mtgoupdater
test-mtgoupdater:
	@echo "==> Testing MTGO Updater..."
	cd mtgoupdater && cargo test -- --nocapture
	@echo "=== Done testing MTGO updater ==="

.PHONY: clean
clean:
	rm -rf mtgoparser/build
	@echo "mtgoparser cleaned"
	cd mtgoupdater && cargo clean
	cd mtgogetter && go clean
