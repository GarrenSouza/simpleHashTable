
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Headers
#include "hashTable.h"

// External dependencies
#include "dependencies/murmur3.h"

uint32_t murmurHashing(char *string, uint32_t seed) {
    uint32_t hash;
    MurmurHash3_x86_32((const void *)string, strlen(string),
                       MURMUR_SEED_COEFCIENT, &hash);
    return hash;
}

uint32_t polynomialHashing(char *string, uint32_t coeficient) {
    size_t acum = 0;
    for (size_t i = 0; i < strlen(string); i++) {
        acum = coeficient * acum + string[i];
    }
    return acum;
}

// // Closed Addressing (Chaining)

int ClosedAddressingInsert(stringHashTable *hashTable, char *string) {
    if ((hashTable->load) < (hashTable->size) && hashTable->searchKey(hashTable, string) == -1) {
        stringNode *newString = (stringNode *)malloc(sizeof(stringNode));
        uint32_t hash = hashTable->mainHashingFunction(string, hashTable->type & 1 ? MURMUR_SEED_COEFCIENT : POLYNOMIAL_COEFCIENT) % hashTable->size;
        newString->string = string;
        newString->nextString = *(hashTable->dataArray + hash);
        *(hashTable->dataArray + hash) = newString;
        (hashTable->load)++;
        if (newString->nextString != NULL) (hashTable->collisions)++;
        return TRUE;
    }
    return FALSE;
}

int ClosedAddressingDelete(stringHashTable *hashTable, char *string) {
    uint32_t hash = hashTable->mainHashingFunction(string, hashTable->type & 1 ? MURMUR_SEED_COEFCIENT : POLYNOMIAL_COEFCIENT) % hashTable->size;
    stringNode *iterator = *(hashTable->dataArray + hash);
    stringNode *aux = NULL;
    while (iterator != NULL && strcmp(string, iterator->string)) {
        aux = iterator;
        iterator = iterator->nextString;
    }
    if (iterator != NULL) {
        if (aux == NULL)
            *(hashTable->dataArray + hash) = iterator->nextString;
        else
            aux->nextString = iterator->nextString;
        free(iterator);
        return TRUE;
    }
    return FALSE;
}

int ClosedAddressingSearch(stringHashTable *hashTable, char *string) {
    stringNode *iterator = *(hashTable->dataArray + (hashTable->mainHashingFunction(string, hashTable->type & 1 ? MURMUR_SEED_COEFCIENT : POLYNOMIAL_COEFCIENT) % hashTable->size));
    int iterations = 0;
    while (iterator != NULL) {
        iterations++;
        if (strcmp(string, iterator->string) == 0) {
            hashTable->entriesCheckedSoFar += iterations;
            return iterations;
        }
        iterator = iterator->nextString;
    }
    hashTable->entriesCheckedSoFar += iterations;
    return -1;
}

// // Open Addressing

int OpenAddressingInsert(stringHashTable *hashTable, char *string) {
    if ((hashTable->load) < (hashTable->size) && hashTable->searchKey(hashTable, string) == -1) {
        uint32_t hash = hashTable->mainHashingFunction(string, hashTable->type & 1 ? MURMUR_SEED_COEFCIENT : POLYNOMIAL_COEFCIENT);
        uint32_t hashTableSize = hashTable->size;
        stringNode *aux = *(hashTable->dataArray + hash % hashTableSize);
        if (aux != NULL) {
            (hashTable->collisions)++;
            if (aux->isActive) {
                uint32_t hash_b = hashTable->scndHashingFunction(string, hashTable->type & 1 ? POLYNOMIAL_COEFCIENT : MURMUR_SEED_COEFCIENT);
                for (int i = 1; aux != NULL && aux->isActive; i++) {
                    hash += hash_b;
                    aux = *(hashTable->dataArray + hash % hashTableSize);
                }
            }
        }
        if (aux != NULL) {
            aux->string = string;
            aux->isActive = TRUE;
        } else {
            stringNode *newString = (stringNode *)malloc(sizeof(stringNode));
            newString->string = string;
            newString->isActive = TRUE;
            *(hashTable->dataArray + hash % hashTable->size) = newString;
        }
        (hashTable->load)++;
        return TRUE;
    }
    return FALSE;
}

int OpenAddressingSearch(stringHashTable *hashTable, char *string) {
    uint32_t hash = hashTable->mainHashingFunction(string, hashTable->type & 1 ? MURMUR_SEED_COEFCIENT : POLYNOMIAL_COEFCIENT);
    uint32_t hashTableSize = hashTable->size;
    stringNode *aux = *(hashTable->dataArray + hash % hashTableSize);
    int i = 0;
    if (aux != NULL) {
        i++;
        if (aux->isActive && strcmp(aux->string, string) == 0) {
            (hashTable->entriesCheckedSoFar) += i;
            return i;
        }
        uint32_t hash_b = hashTable->scndHashingFunction(string, hashTable->type & 1 ? POLYNOMIAL_COEFCIENT : MURMUR_SEED_COEFCIENT);

        for (; aux != NULL && i < hashTable->size; i++) {
            if (strcmp(aux->string, string) == 0) {
                (hashTable->entriesCheckedSoFar) += i + 1;
                return i + 1;
            }
            hash += hash_b;
            aux = *(hashTable->dataArray + hash % hashTableSize);
        }
    }
    (hashTable->entriesCheckedSoFar) += i + 1;
    return -1;
}

