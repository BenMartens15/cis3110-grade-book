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

        char* property = (char *)malloc(strlen(currentLine) + 1);
        snprintf(property, strlen(currentLine) + 1, "%s", currentLine);
        property = strtok(property, ";:");
        if (strncasecmp(property, "FN", strlen(property)) == 0) {
            Property* newProperty = (Property*)malloc(sizeof(Property));
            newProperty->name = "FN";
            newProperty->group = "";
            newProperty->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
            newProperty->values = initializeList(valueToString, deleteValue, compareValues);
            char* value = strtok(NULL, "");
            insertBack(newProperty->values, value);

            newCard->fn = newProperty;
        }

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

void deleteCard(Card* obj);

char* cardToString(const Card* obj) {
    size_t length = strlen(obj->fn->values->printData(getFromFront(obj->fn->values))) + 1; // + 1 for null terminator
    char* cardString = (char *)malloc(length);

    snprintf(cardString, 3 + length + 1, "FN:%s\n", obj->fn->values->printData(getFromFront(obj->fn->values))); // plus 1 for newline character

    return cardString;
}

char* errorToString(VCardErrorCode err);
// *************************************************************************

// ************* List helper functions ************************************* 
void deleteProperty(void* toBeDeleted) {

}

int compareProperties(const void* first,const void* second) {
    return 0;
}

char* propertyToString(void* prop) {
    return NULL;
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

}

int compareValues(const void* first,const void* second) {
    return 0;
}

char* valueToString(void* val) {
    char* outString = (char*)malloc(strlen((char*)val));

    sprintf(outString, "%s", (char*)val);

    return outString;
}

void deleteDate(void* toBeDeleted);
int compareDates(const void* first,const void* second);
char* dateToString(void* date);
// **************************************************************************