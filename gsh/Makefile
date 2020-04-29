CC      := gcc
CCFLAGS := 
LDFLAGS :=

TARGETS:= gsh
MAINS  := $(addsuffix .o, $(TARGETS) )
OBJ    := $(MAINS)

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(OBJ)

$(OBJ): %.o : %.c
	$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAINS), $(OBJ)) %.o
	$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)
