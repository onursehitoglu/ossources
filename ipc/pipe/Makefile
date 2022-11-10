CSOURCES = $(wildcard *.c)
CTARGETS = $(CSOURCES:%.c=%)
CPPSOURCES += $(wildcard *.cpp)
CPPTARGETS = $(CPPSOURCES:%.cpp=%)

TARGETS = $(CTARGETS) $(CPPTARGETS)

all: $(TARGETS)

% : %.c
	gcc -o $@ $< 


clean:
	-rm $(TARGETS)
