CURRENT_DIR:=$(PWD)

WORK_DIR:=$(CURRENT_DIR)/_work
BUILD_DIR:=$(WORK_DIR)/build
INSTALL_DIR:=$(WORK_DIR)/install

CMAKE_OPTIONS=\
	-DCMAKE_BUILD_TYPE="Debug" \
	-DCMAKE_INSTALL_PREFIX:PATH="$(INSTALL_DIR)"

all: build

build: $(BUILD_DIR)
	(cd "$(BUILD_DIR)" && cmake $(CMAKE_OPTIONS) "$(CURRENT_DIR)" )
	(cd "$(BUILD_DIR)" && cmake --build . -- all)

$(BUILD_DIR):
	mkdir -p "$(BUILD_DIR)"

clean:
	rm -rf "$(WORK_DIR)"
