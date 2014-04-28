CC = gcc
CFLAGS = -O2 -Wall

EXECS = ish

ISH_DEP = ish.c parser/parse.c parser/print.c


.PHONY : all
all : $(EXECS)

%.o : %.c
	$(CC) $(CFLAGS) -o $@ $<

ish : $(ISH_DEP)
	$(CC) $(CFLAGS) -o $@ $(ISH_DEP)

.PHONY : clean
clean :
	rm -f *.o a.out $(EXECS)

