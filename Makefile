CC = gcc
CFLAGS = -O2 -Wall

EXECS = ish

ISH_DEP =ish.c parser/parse.c parser/print.c
ISH_OBJS=$(ISH_DEP:.c=.o)

.PHONY : all
all : $(EXECS)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ish : $(ISH_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ISH_OBJS)

.PHONY : clean
clean :
	rm -f $(ISH_OBJS) $(EXECS)
