#include <stdio.h>
#include "VCParser.h"

int main() {
    Card* testCard;
    VCardErrorCode error;

    error = createCard("../testCard.vcf", &testCard);
    if (error == OK) {
        char* cardString = cardToString(testCard);
        printf("%s\n", cardString);
        free(cardString);
    } else {
        printf("Error: %d\n", error);
    }
    deleteCard(testCard);
}