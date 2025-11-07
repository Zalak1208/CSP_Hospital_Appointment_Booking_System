#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "authentication.h"   // include your header

// Function prototypes
void signup(char *username);
int login(char *username);
void createUserAppointmentFile(const char *username);

// ---------- AUTHENTICATION FUNCTION ----------
int authenticate(char *username) {
    int choice;

    while (1) {
        printf("\n=== Hospital System Authentication ===\n");
        printf("1. Signup\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            signup(username);
            return 1;  // success
        } 
        else if (choice == 2) {
            if (login(username)) {
                printf("✅ Login successful! Welcome, %s!\n", username);
                return 1;  // success
            } else {
                printf("❌ Invalid username or password. Try again.\n");
            }
        } 
        else if (choice == 3) {
            printf("Exiting...\n");
            exit(0);
        } 
        else {
            printf("Invalid choice! Try again.\n");
        }
    }
}

// ---------- SIGNUP ----------
void signup(char *username) {
    char password[MAX_LEN];
    FILE *fp = fopen("users.txt", "a+");
    if (!fp) {
        printf("Error opening user file.\n");
        return;
    }

    printf("Enter a new username: ");
    scanf("%s", username);
    printf("Enter a new password: ");
    scanf("%s", password);

    // Check if user already exists
    char u[MAX_LEN], p[MAX_LEN];
    rewind(fp);
    while (fscanf(fp, "%s %s", u, p) != EOF) {
        if (strcmp(u, username) == 0) {
            printf("❌ Username already exists. Try another one.\n");
            fclose(fp);
            return;
        }
    }

    // Write new user
    fprintf(fp, "%s %s\n", username, password);
    fclose(fp);

    printf("✅ Account created successfully for %s!\n", username);
    createUserAppointmentFile(username);
}

// ---------- LOGIN ----------
int login(char *username) {
    char password[MAX_LEN];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    FILE *fp = fopen("users.txt", "r");
    if (!fp) {
        printf("No users registered yet.\n");
        return 0;
    }

    char u[MAX_LEN], p[MAX_LEN];
    while (fscanf(fp, "%s %s", u, p) != EOF) {
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            fclose(fp);
            createUserAppointmentFile(username);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

// ---------- CREATE USER APPOINTMENT FILE ----------
void createUserAppointmentFile(const char *username) {
    char filename[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);

    FILE *fp = fopen(filename, "a");  // creates if missing
    if (fp) {
        fclose(fp);
        printf("📁 Appointment file ready: %s\n", filename);
    } else {
        printf("⚠️ Error creating appointment file for %s.\n", username);
    }
}
