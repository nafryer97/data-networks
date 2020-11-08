SHELL = /bin/sh
OBJDIR := ./ofiles
OBJS := $(addprefix $(OBJDIR)/,fryerP2Server.o fryerP2Sender.o fryerP2Receiver.o fryerP2Common.o)
SRCDIR := ./src
SRCS := $(addprefix $(SRCDIR)/,fryerP2Receiver.c fryerP2Sender.c fryerP2Server.c fryerP2Receiver.h fryerP2Sender.h fryerP2Server.h fryerP2Common.h fryerP2Common.c)
BINS := server sender receiver
CC := gcc

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all clean clean-all

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(SRCDIR)/%.h
	$(CC) -o $@ -c $<

all : $(BINS)

server : $(OBJDIR)/fryerP2Common.o $(OBJDIR)/fryerP2Server.o
	$(CC) -o $@ $^

sender : $(OBJDIR)/fryerP2Common.o $(OBJDIR)/fryerP2Sender.o
	$(CC) -o $@ $^

receiver : $(OBJDIR)/fryerP2Common.o $(OBJDIR)/fryerP2Receiver.o
	$(CC) -o $@ $^

$(OBJS) : | $(OBJDIR)

$(OBJDIR) : 
	mkdir $(OBJDIR)

clean :
	rm -v $(BINS)

clean-all : 
	rm -v $(BINS) $(OBJS)
