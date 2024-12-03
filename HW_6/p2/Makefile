test_spinlock: test_spinlock.o spinlock.o tas64.o
	gcc -o test_spinlock.out test_spinlock.o spinlock.o tas64.o

test_spinlock.o: test_spinlock.c
	gcc -c test_spinlock.c
	
spinlock.o: spinlock.c spinlock.h
	gcc -c spinlock.c

tas64.o: tas64.S tas.h
	gcc -c tas64.S -o tas64.o

test_semaphores: test_semaphores.o semaphores.o tas64.o
	gcc -o test_semaphores.out test_semaphores.o semaphores.o tas64.o

test_semaphores.o: test_semaphores.c
	gcc -c test_semaphores.c

semaphores.o: semaphores.c semaphores.h
	gcc -c semaphores.c

shell_game: shell_game.o semaphores.o tas64.o
	gcc -o test_semaphores.out shell_game.o semaphores.o tas64.o

shell_game.o: shell_game.o
	gcc -c shell_game.c

clean:
	rm *.out *.o