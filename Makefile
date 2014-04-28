CC = gcc
CFLAGS = -O2 -Wall

EXECS = myexec ish

ISH_DEP = parser/parse.c parser/print.c

.PHONY : all
all : $(EXECS)

% : %.c
	$(CC) $(CFLAGS) -o $@ $<

ish : ish.c $(ISH_DEP)
	$(CC) $(CFLAGS) -o $@ $< $(ISH_DEP)

.PHONY : clean
clean :
	rm -f *.o a.out $(EXECS)

