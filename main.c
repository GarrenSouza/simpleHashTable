#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashTable.h"

#define MAX_NAMES 10000
#define NAME_BUFFER_LENGTH 256

/**
 * Struct to store a list of strings
 * Properties:
 *  dataArray
 *  size
 * */
typedef struct stringList {
    char **names;
    size_t listSize;
} stringList;

/**
 * Reads all the lines from a text file storing each one as an entry in a
 * stringList instance that is returned once the file is over
 *
 * @param fileName The name of the file
 * */
stringList *getFileNames(char *fileName) {
    FILE *file = fopen(fileName, "r");
    printf("# Trying to get something from '%s'\n", fileName);
    if (file != NULL) {
        printf("> Reading from '%s'...\n", fileName);
        char *nameBuffer = (char *)malloc(NAME_BUFFER_LENGTH * sizeof(char)), *name = NULL;
        int nameLength;
        size_t nameCounter = 0;

        stringList *List = (stringList *)malloc(sizeof(stringList));
        List->names = (char **)malloc(MAX_NAMES * sizeof(char *));
        List->listSize = 0;

        while (fgets(nameBuffer, NAME_BUFFER_LENGTH, file) != NULL) {
            nameLength = strlen(nameBuffer) - 1;
            name = (char *)malloc(nameLength * sizeof(char));
            strncpy(name, nameBuffer, nameLength);
            name[nameLength - 1] = '\0';

            *(List->names + nameCounter) = name;
            nameCounter++;
        }
        printf("> Done with reading (got %ld names)\n\n", nameCounter);
        fclose(file);
        List->listSize = nameCounter;
        return List;
    }
    printf("> Error while opening the file!\n");
    return NULL;
}

void printStringList(stringList *List) {
    if (List != NULL) {
        for (size_t i = 0; i < List->listSize; i++) {
            if (List->names[i] != NULL) {
                printf("%s, ", List->names[i]);
            }
        }
        printf("\n");
    }
}

void destroyStringListContent(stringList *List) {
    for (size_t i = 0; i < List->listSize; i++) {
        free(*(List->names + i));
    }
}

void destroyStringList(stringList *List) {
    free(List->names);
    free(List);
}

int main() {
    int h = 1, a = 1;
    printf("|-----------------------------LAB 4-----------------------------|\n");
    printf("Choose the main hashing function (0) Polynomial | (1) Murmur3 : ");
    scanf("%d", &h);
    printf("Choose the addressing mode (0) Open | (1) Closed : ");
    scanf("%d", &a);
    printf("\n");
    a = a & ~1 ? 1 : a;
    h = h & ~1 ? 1 : h;
    stringList *namesToInsert = getFileNames("arquivos-nomes-consultas/nomes_10000.txt");
    stringHashTable *s = createHashTable(HASH_TABLE_SIZE, a, h);
    for (size_t i = 0; i < namesToInsert->listSize; i++) {
        s->add(s, *(namesToInsert->names + i));
    }

    stringList *namesToLookUp = getFileNames("arquivos-nomes-consultas/consultas.txt");
    stringList *namesFound, *namesNotFound;

    namesFound = (stringList *)malloc(sizeof(stringList));
    namesNotFound = (stringList *)malloc(sizeof(stringList));

    namesFound->names = (char **)malloc(50 * sizeof(char *));
    namesFound->listSize = 0;
    namesNotFound->names = (char **)malloc(50 * sizeof(char *));
    namesNotFound->listSize = 0;

    int totalSearchSteps = 0, searchSteps, minSearchSteps = HASH_TABLE_SIZE, maxSearchSteps = 1;
    s->entriesCheckedSoFar = 0;
    for (size_t i = 0; i < namesToLookUp->listSize; i++) {
        searchSteps = s->searchKey(s, *(namesToLookUp->names + i));
        if (searchSteps != -1) {
            (namesFound->listSize)++;
            *(namesFound->names + namesFound->listSize - 1) = *(namesToLookUp->names + i);
            if (searchSteps < minSearchSteps) minSearchSteps = searchSteps;
            if (searchSteps > maxSearchSteps) maxSearchSteps = searchSteps;
        } else {
            (namesNotFound->listSize)++;
            *(namesNotFound->names + namesNotFound->listSize - 1) = *(namesToLookUp->names + i);
        }
    }

    printHashTable(s, 0);
    printf("\n# Search\n");
    printf("> %lu names found:\n", namesFound->listSize);
    printStringList(namesFound);
    printf("> %lu names not found:\n", namesNotFound->listSize);
    printStringList(namesNotFound);
    printf("\n");
    printf("> Strings found with %d steps (minimum)\n", minSearchSteps);
    for (size_t i = 0; i < namesFound->listSize; i++) {
        if (s->searchKey(s, namesToLookUp->names[i]) == minSearchSteps) printf("%s, ", namesToLookUp->names[i]);
    }
    printf("\n");
    printf("> Strings found with %d steps (maximum)\n", maxSearchSteps);
    for (size_t i = 0; i < namesFound->listSize; i++) {
        if (s->searchKey(s, namesToLookUp->names[i]) == maxSearchSteps) printf("%s, ", namesToLookUp->names[i]);
    }
    printf("\n");
    destroyHashTable(s);
    destroyStringListContent(namesToInsert);
    destroyStringList(namesToInsert);
}