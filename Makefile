# Default flags
BUILD_MODE := Debug
MTGOPARSER_GENERATOR := "Ninja Multi-Config"
MTGOPARSER_BUILD_MODE := Release
MTGOPARSER_ENABLE_COV := false
MTGOPARSER_USER_LINKER := On
MTGOPARSER_LINKER := mold
MTGOPARSER_EXCLUDE_BOOST_LIBS := "serialization;asio;json;graph;log;property_tree;wave;contract;coroutine;date_time;fiber;locale;thread;type_erasure;test;url;python;compute;crc;dll;endian;lamda;fusion;geometry;gil;regex;iostreams;filesystem;program_options;random;math;multiprecision;mysql;stacktrace;"

# Set generator to "Ninja Multi-Config" for unix-like systems.
ifeq ($(shell uname -s),Linux)
    OS_TYPE := Linux
	MTGOPARSER_GENERATOR := "Ninja Multi-Config"
	MTGOPARSER_IPO := On
	ifeq ($(shell which mold),)
    	$(error "No mold in PATH:$(PATH), please install mold for improved link times. Installable for most systems with `apt install mold`")
	endif

else ifeq ($(shell uname -s),Darwin)
    OS_TYPE := macOS
	MTGOPARSER_GENERATOR := "Ninja Multi-Config"
	MTGOPARSER_IPO := Off
	MTGOPARSER_USER_LINKER := Off
else ifeq ($(shell uname -o),Msys)
    $(error Operating System is detected as Windows (Msys). This Makefile is not intended for Windows systems. Use the powershell script wmake.ps1 instead)
else
    OS_TYPE := Unknown
	MTGOPARSER_GENERATOR := "Ninja Multi-Config"
	MTGOPARSER_IPO := On
endif



# Minimum supported versions
RUST_MIN_VERSION := 1.70.0
GO_MIN_VERSION := 1.20
CMAKE_MIN_VERSION := 3.20
GCC_MIN_VERSION := 12.1.0
LLVM_MIN_VERSION := 15.0.3

# Get version from a unix-like terminal
RUST_VERSION := $(shell rustc --version | grep -o '[0-9]*\.[0-9]*\.[0-9]*')
ifeq ($(RUST_VERSION),)
	RUST_VERSION := "NOT FOUND"
endif

	GO_VERSION := $(shell go version | cut -d' ' -f3 | sed 's/go//')
ifeq ($(GO_VERSION),)
	GO_VERSION := "NOT FOUND"
endif

CMAKE_VERSION := $(shell cmake --version | cut -d' ' -f3 | head -n 1)
ifeq ($(CMAKE_VERSION),)
	CMAKE_VERSION := "NOT FOUND"
endif

CLANG_VERSION := $(shell clang --version | grep -o '[0-9]*\.[0-9]*\.[0-9]*' | head -n 1)
ifeq ($(CLANG_VERSION),)
	CLANG_VERSION := "NOT FOUND"
endif

GCC_VERSION := $(shell gcc --version | grep -o '[0-9]*\.[0-9]*\.[0-9]*' | head -n 1)
ifeq ($(GCC_VERSION),)
	GCC_VERSION := "NOT FOUND"
endif

MTGOPARSER_NINJA_VERSION := $(shell ninja --version | grep -o '[0-9]*\.[0-9]*\.[0-9]*' | head -n 1)
ifeq ($(MTGOPARSER_NINJA_VERSION),)
	MTGOPARSER_NINJA_VERSION := "NOT FOUND"
endif

ifeq ($(MTGOPARSER_LINKER),mold)
	MOLD_VERSION := $(shell mold --version | grep -o '[0-9]*\.[0-9]*\.[0-9]*' | head -n 1)
	ifeq ($(MOLD_VERSION),)
		MOLD_VERSION := "NOT FOUND"
	endif
endif


.PHONY: all
all:
	@echo "----------------------------------"
	@echo "==> Building all targets"
	@echo "----------------------------------"
	$(call fn_show_versions)
	@echo "==> Building MTGO Getter..."
	$(call fn_build_mtgogetter)
	@echo "=== Done building MTGO Getter ==="
	@echo "==> Building MTGO Parser..."
	$(call fn_build_mtgoparser)
	@echo "=== Done building MTGO Parser ==="
	@echo "==> Building MTGO Updater..."
	$(call fn_build_mtgoupdater)
	@echo "=== Done building MTGO Updater ==="
	@echo "==> Building MTGO GUI..."
	$(call fn_build_mtgogui)
	@echo "=== Done building MTGO GUI ==="
	@echo "================================= "
	@echo "=== Done building all targets === "
	@echo "================================= "