int OpenAddressingDelete(stringHashTable *hashTable, char *string) {
    uint32_t hash = hashTable->mainHashingFunction(string, hashTable->type & 1 ? MURMUR_SEED_COEFCIENT : POLYNOMIAL_COEFCIENT);
    uint32_t hashTableSize = hashTable->size;
    stringNode *aux = *(hashTable->dataArray + hash % hashTableSize);
    if (aux != NULL) {
        if (aux->isActive && strcmp(aux->string, string) == 0) {
            aux->isActive = FALSE;
            return TRUE
        }
        uint32_t hash_b = hashTable->scndHashingFunction(string, hashTable->type & 1 ? POLYNOMIAL_COEFCIENT : MURMUR_SEED_COEFCIENT);
        for (int i = 1; aux != NULL && i < hashTable->size; i++) {
            if (strcmp(aux->string, string) == 0) {
                if (aux->isActive) {
                    aux->isActive = FALSE;
                    return TRUE;
                }
                return FALSE;
            }
            hash += hash_b;
            aux = *(hashTable->dataArray + hash % hashTableSize);
        }
    }
    return FALSE;
}

stringHashTable *createHashTable(size_t size, uint8_t addressingMode,
                                 uint8_t hashingFunction) {
    if (size) {
        stringHashTable *newStringHashtable =
            (stringHashTable *)malloc(sizeof(stringHashTable));
        newStringHashtable->dataArray =
            (stringNode **)malloc(size * sizeof(stringNode *));
        newStringHashtable->load = 0;
        newStringHashtable->collisions = 0;
        newStringHashtable->size = size;
        newStringHashtable->type = addressingMode;
        newStringHashtable->type <<= 1;
        newStringHashtable->type |= hashingFunction;
        newStringHashtable->entriesCheckedSoFar = 0;
        for (size_t i = 0; i < size; i++)
            *(newStringHashtable->dataArray + i) = NULL;

        switch (hashingFunction) {
            case POLYNOMIAL_HASHING_FUNCTION:
                newStringHashtable->mainHashingFunction = polynomialHashing;
                newStringHashtable->scndHashingFunction = murmurHashing;
                break;
            case MURMUR_HASHING_FUNCTION:
                newStringHashtable->mainHashingFunction = murmurHashing;
                newStringHashtable->scndHashingFunction =
                    polynomialHashing;
                break;
            default:
                return NULL;
        }
        switch (addressingMode) {
            case OPEN_ADDRESS_CR:
                newStringHashtable->add = OpenAddressingInsert;
                newStringHashtable->delete = OpenAddressingDelete;
                newStringHashtable->searchKey = OpenAddressingSearch;
                break;
            case CLOSED_ADDRESS_CR:
                newStringHashtable->add = ClosedAddressingInsert;
                newStringHashtable->delete = ClosedAddressingDelete;
                newStringHashtable->searchKey = ClosedAddressingSearch;
                break;
            default:
                return NULL;
        }
        return newStringHashtable;
    }
    return NULL;
}

void destroyHashTable(stringHashTable *hashTable) {
    stringNode *aux, *next;
    // chaining case
    if (hashTable->type & 2) {
        for (size_t i = 0; i < hashTable->size; i++) {
            aux = *(hashTable->dataArray + i);
            while (aux != NULL) {
                next = aux->nextString;
                free(aux);
                aux = next;
            }
        }
    } else {
        for (size_t i = 0; i < hashTable->size; i++) {
            aux = *(hashTable->dataArray + i);
            if (aux != NULL && aux->isActive) {
                free(aux);
            }
        }
    }
    free(hashTable->dataArray);
    free(hashTable);
}

void printHashTable(stringHashTable *hashTable, int printEntries) {
    stringNode *aux;
    // chaining case
    if (hashTable->type & 2) {
        if (printEntries) {
            printf("-----|Keys|-----\n");
            for (size_t i = 0; i < hashTable->size; i++) {
                aux = *(hashTable->dataArray + i);
                while (aux != NULL) {
                    printf("> %s\n", aux->string);
                    aux = aux->nextString;
                }
            }
            printf("----------------\n");
        }
        printf("Addressing: Closed (Chaining)\n");
    } else {
        if (printEntries) {
            printf("-----|Keys|-----\n");
            for (size_t i = 0; i < hashTable->size; i++) {
                aux = *(hashTable->dataArray + i);
                if (aux != NULL && aux->isActive) printf("> %s\n", aux->string);
            }
            printf("----------------\n");
        }
        printf("Addressing: Open (Double Hashing)\n");
    }
    printf("Size: %u\n", hashTable->size);
    printf("Load: %u\n", hashTable->load);
    printf("Collisions: %lu\n", hashTable->collisions);
    printf("Entries checked so far: %lu\n", hashTable->entriesCheckedSoFar);
    printf("Primary Hashing algorithm: %s\n", hashTable->type & 1 ? "Murmur3" : "Polynomial");
    printf("Secondary Hashing algorithm: %s\n", hashTable->type & 1 ? "Polynomial" : "Murmur3");
}