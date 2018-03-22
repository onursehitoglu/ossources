CSOURCES = $(wildcard *.c)
CTARGETS = $(CSOURCES:%.c=%)
CPPSOURCES += $(wildcard *.cpp)
CPPTARGETS = $(CPPSOURCES:%.cpp=%)

TARGETS = $(CTARGETS) $(CPPTARGETS)

all: $(TARGETS)

sharedmmapedit: sharedmmapedit.c
	gcc -o sharedmmapedit sharedmmapedit.c -lncurses

% : %.c
	gcc -o $@ $< 


clean:
	-rm $(TARGETS)
