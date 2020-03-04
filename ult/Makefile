CSOURCES = $(wildcard *.c)
CTARGETS = $(CSOURCES:%.c=%)
CPPSOURCES += $(wildcard *.cpp)
CPPTARGETS = $(CPPSOURCES:%.cpp=%)
CFLAGS = -g

TARGETS = $(CTARGETS) $(CPPTARGETS)

all: $(TARGETS)

% : %.c
	gcc -o $@ $(CFLAGS) $< 


clean:
	-rm $(TARGETS)
