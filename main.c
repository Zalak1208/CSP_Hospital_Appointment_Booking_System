// main.c
#include <stdio.h>
#include "authentication.h"
#include "main_menu.h"

int main() {
    char username[MAX_LEN];

    printf("=== Welcome to Hospital Management System ===\n");

    // Run authentication first
    if (authenticate(username)) {
        // After successful login/signup, open main menu
        mainMenu(username);
    }

    printf("\nThank you for using the system!\n");
    return 0;
}

