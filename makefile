CC=gcc

program: dependencies/murmur3.c hashTable.c main.c
	$(CC) -o lab_4 dependencies/murmur3.c hashTable.c main.c