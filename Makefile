CFLAGS=-Wall -Wextra -std=gnu11 -pedantic -ggdb
heap: my_malloc2.c
	$(CC) $(CFLAGS) -o heap my_malloc2.c