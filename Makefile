CC = gcc
CFLAGS = -O2 -Wall -DUSE_READLINE=0

EXEC = bin/ish

ISH_SRC =getline.c int_list.c ish.c job.c path.c signal.c parser/parse.c
ISH_OBJS=$(ISH_SRC:.c=.o)

.PHONY : all run clean cmake cmake-run
all : $(EXEC)

run : $(EXEC)
	./$(EXEC)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXEC) : $(ISH_OBJS)
	if [ ! -d bin ] ; then mkdir bin ; fi
	$(CC) $(CFLAGS) -o $@ $(ISH_OBJS)

clean :
	rm -f $(ISH_OBJS) $(EXEC)

# This target uses cmake.
cmake : 
	cd build; cmake ..; make

cmake-run : cmake
	./$(EXEC)

