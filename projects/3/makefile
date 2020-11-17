SHELL = /bin/sh
OBJDIR := ./ofiles
SRCDIR := ./src
OBJS := $(addprefix $(OBJDIR)/,fryerP3Server.o fryerP3Client.o fryerP3Common.o)
SRCS := $(addprefix $(SRCDIR)/,fryerP3Client.c fryerP3Server.c fryerP3Client.h fryerP3Server.h fryerP3Common.c fryerP3Common.h)
BINS := server client
CC := gcc

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all clean clean-all

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(SRCDIR)/%.h
	$(CC) -o $@ -c $<

all : $(BINS)

server : $(OBJDIR)/fryerP3Common.o $(OBJDIR)/fryerP3Server.o
	$(CC) -pthread -o $@ $^

client : $(OBJDIR)/fryerP3Common.o $(OBJDIR)/fryerP3Client.o
	$(CC) -pthread -o $@ $^

$(OBJS) : | $(OBJDIR)

$(OBJDIR) : 
	mkdir $(OBJDIR)

clean :
	rm -v $(BINS)

clean-all : 
	rm -v $(BINS) $(OBJS)
