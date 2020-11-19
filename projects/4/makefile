SHELL = /bin/sh
OBJDIR := ./ofiles
SRCDIR := ./src
OBJS := $(addprefix $(OBJDIR)/,fryerSelectiveRepeatSender.o fryerSelectiveRepeatReceiver.o fryerSelectiveRepeatCommon.o)
SRCS := $(addprefix $(SRCDIR)/,fryerSelectiveRepeatReceiver.c fryerSelectiveRepeatSender.c fryerSelectiveRepeatReceiver.h fryerSelectiveRepeatSender.h fryerSelectRepeatCommon.c fryerSelectiveRepeatCommon.h)
BINS := sender receiver
CC := gcc

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all clean clean-all

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(SRCDIR)/%.h
	$(CC) -pthread -o $@ -c $<

all : $(BINS)

sender : $(OBJDIR)/fryerSelectiveRepeatCommon.o $(OBJDIR)/fryerSelectiveRepeatSender.o
	$(CC) -pthread -o $@ $^

receiver : $(OBJDIR)/fryerSelectiveRepeatCommon.o $(OBJDIR)/fryerSelectiveRepeatReceiver.o
	$(CC) -pthread -o $@ $^

$(OBJS) : | $(OBJDIR)

$(OBJDIR) : 
	mkdir $(OBJDIR)

clean :
	rm -v $(BINS)

clean-all : 
	rm -v $(BINS) $(OBJS)
