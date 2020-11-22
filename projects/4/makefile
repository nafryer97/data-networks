SHELL = /bin/sh
OBJDIR := ./ofiles
SRCDIR := ./src
OBJS := $(addprefix $(OBJDIR)/,fryerSelectiveRepeatSender.o fryerSelectiveRepeatReceiver.o fryerSelectiveRepeatCommon.o)
SRCS := $(addprefix $(SRCDIR)/,fryerSelectiveRepeatReceiver.c fryerSelectiveRepeatSender.c fryerSelectiveRepeatReceiver.h fryerSelectiveRepeatSender.h fryerSelectRepeatCommon.c fryerSelectiveRepeatCommon.h)
BINS := sender receiver
OPTIMIZED_BINS := sender_opt receiver_opt
CC := gcc

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all clean clean-all

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(SRCDIR)/%.h
	$(CC) -g -Wall -pthread -o $@ -c $< -lpthread

all : $(BINS)

optimize : $(OPTIMIZED_BINS)

sender : $(OBJDIR)/fryerSelectiveRepeatCommon.o $(OBJDIR)/fryerSelectiveRepeatSender.o
	$(CC) -g -Wall -pthread -o $@ $^ -lpthread

receiver : $(OBJDIR)/fryerSelectiveRepeatCommon.o $(OBJDIR)/fryerSelectiveRepeatReceiver.o
	$(CC) -g -Wall -pthread -o $@ $^ -lpthread

sender_opt : src/fryerSelectiveRepeatSender.c src/fryerSelectiveRepeatCommon.c
	$(CC) -g -O2 -Wall -pthread -o $@ $^ -lpthread

receiver_opt : src/fryerSelectiveRepeatReceiver.c src/fryerSelectiveRepeatCommon.c
	$(CC) -g -O2 -Wall -pthread -o $@ $^ -lpthread

$(OBJS) : | $(OBJDIR)

$(OBJDIR) : 
	mkdir $(OBJDIR)

clean :
	rm -v $(BINS) $(OPTIMIZED_BINS)

clean-all : 
	rm -v $(BINS) $(OPTIMIZED_BINS) $(OBJS)
