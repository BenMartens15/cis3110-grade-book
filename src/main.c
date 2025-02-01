#include <stdio.h>
#include "VCParser.h"

int main() {
    Card* testCard = (Card*)malloc(sizeof(Card));
    testCard->optionalProperties = initializeList(propertyToString, deleteProperty, compareProperties);
    testCard->birthday = NULL;
    testCard->anniversary = NULL;

    VCardErrorCode error;

    error = createCard("../testCard.vcf", &testCard);
    if (error == OK) {
        printf("%s\n", cardToString(testCard));
    } else {
        printf("Error: %d\n", error);
    }
}