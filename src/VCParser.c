// Author: Ben Martens (1349551)

#define _GNU_SOURCE
#include "VCParser.h"

static Property* createProperty(Card* card, const char* currentLine);
static void parsePropertyValues(List* valueList, const char* name, char* valueString);

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
    free(property->name);
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
    
    if (strcasecmp(propertyName, "FN") == 0) {
        char* token = strtok(valueString, ";"); // get the first value
        newProperty->name = (char*)malloc(strlen(propertyName) + 1);
        strcpy(newProperty->name, propertyName);
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(newProperty->values, (void*)value);
        card->fn = newProperty;
    } else if (strcasecmp(propertyName, "SOURCE") == 0 ||
            strcasecmp(propertyName, "KIND") == 0 ||
            strcasecmp(propertyName, "XML") == 0 ||
            strcasecmp(propertyName, "N") == 0 ||
            strcasecmp(propertyName, "NICKNAME") == 0 ||
            strcasecmp(propertyName, "PHOTO") == 0 ||
            strcasecmp(propertyName, "GENDER") == 0 ||
            strcasecmp(propertyName, "ADR") == 0 ||
            strcasecmp(propertyName, "TEL") == 0 ||
            strcasecmp(propertyName, "EMAIL") == 0 ||
            strcasecmp(propertyName, "IIMP") == 0 ||
            strcasecmp(propertyName, "LANG") == 0 ||
            strcasecmp(propertyName, "TZ") == 0 ||
            strcasecmp(propertyName, "GEO") == 0 ||
            strcasecmp(propertyName, "TITLE") == 0 ||
            strcasecmp(propertyName, "ROLE") == 0 ||
            strcasecmp(propertyName, "LOGO") == 0 ||
            strcasecmp(propertyName, "ORG") == 0 ||
            strcasecmp(propertyName, "MEMBER") == 0 ||
            strcasecmp(propertyName, "RELATED") == 0 ||
            strcasecmp(propertyName, "CATEGORIES") == 0 ||
            strcasecmp(propertyName, "NOTE") == 0 ||
            strcasecmp(propertyName, "PRODID") == 0 ||
            strcasecmp(propertyName, "REV") == 0 ||
            strcasecmp(propertyName, "SOUND") == 0 ||
            strcasecmp(propertyName, "UID") == 0 ||
            strcasecmp(propertyName, "CLIENTPIDMAP") == 0 ||
            strcasecmp(propertyName, "URL") == 0 ||
            strcasecmp(propertyName, "VERSION") == 0 ||
            strcasecmp(propertyName, "KEY") == 0 ||
            strcasecmp(propertyName, "FBURL") == 0 ||
            strcasecmp(propertyName, "CALADRURI") == 0 ||
            strcasecmp(propertyName, "CALURI") == 0) {
        newProperty->name = (char*)malloc(strlen(propertyName) + 1);
        strcpy(newProperty->name, propertyName);
        parsePropertyValues(newProperty->values, propertyName, valueString);
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

void parsePropertyValues(List* valueList, const char* name, char* valueString) {
    char* token = strtok(valueString, ";"); // get the first value

    while (token != NULL) {
        char* value = (char*)malloc(strlen(token) + 1);
        strcpy(value, token);
        insertBack(valueList, (void*)value);
        token = strtok(NULL, ";");
    }
}
// **************************************************************************
