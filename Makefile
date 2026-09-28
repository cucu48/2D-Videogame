BUILD_DIR := build
TARGET := $(BUILD_DIR)/game

CXX ?= g++
CPPFLAGS += $(shell pkg-config --cflags glew 2>/dev/null)
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Wpedantic -O0 -g
LDLIBS += $(shell pkg-config --libs glew 2>/dev/null) -lglfw -lSOIL

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) -r $(BUILD_DIR) $(TARGET)

-include $(DEPS)

install-debian:
	sudo apt install build-essential pkg-config libglew-dev libglfw3-dev libglm-dev libsoil-dev libgl1-mesa-dev mesa-utils
