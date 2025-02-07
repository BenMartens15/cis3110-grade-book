// Author: Ben Martens (1349551)

#define _GNU_SOURCE
#include "VCParser.h"

static Property* createProperty(Card* card, const char* currentLine);

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
    Property* property = NULL;

    if (toBeDeleted == NULL) {
        return;
    }
    
    property = (Property*)toBeDeleted;
    freeList(property->parameters);
    freeList(property->values);
    free(property);
}

int compareProperties(const void* first,const void* second) {
    return 0;
}

char* propertyToString(void* prop) {
    Property* property = NULL;
    char* propertyString = NULL;

    if (prop == NULL) {
        return NULL;
    }

    property = (Property*)prop;
    size_t length = strlen(property->name) + 1;

    propertyString = (char*)malloc(length + 1);
    snprintf(propertyString, length + 1, "%s", property->name);

    void* paramElem;
    ListIterator paramIter = createIterator(property->parameters);
    while ((paramElem = nextElement(&paramIter)) != NULL) {
        Parameter* param = (Parameter*)paramElem;
        size_t length = 1 + strlen(param->name) + 1 + strlen(param->value) + 1;
        propertyString = realloc(propertyString, strlen(propertyString) + length);
        strcat(propertyString, ";");
        strcat(propertyString, param->name);
        strcat(propertyString, "=");
        strcat(propertyString, param->value);
    }
    propertyString = realloc(propertyString, strlen(propertyString) + 2);
    strcat(propertyString, ":");

    void* valueElem;
    ListIterator valueIter = createIterator(property->values);
    while ((valueElem = nextElement(&valueIter)) != NULL) {
        char* value = (char*)valueElem;
        propertyString = realloc(propertyString, strlen(propertyString) + strlen(value) + 2);
        strcat(propertyString, property->values->printData(value));
        strcat(propertyString, ";");
    }
    propertyString[strlen(propertyString) - 1] = '\n';

    return propertyString;
}

void deleteParameter(void* toBeDeleted) {
    Parameter* param = NULL;
    
    if (toBeDeleted == NULL) {
        return;
    }

    param = (Parameter*)toBeDeleted;
    free(param->name);
    free(param->value);
    free(param);
}

int compareParameters(const void* first,const void* second) {
    return 0;
}

char* parameterToString(void* param) {
    Parameter* parameter = NULL;
    char* parameterString = NULL;

    if (param == NULL) {
        return NULL;
    }

    parameter = (Parameter*)param;
    size_t length = strlen(parameter->name) + 1 + strlen(parameter->value) + 1;
    parameterString = (char*)malloc(length);
    snprintf(parameterString, length, "%s=%s", parameter->name, parameter->value);

    return parameterString;
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
Property* createProperty(Card* card, const char* stringToParse) {
    char* propertyName = NULL;
    char* paramString = NULL;
    char* valueString = NULL;
    char* propertyString = NULL; // just a temporary string so that the original one isn't modified by strtok
    Property* newProperty = NULL;

    newProperty = (Property*)malloc(sizeof(Property));
    newProperty->group = "";
    newProperty->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
    newProperty->values = initializeList(valueToString, deleteValue, compareValues);
    propertyString = (char*)malloc(strlen(stringToParse) + 1);
    snprintf(propertyString, strlen(stringToParse) + 1, "%s", stringToParse);
    
    valueString = strpbrk(propertyString, ":") + 1; // set valueString to everything after colon
    paramString = strtok(propertyString, ":"); // set paramString to everything before colon
    propertyName = strtok(paramString, ";:");

    char* paramToken = strtok(NULL, ";");
    while (paramToken) {
        Parameter* newParam = (Parameter*)malloc(sizeof(Parameter));
        int paramNameLen = strcspn(paramToken, "=");
        newParam->name = (char*)malloc(paramNameLen + 1);
        newParam->value = (char*)malloc(strlen(paramToken) - paramNameLen);
        strncpy(newParam->name, paramToken, paramNameLen + 1);
        newParam->name[paramNameLen] = '\0';
        strncpy(newParam->value, paramToken + paramNameLen + 1, strlen(paramToken) - paramNameLen);
        paramToken = strtok(NULL, ";");
        insertBack(newProperty->parameters, newParam);
    }
    
    char* token = strtok(valueString, ";"); // get the first value
    if (strcasecmp(propertyName, "FN") == 0) {
        newProperty->name = "FN";
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        card->fn = newProperty;
    } else if (strcasecmp(propertyName, "N") == 0) {
        newProperty->name = "N";
        while (token != NULL) {
            char* value = (char*)malloc(strlen(token) + 1);
            strcpy(value, token);
            insertBack(newProperty->values, (void*)value);
            token = strtok(NULL, ";");
        }
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyName, "BDAY") == 0) {
        newProperty->name = "BDAY";
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyName, "GENDER") == 0) {
        newProperty->name = "GENDER";
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyName, "ANNIVERSARY") == 0) {
        newProperty->name = "ANNIVERSARY";
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyName, "LANG") == 0) {
        newProperty->name = "LANG";
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        insertBack(card->optionalProperties, (void*)newProperty);
    } else if (strcasecmp(propertyName, "TEL") == 0) {
        newProperty->name = "TEL";
        while (token != NULL) {
            char* value = (char*)malloc(strlen(token) + 1);
            strcpy(value, token);
            insertBack(newProperty->values, (void*)value);
            token = strtok(NULL, ";");
        }
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
