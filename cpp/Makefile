# Build settings
CXX := clang++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -pedantic -Iinclude

# Source files needed for the test program
SRC := src/thermal_controller.cpp \
       tests/test_therm_controller.cpp

# Object files created from the source files
OBJS := $(SRC:.cpp=.o)

# Test executable created in cpp/
TARGET := test_therm_controller

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)