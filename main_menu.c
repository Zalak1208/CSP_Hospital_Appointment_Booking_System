#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appointment.h"
#include "main_menu.h"

// ---------- MAIN MENU ----------
void mainMenu(const char *username) {
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
                bookAppointment(username);
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

void viewAppointments(const char *username) {
    char filename[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("❌ No appointments found for %s.\n", username);
        return;
    }

    printf("\n=== Your Appointments ===\n");

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        char doctorType[100], doctorName[100], area[100], hospital[100], day[50], slot[50];
        // read 6 fields (matches your saved format)
        if (sscanf(line, "%99[^,],%99[^,],%99[^,],%99[^,],%49[^,],%49[^\n]",
                   doctorType, doctorName, area, hospital, day, slot) == 6) {
            printf("\n-------------------------------------\n");
            printf("✅ Appointment #%d\n", ++count);
            printf("Doctor Type : %s\n", doctorType);
            printf("Doctor Name : %s\n", doctorName);
            printf("Area        : %s\n", area);
            printf("Hospital    : %s\n", hospital);
            printf("Day         : %s\n", day);
            printf("Time Slot   : %s\n", slot);
            printf("-------------------------------------\n");
        }
    }

    if (count == 0)
        printf("⚠️ No appointments booked yet.\n");

    fclose(fp);
    printf("\nPress Enter to return to the main menu...");
    while(getchar() != '\n'); // consume leftover newline
     getchar(); // wait for user to press Enter

}
// ---------- DELETE APPOINTMENT ----------
void deleteAppointment(const char *username) {
    Appointment appt;
    char filename[MAX_LEN + 20], tempFile[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);
    sprintf(tempFile, "%s_temp.txt", username);

    FILE *fp = fopen(filename, "r");
    FILE *temp = fopen(tempFile, "w");
    if (!fp || !temp) {
        printf("❌ Error opening file.\n");
        return;
    }

    char doctorName[MAX_LEN];
    printf("Enter doctor name to delete appointment: ");
    scanf(" %[^\n]", doctorName);

    int deleted = 0;
    while (fscanf(fp, "%199[^,],%199[^,],%199[^,],%199[^,],%199[^,],%199[^\n]\n",
                  appt.doctorType, appt.doctorName, appt.area, appt.hospital,
                  appt.workingDays, appt.time) == 6) {

        if (strcmp(appt.doctorName, doctorName) == 0) {
            deleted = 1;
            continue; // skip writing this one
        }
        fprintf(temp, "%s,%s,%s,%s,%s,%s\n",
                appt.doctorType, appt.doctorName, appt.area,
                appt.hospital, appt.workingDays, appt.time);
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
    Appointment appt;
    char filename[MAX_LEN + 20], tempFile[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);
    sprintf(tempFile, "%s_temp.txt", username);

    FILE *fp = fopen(filename, "r");
    FILE *temp = fopen(tempFile, "w");
    if (!fp || !temp) {
        printf("❌ Error opening file.\n");
        return;
    }

    char doctorName[MAX_LEN];
    printf("Enter doctor name to edit appointment: ");
    scanf(" %[^\n]", doctorName);

    int edited = 0;
    while (fscanf(fp, "%199[^,],%199[^,],%199[^,],%199[^,],%199[^,],%199[^\n]\n",
                  appt.doctorType, appt.doctorName, appt.area, appt.hospital,
                  appt.workingDays, appt.time) == 6) {

        if (strcmp(appt.doctorName, doctorName) == 0) {
            printf("Enter new time (e.g. 10:00-10:30): ");
            scanf(" %[^\n]", appt.time);
            edited = 1;
        }
        fprintf(temp, "%s,%s,%s,%s,%s,%s\n",
                appt.doctorType, appt.doctorName, appt.area,
                appt.hospital, appt.workingDays, appt.time);
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
