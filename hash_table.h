#include <stdio.h>
#include <stdint.h>

#ifndef HASH_CONFIG

#define OPEN_ADDRESS_CR 0
#define CLOSED_ADDRESS_CR 1

#define FST_HASH_FUNCTION 0
#define SND_HASH_FUNCTION 1

#endif

typedef struct string
{
    uint32_t hashValue;
    char *string;
    uint8_t flags;
} string;

typedef struct stringNode
{
    uint32_t hashValue;
    char *string;
    uint8_t flags;
    stringNode *nextString;
} stringNode;

typedef struct addressingMode
{
    uint8_t type;
    void (*deletionRoutine)(stringHashTable *, uint32_t hashValue),
        (*insertionRoutine)(stringHashTable *, char *string),
        (*searchRoutine)(stringHashTable *, char *string);
} addressingMode;

typedef struct stringHashTable
{
    size_t size, load;
    const uint32_t (*hashingRoutine)(char *);
    const stringNode *data_array;
    const addressingMode addressMode;
} stringHashTable;

// HashTable Methods
stringHashTable createHashTable(size_t size, uint32_t (*hashingRoutine)(char *), addressingMode *addressMode);
addressingMode createAddressingMode(uint8_t addressingModeCode);
void destroyHashTable(stringHashTable* hashTable);

// // Addressing Methods

// Closed Addressing Methods
void ClosedAddressingInsert(stringHashTable *hashTable, char *string);
void ClosedAddressingDelete(stringHashTable *hashTable, char *string);
void ClosedAddressingSearch(stringHashTable *hashTable, char *string);

// Open Address Methods
void OpenAddressingInsert(stringHashTable *hashTable, char *string);
void OpenAddressingDelete(stringHashTable *hashTable, char *string);
void OpenAddressingSearch(stringHashTable *hashTable, char *string);

// Available Hashing Functions
uint32_t polynomialHashing(char *string, uint8_t coeficient, size_t tableSize);
uint32_t otherHashing(char *string);