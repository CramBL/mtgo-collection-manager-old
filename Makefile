# Default flags
# MTGOPARSER_GENERATOR := "Ninja Multi-Config"
MTGOPARSER_IPO := On
MTGOPARSER_BUILD_MODE := Release	

# Set generator to "Ninja Multi-Config" for unix-like systems.
platform_id != uname
MTGOPARSER_GENERATOR != if [ $(platform_id) = Linux ] || \
    [ $(platform_id) = Darwin ]; then \
        echo "Ninja Multi-Config"; \
    else \
        echo "Visual Studio 17 2022"; \
    fi

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
	show_versions \
	build_mtgogetter \
	build_mtgoparser \
	build_mtgoupdater \

.PHONY: test
test:\
	show_versions \
	test_mtgogetter \
	test_mtgoparser \
	test_mtgoupdater \

.PHONY: show_versions
show_versions:
	@echo "Checking Rust, Go, and CMake versions..."
	@echo "Rust : v$(RUST_VERSION) (min. $(RUST_MIN_VERSION))"
	@echo "Go   : v$(GO_VERSION) (min. $(GO_MIN_VERSION))"
	@echo "CMake: v$(CMAKE_VERSION) (min. $(CMAKE_MIN_VERSION))"

.PHONY: build_mtgogetter
build_mtgogetter:
	@echo "==> Building MTGO Getter..."
	go build -C mtgogetter -v
	@echo "=== Done building MTGO Getter ==="

.PHONY: test_mtgogetter
test_mtgogetter:
	@echo "==> Testing MTGO Getter..."
	go test -C mtgogetter -v ./...
	@echo "=== Done testing MTGO Getter ==="

.PHONY: build_mtgoparser
build_mtgoparser:
	@echo "==> Building MTGO Parser..."
	cd mtgoparser && cmake -S . -B build -G "$(MTGOPARSER_GENERATOR)" -Dmtgoparser_ENABLE_IPO=$(MTGOPARSER_IPO)
	cd mtgoparser && cmake --build build --config $(MTGOPARSER_BUILD_MODE)
	@echo "=== Done building MTGO Parser ==="

.PHONY: test_mtgoparser
test_mtgoparser:
	@echo "==> Testing MTGO Parser..."
	cd mtgoparser/build && ctest 
	@echo "=== Done testing MTGO Parser ==="


.PHONY: build_mtgoupdater
build_mtgoupdater:
	@echo "==> Building MTGO Updater..."
	cd mtgoupdater && cargo build
	@echo "=== Done building MTGO Updater ==="
	
.PHONY: build_mtgoupdater
test_mtgoupdater:
	@echo "==> Testing MTGO Updater..."
	cd mtgoupdater && cargo test -- --nocapture
	@echo "=== Done testing MTGO updater ==="

.PHONY: clean
clean:
	rm -rf mtgoparser/build
	@echo "mtgoparser cleaned"
	cd mtgoupdater && cargo clean
	cd mtgogetter && go clean
