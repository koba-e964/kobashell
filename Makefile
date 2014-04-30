CC = gcc
CFLAGS = -O2 -Wall

EXEC = ish

ISH_SRC =ish.c parser/parse.c parser/print.c
ISH_OBJS=$(ISH_SRC:.c=.o)

.PHONY : all run clean
all : $(EXEC)

run : $(EXEC)
	./$(EXEC)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXEC) : $(ISH_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ISH_OBJS)

clean :
	rm -f $(ISH_OBJS) $(EXEC)
