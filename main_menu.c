// main_menu.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_menu.h"
#include "appointment.h"

// ---------- MAIN MENU ----------

// When user selects "Book Appointment" option
void mainMenu(char *username) {
    int choice;
    do {
        printf("\n=== Main Menu ===\n");
        printf("1. Book Appointment\n");
        printf("2. View Appointments\n");
        printf("3. Edit Appointment\n");
        printf("4. Delete Appointment\n");
        printf("5. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                bookAppointment(username);  // <-- function is CALLED, not defined
                break;
            case 2:
                viewAppointments(username);
                break;
            case 3:
                editAppointment(username);
                break;
            case 4:
                deleteAppointment(username);
                break;
            case 5:
                printf("Logging out...\n");
                break;
            default:
                printf("Invalid choice! Try again.\n");
        }
    } while (choice != 5);
}




// ---------- VIEW APPOINTMENTS ----------
void viewAppointments(const char *username) {
    char filename[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);

    printf("🧭 Current Working Directory:\n");
    system("pwd");  // shows exactly where program is looking

    printf("🔍 Looking for: %s\n", filename);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("❌ File not found in current directory.\n");
        system("ls -l");  // list files in that directory
        return;
    }

    printf("✅ File opened successfully!\n\n");

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        printf("LINE %d: %s", ++count, line);
    }

    if (count == 0)
        printf("⚠️ File is empty.\n");
    fclose(fp);
}




// ---------- DELETE APPOINTMENT ----------
void deleteAppointment(const char *username) {
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
void editAppointment(const char *username) {
    char filename[100];
    sprintf(filename, "%s_appointments.txt", username);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("⚠️ No appointments found for this user.\n");
        return;
    }

    char appointments[200][256];
    int count = 0;

    while (fgets(appointments[count], sizeof(appointments[count]), fp)) {
        appointments[count][strcspn(appointments[count], "\n")] = '\0';
        count++;
    }
    fclose(fp);

    if (count == 0) {
        printf("⚠️ You have no appointments to edit.\n");
        return;
    }

    printf("\n=== Your Appointments ===\n");
    for (int i = 0; i < count; i++) {
        printf("%d. %s\n", i + 1, appointments[i]);
    }

    int choice;
    printf("Enter the number of the appointment you want to edit: ");
    scanf("%d", &choice);
    while (getchar() != '\n');

    if (choice < 1 || choice > count) {
        printf("❌ Invalid choice.\n");
        return;
    }

    // Remove from user file
    FILE *fp2 = fopen(filename, "w");
    for (int i = 0; i < count; i++) {
        if (i != choice - 1)
            fprintf(fp2, "%s\n", appointments[i]);
    }
    fclose(fp2);

    // Remove from global log
    FILE *log = fopen("appointments_log.txt", "r");
    FILE *temp = fopen("temp_log.txt", "w");
    char line[256];
    while (fgets(line, sizeof(line), log)) {
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, appointments[choice - 1]) != 0) {
            fprintf(temp, "%s\n", line);
        }
    }
    fclose(log);
    fclose(temp);
    remove("appointments_log.txt");
    rename("temp_log.txt", "appointments_log.txt");

    printf("\n🌀 Now please book a new appointment.\n");
    bookAppointment(username);
}

    // struct Appointment appt;
    // char filename[MAX_LEN + 20], tempFile[MAX_LEN + 20];
    // sprintf(filename, "%s_appointments.txt", username);
    // sprintf(tempFile, "%s_temp.txt", username);

    // FILE *fp = fopen(filename, "r");
    // FILE *temp = fopen(tempFile, "w");
    // if (!fp || !temp) {
    //     printf("Error opening file.\n");
    //     return;
    // }

    // char doctor[MAX_LEN], date[20];
    // printf("Enter doctor type to edit: ");
    // scanf("%s", doctor);
    // printf("Enter appointment date to edit: ");
    // scanf("%s", date);

    // int edited = 0;
    // while (fscanf(fp, "%s %s %s %s", appt.username, appt.doctorType, appt.date, appt.time) != EOF) {
    //     if (strcmp(appt.username, username) == 0 &&
    //         strcmp(appt.doctorType, doctor) == 0 &&
    //         strcmp(appt.date, date) == 0) {

    //         printf("Enter new date (YYYY-MM-DD): ");
    //         scanf("%s", appt.date);
    //         printf("Enter new time (HH:MM): ");
    //         scanf("%s", appt.time);
    //         edited = 1;
    //     }
    //     fprintf(temp, "%s %s %s %s\n", appt.username, appt.doctorType, appt.date, appt.time);
    // }

    // fclose(fp);
    // fclose(temp);
    // remove(filename);
    // rename(tempFile, filename);

    // if (edited)
    //     printf("✅ Appointment updated successfully.\n");
    // else
    //     printf("❌ Appointment not found.\n");
// }
