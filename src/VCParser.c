// Author: Ben Martens (1349551)

#define _GNU_SOURCE
#include "VCParser.h"

// ************* Card parser ***********************************************
VCardErrorCode createCard(char* fileName, Card** obj) {
    VCardErrorCode error = OK;
    FILE* fp;
    char* currentLine = NULL;
    char* nextLine = NULL;
    size_t len = 0;

    Card* newCard = *obj;

    fp = fopen(fileName, "r");
    if (fp == NULL) {
        error = INV_FILE;
        goto EXIT;
    }

    // get the first line to make sure it contains the BEGIN:VCARD property
    getline(&currentLine, &len, fp);
    currentLine[strlen(currentLine) - 2] = '\0'; // remove the \r\n from the string
    getline(&nextLine, &len, fp);
    nextLine[strlen(nextLine) - 2] = '\0'; // remove the \r\n from the string
    
    while (nextLine[0] == ' ') {
        currentLine = realloc(currentLine, strlen(currentLine) + strlen(nextLine));
        if (currentLine) {
            strcat(currentLine, nextLine + 1);
        } else {
            error = OTHER_ERROR;
            goto EXIT;
        }
        getline(&nextLine, &len, fp);
        nextLine[strlen(nextLine) - 2] = '\0';
    }

    if (strncasecmp(currentLine, "BEGIN:VCARD", strlen(currentLine)) != 0) {
        error = INV_CARD;
        goto EXIT;
    }

    // copy nextLine to currentLine
    currentLine = realloc(currentLine, strlen(nextLine) + 1);
    if (currentLine) {
        snprintf(currentLine, strlen(nextLine) + 1, "%s", nextLine);
    } else {
        error = OTHER_ERROR;
        goto EXIT;
    }

    // read the file line-by-line (unfolding any folded lines)
    while (getline(&nextLine, &len, fp) != -1) {
        nextLine[strlen(nextLine) - 2] = '\0'; // remove the \r\n from the string
        while (nextLine[0] == ' ') {
            currentLine = realloc(currentLine, strlen(currentLine) + strlen(nextLine));
            if (currentLine) {
                strcat(currentLine, nextLine + 1);
            } else {
                error = OTHER_ERROR;
                goto EXIT;
            }
            getline(&nextLine, &len, fp);
            nextLine[strlen(nextLine) - 2] = '\0';
        }

        char* property = (char*)malloc(strlen(currentLine) + 1);
        snprintf(property, strlen(currentLine) + 1, "%s", currentLine);
        char* token = strtok(property, ";:");
        if (strcasecmp(property, "FN") == 0) {
            Property* newProperty = (Property*)malloc(sizeof(Property));
            newProperty->name = "FN";
            newProperty->group = "";
            newProperty->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
            newProperty->values = initializeList(valueToString, deleteValue, compareValues);
            token = strtok(NULL, "");
            char* value = (char*)malloc(strlen(token) + 1);
            strcpy(value, token);
            insertBack(newProperty->values, (void*)value);

            newCard->fn = newProperty;
        }
        free(property);

        // copy nextLine to currentLine
        currentLine = realloc(currentLine, strlen(nextLine) + 1);
        if (currentLine) {
            snprintf(currentLine, strlen(nextLine) + 1, "%s", nextLine);
        } else {
            error = OTHER_ERROR;
            goto EXIT;
        }
    }

    if (strncasecmp(currentLine, "END:VCARD", strlen(currentLine)) != 0) {
        error = INV_CARD;
        goto EXIT;
    }

EXIT:
    if (fp) {
        fclose(fp);
    }
    free(currentLine);
    free(nextLine);
    return error;
}

void deleteCard(Card* obj) {
    deleteProperty(obj->fn);
    freeList(obj->optionalProperties);
}

char* cardToString(const Card* obj) {
    char* cardString;

    cardString = propertyToString(obj->fn);

    return cardString;
}

char* errorToString(VCardErrorCode err);
// *************************************************************************

// ************* List helper functions ************************************* 
void deleteProperty(void* toBeDeleted) {
    Property* property = (Property*)toBeDeleted;
    freeList(property->parameters);
    freeList(property->values);
    free(property);
}

int compareProperties(const void* first,const void* second) {
    return 0;
}

char* propertyToString(void* prop) {
    Property* property = (Property*)prop;
    size_t length = strlen(property->name) + 1 + strlen(property->values->printData(getFromFront(property->values)));

    // assume only 1 value for now
    char* propertyString = (char*)malloc(length + 1);
    snprintf(propertyString, length + 1, "%s:%s", property->name, property->values->printData(getFromFront(property->values)));

    return propertyString;
}

void deleteParameter(void* toBeDeleted) {

}

int compareParameters(const void* first,const void* second) {
    return 0;
}

char* parameterToString(void* param) {
    return NULL;
}

void deleteValue(void* toBeDeleted) {
    char* value = (char*)toBeDeleted;
    free(value);
}

int compareValues(const void* first,const void* second) {
    return 0;
}

char* valueToString(void* val) {
    char* outString = (char*)val;

    return outString;
}

void deleteDate(void* toBeDeleted);
int compareDates(const void* first,const void* second);
char* dateToString(void* date);
// **************************************************************************