
PS = c
CC = gcc
CFLAGS = -g -Wall -O0 -DTRACE
DEST := nk-stack
LIBS :=
INCLUDES :=  
SUB_DIR := 
DEST_DIR := 
INSTALL := /usr/local/bin

RM := rm -f
CFLAGS  += -MMD -MF $(patsubst ./%, %, $(patsubst %.o, %.d, $(dir $@).$(notdir $@))) $(addprefix -I, $(INCLUDES))
SRCS := $(wildcard *.$(PS) $(addsuffix /*.$(PS), $(SUB_DIR)))
OBJS := $(patsubst %.$(PS), %.o, $(SRCS))
DEPS := $(patsubst %.$(PS), %.d, $(foreach n,$(SRCS),$(patsubst ./%, %, $(dir $n).$(notdir $n))))
MISS := $(filter-out $(wildcard DEPS), $(DEPS))

all: $(DEST)

clean :
	@$(RM) $(OBJS) 
	@$(RM) $(DEPS) 
	@$(RM) $(DEST)

install:
	@if [ ! -d $(DEST_DIR)$(INSTALL) ]; then mkdir -p $(DEST_DIR)$(INSTALL); fi
	cp -f $(DEST) $(DEST_DIR)$(INSTALL)
ifneq ($(MISS),)
$(MISS):
	@$(RM) $(patsubst %.d, %.o, $(dir $@)$(patsubst .%,%, $(notdir $@)))
endif

-include $(DEPS)

$(DEST): $(OBJS)
	$(CC) -o $(DEST) $(OBJS) $(addprefix -l,$(LIBS))

