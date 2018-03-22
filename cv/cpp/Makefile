CSOURCES = $(wildcard *.c)
CTARGETS = $(CSOURCES:%.c=%)
CPPSOURCES += $(wildcard *.cpp)
CPPTARGETS = $(CPPSOURCES:%.cpp=%)

TARGETS = $(CTARGETS) $(CPPTARGETS)

all: $(TARGETS)

% : %.cpp monitor.h
	g++ -g -o $@ $< -lpthread 

% : %.c
	gcc -g -o $@ $< -lpthread 

clean:
	-rm $(TARGETS)