.PHONY: build-integration integration
build-integration integration:
	@echo "----------------------------------------"
	@echo "==> Building all targets for integration"
	@echo "----------------------------------------"
	$(call fn_show_versions)
	@echo "==> Building MTGO Getter..."
	$(call fn_build_mtgogetter)
	@echo "=== Done building MTGO Getter ==="
	@echo "==> Building MTGO Parser..."
	$(call fn_build_mtgoparser_integration)
	@echo "=== Done building MTGO Parser ==="
	@echo "==> Building MTGO Updater..."
	$(call fn_build_mtgoupdater)
	@echo "=== Done building MTGO Updater ==="
	@echo "==> Building MTGO GUI..."
	$(call fn_build_mtgogui)
	@echo "=== Done building MTGO GUI ==="
	@echo "================================================= "
	@echo "=== Done building all targets for integration === "
	@echo "================================================= "

.PHONY: test
test:
	@echo "----------------------------------"
	@echo "==> Testing all targets"
	@echo "----------------------------------"
	@echo "==> Testing MTGO Getter..."
	$(call fn_test_mtgogetter)
	@echo "=== Done testing MTGO Getter ==="
	@echo "==> testing MTGO Parser..."
	$(call fn_test_mtgoparser)
	@echo "=== Done testing MTGO Parser ==="
	@echo "==> Testing MTGO Updater..."
	$(call fn_test_mtgoupdater)
	@echo "=== Done testing MTGO Updater ==="
	@echo "==> Testing MTGO GUI..."
	$(call fn_test_mtgogui)
	@echo "=== Done testing MTGO GUI ==="
	@echo "================================= "
	@echo "=== Done testing all targets === "
	@echo "================================= "

.PHONY: show-versions versions
show-versions versions:
	@$(call fn_show_versions)


.PHONY: build-mtgogetter mtgogetter
build-mtgogetter mtgogetter:
	@echo "==> Building MTGO Getter..."
	$(call fn_build_mtgogetter)
	@echo "=== Done building MTGO Getter ==="

.PHONY: test-mtgogetter
test-mtgogetter:
	@echo "==> Testing MTGO Getter..."
	$(call fn_test_mtgogetter)
	@echo "=== Done testing MTGO Getter ==="

.PHONY: build-mtgoparser mtgoparser
build-mtgoparser mtgoparser:
	@echo "==> Building MTGO Parser..."
	$(call fn_build_mtgoparser)
	@echo "=== Done building MTGO Parser ==="

# For CI, turning off warnings as errors and other things (trusting the MTGO Parser CI for the more rigorous testing and static analysis)
.PHONY: build-mtgoparser-integration mtgoparser-integration
build-mtgoparser-integration mtgoparser-integration:
	@echo "==> Building MTGO Parser..."
	$(call fn_build_mtgoparser_integration)
	@echo "=== Done building MTGO Parser ==="

.PHONY: bench-mtgoparser
bench-mtgoparser:
	@echo "==> Running benchmarks for MTGO Parser..."
	$(call fn_bench_mtgoparser)
	@echo "=== Done running benchmarks MTGO Parser ==="

.PHONY: test-mtgoparser
test-mtgoparser:
	@echo "==> Testing MTGO Parser..."
	$(call fn_test_mtgoparser)
	@echo "=== Done testing MTGO Parser ==="


.PHONY: build-mtgoupdater
build-mtgoupdater:
	@echo "==> Building MTGO Updater..."
	$(call fn_build_mtgoupdater)
	@echo "=== Done building MTGO Updater ==="

.PHONY: test-mtgoupdater
test-mtgoupdater:
	@echo "==> Testing MTGO Updater..."
	$(call fn_test_mtgoupdater)
	@echo "=== Done testing MTGO updater ==="

.PHONY: build-mtgogui mtgogui
build-mtgogui mtgogui:
	@echo "==> Building MTGO GUI..."
	$(call fn_build_mtgogui)
	@echo "=== Done building MTGO GUI ==="


.PHONY: test-mtgogui
test-mtgogui:
	@echo "==> Testing MTGO GUI..."
	$(call fn_test_mtgogui)
	@echo "=== Done testing MTGO GUI ==="


.PHONY: clean
clean:
	rm -rf mtgoparser/build
	@echo "mtgoparser cleaned"
	cd mtgoupdater && cargo clean
	cd mtgogetter && go clean
	cd mtgogui && cargo clean

#
##############################################################
## Defines for building, testing, benchmarking sub projects ##
##############################################################
#

