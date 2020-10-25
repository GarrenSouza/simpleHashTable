#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "hash_table.h"

uint32_t polynomialHashing(char *string, uint8_t coeficient, size_t tableSize)
{
    size_t acum = 0;
    for (size_t i; i < strlen(string); i++)
    {
        acum = coeficient * acum + string[i];
    }
    return acum % tableSize;
}

uint32_t otherHashing(char *string)
{

}

stringHashTable createHashTable(size_t size, uint32_t (*hashingRoutine)(char *), addressingMode *addressMode)
{
}

addressingMode createAddressingMode(uint8_t addressingModeCode)
{
}

