
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Headers
#include "hash_table.h"

// External dependencies
#include "dependencies/murmur3.h"

/** WORKING
 * String hashing method that applies the murmur3 algorithm to the given string
 * @param string The string over which the hash will be calculated
 * @param seed //TODO
 * */
uint32_t murmurHashing(char *string, uint32_t seed) {
    uint32_t hash;
    MurmurHash3_x86_32((const void *)string, strlen(string),
                       MURMUR_SEED_COEFCIENT, &hash);
    return hash;
}

/** WORKING
 * String hashing method that aplies a polynomial series to the string by
 * associating the first character to the coefficient of highest order.
 *  @param string The string over which the hash will be calculated
 *  @param coeficient The coeficient used to compute the series (i.e.: p such as in zp^0+yp^1+xp^2+...)
 */
uint32_t polynomialHashing(char *string, uint32_t coeficient) {
    size_t acum = 0;
    for (size_t i = 0; i < strlen(string); i++) {
        acum = coeficient * acum + string[i];
    }
    return acum;
}

// // Closed Addressing (Chaining)

int ClosedAddressingInsert(stringHashTable *hashTable, char *string) {
    if ((hashTable->load) < (hashTable->size) &&
        !(hashTable->searchKey(hashTable, string))) {
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
    stringNode *iterator =
        *(hashTable->dataArray +
          (hashTable->mainHashingFunction(string, hashTable->type & 1 ? MURMUR_SEED_COEFCIENT : POLYNOMIAL_COEFCIENT) % hashTable->size));
    while (iterator != NULL) {
        if (strcmp(string, iterator->string) == 0) return TRUE;
        iterator = iterator->nextString;
    }
    return FALSE;
}

// // Open Addressing

int OpenAddressingInsert(stringHashTable *hashTable, char *string) {
    if ((hashTable->load) < (hashTable->size) && !hashTable->searchKey(hashTable, string)) {
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
    if (aux != NULL) {
        if (aux->isActive && strcmp(aux->string, string) == 0) return TRUE;
        uint32_t hash_b = hashTable->scndHashingFunction(string, hashTable->type & 1 ? POLYNOMIAL_COEFCIENT : MURMUR_SEED_COEFCIENT);

        for (int i = 1; aux != NULL && i < hashTable->size; i++) {
            if (strcmp(aux->string, string) == 0) return aux->isActive;
            hash += hash_b;
            aux = *(hashTable->dataArray + hash % hashTableSize);
        }
    }
    return FALSE;
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