# Show versions
define fn_show_versions
		@echo "Operating System: $(OS_TYPE)"
		@echo "   Rust : $(RUST_VERSION) (min. $(RUST_MIN_VERSION))"
		@echo "   Go   : $(GO_VERSION) (min. $(GO_MIN_VERSION))"
		@echo "   C++"
		@echo "     - LLVM     : ${CLANG_VERSION} (min. ${LLVM_MIN_VERSION})"
		@echo "     - GCC      : ${GCC_VERSION} (min. ${GCC_MIN_VERSION})"
		@echo "     - CMake    : $(CMAKE_VERSION) (min. $(CMAKE_MIN_VERSION))"
		@if [ $(MTGOPARSER_GENERATOR) = "Ninja Multi-Config" ]; then \
			echo "     - Generator: $(MTGOPARSER_GENERATOR) $(MTGOPARSER_NINJA_VERSION)"; \
		else \
			echo "     - Generator: $(MTGOPARSER_GENERATOR)"; \
		fi
		@if [ "$(MTGOPARSER_LINKER)" = "mold" ]; then \
			echo "     - Linker   : mold $(MOLD_VERSION)"; \
		fi
endef

##################################
# MTGO Getter - Build & Test     #
##################################
define fn_build_mtgogetter
	go build -C mtgogetter -v
endef

define fn_test_mtgogetter
	go test -C mtgogetter -v ./...
endef



##########################################
# MTGO Parser - Build, test, & benchmark #
##########################################

# Preferred way to build MTGO Parser
define fn_build_mtgoparser
	cd mtgoparser && cmake -S . -B build \
	    -G $(MTGOPARSER_GENERATOR) \
	    -Dmtgoparser_ENABLE_IPO=$(MTGOPARSER_IPO) \
	    -DCMAKE_BUILD_TYPE:STRING=$(MTGOPARSER_BUILD_MODE) \
	    -Dmtgoparser_ENABLE_COVERAGE:BOOL=$(MTGOPARSER_ENABLE_COV) \
	    -DBOOST_EXCLUDE_LIBRARIES=$(MTGOPARSER_EXCLUDE_BOOST_LIBS) \
	    -DUSER_LINKER_OPTION=$(MTGOPARSER_LINKER) \
	    -Dmtgoparser_ENABLE_USER_LINKER:BOOL=$(MTGOPARSER_USER_LINKER)
	cd mtgoparser && cmake --build build --config $(MTGOPARSER_BUILD_MODE)
endef

# Build for integration, disable warnings as errors, and some linting.
define fn_build_mtgoparser_integration
	cd mtgoparser && cmake -S . -B build \
	    -G $(MTGOPARSER_GENERATOR) \
	    -Dmtgoparser_DEPLOYING_BINARY=On \
	    -Dmtgoparser_ENABLE_IPO=$(MTGOPARSER_IPO) \
	    -DCMAKE_BUILD_TYPE:STRING=$(MTGOPARSER_BUILD_MODE) \
	    -Dmtgoparser_ENABLE_COVERAGE:BOOL=$(MTGOPARSER_ENABLE_COV) \
	    -DBOOST_EXCLUDE_LIBRARIES=$(MTGOPARSER_EXCLUDE_BOOST_LIBS) \
	    -Dmtgoparser_WARNINGS_AS_ERRORS:BOOL=OFF \
	    -Dmtgoparser_ENABLE_CLANG_TIDY:BOOL=OFF \
	    -Dmtgoparser_ENABLE_CPPCHECK:BOOL=OFF \
	    -DUSER_LINKER_OPTION=$(MTGOPARSER_LINKER) \
	    -Dmtgoparser_ENABLE_USER_LINKER:BOOL=$(MTGOPARSER_USER_LINKER)
	cd mtgoparser && cmake --build build --config $(MTGOPARSER_BUILD_MODE)
endef

define fn_test_mtgoparser
	cd mtgoparser/build && ctest --output-on-failure
endef

define fn_bench_mtgoparser
	cd mtgoparser/build/test && ./$(MTGOPARSER_BUILD_MODE)/benchmark_xml_parse [.]
endef


##################################
# MTGO Updater - Build & Test    #
##################################
define fn_build_mtgoupdater
	@if [ $(BUILD_MODE) = Release ]; then 		 \
		cd mtgoupdater && cargo build --release; \
	else                                         \
		cd mtgoupdater && cargo build; 			 \
	fi
endef

define fn_test_mtgoupdater
	cd mtgoupdater && cargo test -- --nocapture
endef


##################################
# MTGO GUI - Build & Test    	 #
##################################
define fn_build_mtgogui
	@if [ $(BUILD_MODE) = Release ]; then    \
		cd mtgogui && cargo build --release; \
	else									 \
		cd mtgogui && cargo build;           \
	fi
endef

define fn_test_mtgogui
	@if [ $(uname -s) = Darwin ]; then    								      \
		echo "WARNING CARGO TEST FOR FLTK ARE CURRENTLY NOT WORKING ON MACOS";\
	else																	  \
		cd mtgogui && cargo test -- --nocapture;							  \
	fi
endef