CXX := clang++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -pedantic -Iinclude

CONTROLLER_SRC := src/thermal_controller.cpp
TEST_SRC := tests/test_therm_controller.cpp
DEMO_SRC := src/main.cpp

TEST_TARGET := test_therm_controller
DEMO_TARGET := thermal_controller

all: $(TEST_TARGET) $(DEMO_TARGET)

$(TEST_TARGET): $(CONTROLLER_SRC) $(TEST_SRC) include/thermal_controller.hpp
	$(CXX) $(CXXFLAGS) $(CONTROLLER_SRC) $(TEST_SRC) -o $(TEST_TARGET)

$(DEMO_TARGET): $(CONTROLLER_SRC) $(DEMO_SRC) include/thermal_controller.hpp
	$(CXX) $(CXXFLAGS) $(CONTROLLER_SRC) $(DEMO_SRC) -o $(DEMO_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

demo: $(DEMO_TARGET)
	./$(DEMO_TARGET)

clean:
	rm -f $(TEST_TARGET) $(DEMO_TARGET) src/*.o tests/*.o

.PHONY: all test demo clean