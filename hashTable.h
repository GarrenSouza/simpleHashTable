#include <stdint.h>
#include <stdio.h>

#ifndef _HASH_CONFIG
#define OPEN_ADDRESS_CR 0
#define CLOSED_ADDRESS_CR 1

#define POLYNOMIAL_HASHING_FUNCTION 0
#define MURMUR_HASHING_FUNCTION 1

#define HASH_TABLE_SIZE 20011

#define POLYNOMIAL_COEFCIENT 7
#define MURMUR_SEED_COEFCIENT 0x0855e30d //random number
#endif

#ifndef _BOOLEAN
#define TRUE 1;
#define FALSE 0;
#endif

typedef struct stringNode {
    char *string;
    struct stringNode *nextString;
    uint8_t isActive;
} stringNode;

typedef struct stringHashTable {
    uint32_t load, size;
    size_t collisions, entriesCheckedSoFar;
    uint32_t (*mainHashingFunction)(char *string, uint32_t coeficient),
        (*scndHashingFunction)(char *string, uint32_t coeficient);
    stringNode **dataArray;
    uint8_t type;
    int (*delete)(struct stringHashTable *, char *string),
        (*searchKey)(struct stringHashTable *, char *string),
        (*add)(struct stringHashTable *, char *string);
} stringHashTable;

// HashTable Methods
stringHashTable *createHashTable(size_t size, uint8_t addressingMode, uint8_t hashingFunction);
void destroyHashTable(stringHashTable *hashTable);

// // Addressing Methods

// Closed Addressing Methods
int ClosedAddressingInsert(stringHashTable *hashTable, char *string);
int ClosedAddressingDelete(stringHashTable *hashTable, char *string);
int ClosedAddressingSearch(stringHashTable *hashTable, char *string);

// Open Address Methods
int OpenAddressingInsert(stringHashTable *hashTable, char *string);
int OpenAddressingDelete(stringHashTable *hashTable, char *string);
int OpenAddressingSearch(stringHashTable *hashTable, char *string);

// Available Hashing Functions
uint32_t polynomialHashing(char *string, uint32_t coeficient);
uint32_t murmurHashing(char *string, uint32_t seed);

void printHashTable(stringHashTable *hashTable, int printEntries);