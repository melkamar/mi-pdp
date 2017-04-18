# Declaration of variables
CXX_FLAGS = -g -w -std=c++11 -fopenmp -O3
CXX = mpiCC

# File names
EXEC = solver
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: $(EXEC)
# Main target
$(EXEC): $(OBJECTS)
		$(CXX) -fopenmp -O3 $(OBJECTS) -o $(EXEC)

# To obtain object files
%.o: %.cpp
		$(CXX) -c $(CXX_FLAGS) $< -o $@

# To remove generated files
clean:
		rm -f $(EXEC) $(OBJECTS)
