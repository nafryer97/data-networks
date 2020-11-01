SHELL = /bin/sh
OBJDIR := ./ofiles
OBJS := $(addprefix $(OBJDIR)/,fryerP2Server.o fryerP2Sender.o fryerP2Receiver.o project-common.o)
SRCDIR := ./src
SRCS := $(addprefix $(SRCDIR)/,fryerP2Receiver.c fryerP2Sender.c fryerP2Server.c fryerP2Receiver.h fryerP2Sender.h fryerP2Server.h project-common.h project-common.c)
BINS := server sender receiver
CC := gcc

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all clean clean-all

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(SRCDIR)/%.h
	$(CC) -o $@ -c $<

all : $(BINS)

server : $(OBJDIR)/project-common.o $(OBJDIR)/fryerP2Server.o
	$(CC) -o $@ $^

sender : $(OBJDIR)/project-common.o $(OBJDIR)/fryerP2Sender.o
	$(CC) -o $@ $^

receiver : $(OBJDIR)/project-common.o $(OBJDIR)/fryerP2Receiver.o
	$(CC) -o $@ $^

$(OBJS) : | $(OBJDIR)

$(OBJDIR) : 
	mkdir $(OBJDIR)

clean :
	rm -v $(BINS)

clean-all : 
	rm -v $(BINS) $(OBJS)
