APP := 2DArticulatedBodySimulation
BUILD_DIR := build
SRC := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.h)

CXX := c++
CXXFLAGS := -O2 -std=c++17
RAYLIB_FLAGS := $(shell pkg-config --cflags --libs raylib)

$(BUILD_DIR)/$(APP): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@ $(RAYLIB_FLAGS)

.PHONY: run clean

run: $(BUILD_DIR)/$(APP)
	./$(BUILD_DIR)/$(APP)

clean:
	rm -rf ./build
