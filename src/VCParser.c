// Author: Ben Martens (1349551)

#define _GNU_SOURCE
#include "VCParser.h"

static Property* createProperty(Card* card, char* currentLine);

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

        // if (createProperty(newCard, currentLine) == NULL) {
        //     error = INV_PROP;
        //     goto EXIT;
        // }

        // don't check return value for now until I have all of the properties implemented
        createProperty(newCard, currentLine);

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
    free(obj);
}

char* cardToString(const Card* obj) {
    char* cardString = NULL;
    char* fullName = NULL;
    char* optionalProperties = NULL;

    fullName = propertyToString(obj->fn);
    optionalProperties = toString(obj->optionalProperties);

    int length = strlen(fullName) + strlen(optionalProperties) + 1;
    cardString = (char*)malloc(length);
    snprintf(cardString, length, "%s%s", fullName, optionalProperties);

    free(fullName);
    free(optionalProperties); 

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
    size_t length = strlen(property->name) + 1;

    // assume only 1 value for now
    char* propertyString = (char*)malloc(length + 2);
    snprintf(propertyString, length + 1, "%s:", property->name);

    void* element;
    ListIterator iter = createIterator(property->values);
    while ((element = nextElement(&iter))) {
        char* value = (char*)element;
        propertyString = realloc(propertyString, strlen(propertyString) + strlen(value) + 2);
        strcat(propertyString, property->values->printData(value));
        strcat(propertyString, ";");
    }
    propertyString[strlen(propertyString) - 1] = '\n';

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

// ************* Static helper functions ************************************
Property* createProperty(Card* card, char* stringToParse) {
    char* propertyString = NULL;
    char* token = NULL;
    Property* newProperty = NULL;

    newProperty = (Property*)malloc(sizeof(Property));
    newProperty->group = "";
    newProperty->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
    newProperty->values = initializeList(valueToString, deleteValue, compareValues);
    propertyString = (char*)malloc(strlen(stringToParse) + 1);
    snprintf(propertyString, strlen(stringToParse) + 1, "%s", stringToParse);
    token = strtok(propertyString, ";:");

    if (strcasecmp(propertyString, "FN") == 0) {
        newProperty->name = "FN";
        token = strtok(NULL, "");
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        card->fn = newProperty;
    } else if (strcasecmp(propertyString, "N") == 0) {
        newProperty->name = "N";
        token = strtok(NULL, ";");
        while (token != NULL) {
            char* value = (char*)malloc(strlen(token) + 1);
            strcpy(value, token);
            insertBack(newProperty->values, (void*)value);
            token = strtok(NULL, ";");
        }
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyString, "BDAY") == 0) {
        newProperty->name = "BDAY";
        token = strtok(NULL, "");
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyString, "GENDER") == 0) {
        newProperty->name = "GENDER";
        token = strtok(NULL, "");
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyString, "ANNIVERSARY") == 0) {
        newProperty->name = "ANNIVERSARY";
        token = strtok(NULL, "");
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyString, "LANG") == 0) {
        newProperty->name = "LANG";
        token = strtok(NULL, "");
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        insertBack(card->optionalProperties, (void*)newProperty);
    } else {
        freeList(newProperty->parameters);
        freeList(newProperty->values);
        free(newProperty);
        free(propertyString);
        return NULL;
    }

    free(propertyString);
    return newProperty;
}
// **************************************************************************
