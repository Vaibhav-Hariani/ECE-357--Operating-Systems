test_spinlock: test_spinlock.o spinlock.o
	g++ -o test_spinlock.out test_spinlock.o spinlock.o

test_spinlock.o: test_spinlock.c
	g++ -c test_spinlock.c
	
spinlock.o: spinlock.c spinlock.h
	g++ -c spinlock.c

test_semaphores: test_semaphores.o semaphores.o
	g++ -o test_semaphores.out test_semaphores.o semaphores.o

test_semaphores.o: test_semaphores.c
	g++ -c test_semaphores.c

semaphores.o: semaphores.c semaphores.h
	g++ -c semaphores.c

shell_game: shell_game.o semaphores.o
	g++ -o test_semaphores.out shell_game.o semaphores.o

shell_game.o: shell_game.o
	g++ -c shell_game.c

clean:
	rm *.out *.o