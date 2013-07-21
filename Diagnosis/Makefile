
TARGET=Diagnosis
CXX=clang
CFLAGS=-std=c++11 -O2 -Wall -Wextra
FLAGS=`llvm-config --cxxflags --ldflags` -lclang $(CFLAGS)

SAMPLE_DIR=samples
SAMPLES:=$(wildcard $(SAMPLE_DIR)/*.cc)
SAMPLES_OUT:=$(subst .cc,_out.txt,$(SAMPLES))

all: $(TARGET)

$(TARGET): $(TARGET).cc
	$(CXX) $(TARGET).cc -o $(TARGET) $(FLAGS)

run: $(SAMPLES_OUT)

$(SAMPLE_DIR)/%_out.txt: $(TARGET) $(SAMPLE_DIR)/%.cc
	./$(TARGET) $(SAMPLE_DIR)/$*.cc `llvm-config --cxxflags` > $@ 2>&1

clean: 
	@find . -name '*~' | xargs rm -f
	@rm -f $(TARGET) $(SAMPLES_OUT)

.PHONY: check-syntax
check-syntax:
	@$(CXX) -c $(CHK_SOURCES) -fsyntax-only $(FLAGS)
