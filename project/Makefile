# Declaration of variables
CXX_FLAGS = -g -w -std=c++11 -fopenmp -O3
CXX_FLAGS_SEQ = -g -w -std=c++11 -O3
CXX = mpiCC

# File names
EXEC = solver
EXEC_SEQ = solver-seq
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS_SEQ = $(SOURCES:.cpp=.oseq)

all: $(EXEC) $(EXEC_SEQ)
# Main target
$(EXEC): $(OBJECTS)
		$(CXX) -fopenmp -O3 $(OBJECTS) -o $(EXEC)

$(EXEC_SEQ): $(OBJECTS_SEQ)
		$(CXX) -O3 $(OBJECTS_SEQ) -o $(EXEC_SEQ)

# To obtain object files
%.o: %.cpp
		$(CXX) -c $(CXX_FLAGS) $< -o $@

%.oseq: %.cpp
		$(CXX) -c $(CXX_FLAGS_SEQ) $< -o $@

# To remove generated files
clean:
		rm -f $(EXEC) $(OBJECTS)
		rm -f $(EXEC_SEQ) $(OBJECTS_SEQ)
