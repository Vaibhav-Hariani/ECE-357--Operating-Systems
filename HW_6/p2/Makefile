test_spinlock: test_spinlock.o spinlock.o tas64.o
	gcc -o test_spinlock.out test_spinlock.o spinlock.o tas64.o

test_spinlock.o: test_spinlock.c
	gcc -c test_spinlock.c
	
spinlock.o: spinlock.c spinlock.h
	gcc -c spinlock.c

tas64.o: tas64.S tas.h
	gcc -c tas64.S -o tas64.o

semaphores.o: sem.c sem.h
	gcc -c sem.c

shell_game: shell_game.o sem.o spinlock.o tas64.o
	gcc -o shell_game.out shell_game.o sem.o spinlock.o tas64.o

debug_game:
	gcc -g -o debug_game.out shell_game.c sem.c spinlock.c tas64.S

shell_game.o: shell_game.o
	gcc -c shell_game.c

clean:
	rm *.out *.o