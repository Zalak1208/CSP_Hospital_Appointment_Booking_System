// main_menu.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_menu.h"

// ---------- MAIN MENU ----------
void mainMenu(char *username) {
    int choice;

    while (1) {
        printf("\n=== Main Menu for %s ===\n", username);
        printf("1. Book Appointment\n");
        printf("2. View Appointments\n");
        printf("3. Delete Appointment\n");
        printf("4. Edit Appointment\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: 
                bookAppointment(username);
                break;
            case 2: 
                viewAppointments(username);
                break;
            case 3: 
                deleteAppointment(username);
                break;
            case 4: 
                editAppointment(username);
                break;
            case 5:
                printf("Exiting Main Menu. Goodbye!\n");
                return;
            default:
                printf("Invalid choice! Try again.\n");
        }
    }
}

// ---------- BOOK APPOINTMENT (Placeholder) ----------
void bookAppointment(char *username) {
    printf("\n🚧 Booking feature coming soon for user '%s'...\n", username);
}

// ---------- VIEW APPOINTMENTS ----------
void viewAppointments(char *username) {
    struct Appointment appt;
    char filename[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("No appointments found yet.\n");
        return;
    }

    int found = 0;
    printf("\nYour Appointments:\n");
    while (fscanf(fp, "%s %s %s %s", appt.username, appt.doctorType, appt.date, appt.time) != EOF) {
        if (strcmp(appt.username, username) == 0) {
            found = 1;
            printf("👩‍⚕️ Doctor: %s | 📅 Date: %s | 🕒 Time: %s\n",
                   appt.doctorType, appt.date, appt.time);
        }
    }

    if (!found)
        printf("No appointments yet!\n");

    fclose(fp);
}

// ---------- DELETE APPOINTMENT ----------
void deleteAppointment(char *username) {
    struct Appointment appt;
    char filename[MAX_LEN + 20], tempFile[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);
    sprintf(tempFile, "%s_temp.txt", username);

    FILE *fp = fopen(filename, "r");
    FILE *temp = fopen(tempFile, "w");
    if (!fp || !temp) {
        printf("Error opening file.\n");
        return;
    }

    char doctor[MAX_LEN], date[20];
    printf("Enter doctor type to delete (e.g., Cardiologist): ");
    scanf("%s", doctor);
    printf("Enter appointment date to delete (YYYY-MM-DD): ");
    scanf("%s", date);

    int deleted = 0;
    while (fscanf(fp, "%s %s %s %s", appt.username, appt.doctorType, appt.date, appt.time) != EOF) {
        if (strcmp(appt.username, username) == 0 &&
            strcmp(appt.doctorType, doctor) == 0 &&
            strcmp(appt.date, date) == 0) {
            deleted = 1;
            continue;
        }
        fprintf(temp, "%s %s %s %s\n", appt.username, appt.doctorType, appt.date, appt.time);
    }

    fclose(fp);
    fclose(temp);
    remove(filename);
    rename(tempFile, filename);

    if (deleted)
        printf("✅ Appointment deleted successfully.\n");
    else
        printf("❌ No matching appointment found.\n");
}

// ---------- EDIT APPOINTMENT ----------
void editAppointment(char *username) {
    struct Appointment appt;
    char filename[MAX_LEN + 20], tempFile[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);
    sprintf(tempFile, "%s_temp.txt", username);

    FILE *fp = fopen(filename, "r");
    FILE *temp = fopen(tempFile, "w");
    if (!fp || !temp) {
        printf("Error opening file.\n");
        return;
    }

    char doctor[MAX_LEN], date[20];
    printf("Enter doctor type to edit: ");
    scanf("%s", doctor);
    printf("Enter appointment date to edit: ");
    scanf("%s", date);

    int edited = 0;
    while (fscanf(fp, "%s %s %s %s", appt.username, appt.doctorType, appt.date, appt.time) != EOF) {
        if (strcmp(appt.username, username) == 0 &&
            strcmp(appt.doctorType, doctor) == 0 &&
            strcmp(appt.date, date) == 0) {

            printf("Enter new date (YYYY-MM-DD): ");
            scanf("%s", appt.date);
            printf("Enter new time (HH:MM): ");
            scanf("%s", appt.time);
            edited = 1;
        }
        fprintf(temp, "%s %s %s %s\n", appt.username, appt.doctorType, appt.date, appt.time);
    }

    fclose(fp);
    fclose(temp);
    remove(filename);
    rename(tempFile, filename);

    if (edited)
        printf("✅ Appointment updated successfully.\n");
    else
        printf("❌ Appointment not found.\n");
}
