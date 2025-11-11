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
    char filename[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);

    FILE *fp1 = fopen(filename, "rw");
    if (!fp1) {
        printf("❌ File not found in current directory.\n");
        return;
    }

    FILE *fp2 = fopen("appointments_log.txt", "rw");
    if (!fp2) {
        printf("❌ Could not open log file.\n");
        fclose(fp1);
        return;
    }

    char line[512], tempLine[512];
    int i = 1;
    while(fgets(line, sizeof(line), fp1)) {
        printf("%d. %s", i, line);
        i++;
    }

    printf("\nEnter the line number of the appointment to delete: ");
    int lineToDelete;
    scanf("%d", &lineToDelete);
    rewind(fp1);

    FILE *temp1 = fopen("temp1.txt", "w");
    if (!temp1) {
        printf("❌ Could not open temporary file.\n");
        fclose(fp1);
        fclose(fp2);
        return;
    }
    int currentLine = 1;
    while (fgets(line, sizeof(line), fp1)) {
        if (currentLine != lineToDelete) {
            fputs(line, temp1);
        }
        else {
            strcpy(tempLine, line);
        }
        currentLine++;
    }

    FILE* temp2 = fopen("temp2.txt", "w");
    if (!temp2) {
        printf("❌ Could not open second temporary file.\n");
        fclose(fp1);
        fclose(fp2);
        fclose(temp1);
        return;
    }

    while (fgets(line, sizeof(line), fp2)) {
        if(strcmp(line, tempLine) != 0) {
            fputs(line, temp2);
        }
    }
    
    fclose(temp2);
    fclose(fp1);
    fclose(temp1);
    fclose(fp2);
    remove(filename);
    remove("appointments_log.txt");
    rename("temp2.txt", "appointments_log.txt");
    rename("temp1.txt", filename);
    printf("✅ Appointment deleted successfully.\n");
}

// ---------- EDIT APPOINTMENT ----------
void editAppointment(const char *username) {
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
