#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 200
#define MAX_RECORDS 3000

typedef struct {
    char doctorType[MAX_LEN];
    char doctorName[MAX_LEN];
    char area[MAX_LEN];
    char hospital[MAX_LEN];
    char workingDays[MAX_LEN];
    char time[MAX_LEN];
} Doctor;

// ---------- Function Prototypes ----------
int loadDoctors(Doctor doctors[]);
void bookAppointment(const char *username);

// ----------------------------------------
// Load all doctor data from database.txt
// ----------------------------------------
int loadDoctors(Doctor doctors[]) {
    FILE *fp = fopen("database.txt", "r");
    if (!fp) {
        printf("❌ Error: database.txt not found!\n");
        return 0;
    }

    char line[MAX_LEN * 3];
    int count = 0;

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (sscanf(line, "%199[^,],%199[^,],%199[^,],%199[^,],%199[^,],%199[^\n]",
                   doctors[count].doctorType,
                   doctors[count].doctorName,
                   doctors[count].area,
                   doctors[count].hospital,
                   doctors[count].workingDays,
                   doctors[count].time) == 6) {
            count++;
            if (count >= MAX_RECORDS) break;
        }
    }
    fclose(fp);
    return count;
}

// ----------------------------------------
// Utility: check if string already exists in list
// ----------------------------------------
int existsInList(char list[][MAX_LEN], int size, const char *str) {
    for (int i = 0; i < size; i++) {
        if (strcmp(list[i], str) == 0)
            return 1;
    }
    return 0;
}

// ----------------------------------------
// Book appointment interactively
// ----------------------------------------
void bookAppointment(const char *username) {
    Doctor doctors[MAX_RECORDS];
    int total = loadDoctors(doctors);
    if (total == 0) {
        printf("⚠️ No doctor data loaded.\n");
        return;
    }

    // === Step 1: Doctor Type ===
    printf("\n=== Step 1: Select Doctor Type ===\n");

    char types[100][MAX_LEN];
    int typeCount = 0;
    for (int i = 0; i < total; i++) {
        if (!existsInList(types, typeCount, doctors[i].doctorType)) {
            strcpy(types[typeCount++], doctors[i].doctorType);
        }
    }

    for (int i = 0; i < typeCount; i++)
        printf("%d. %s\n", i + 1, types[i]);

    int typeChoice;
    printf("Enter choice: ");
    if (scanf("%d", &typeChoice) != 1 || typeChoice < 1 || typeChoice > typeCount) {
        printf("❌ Invalid choice.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    char chosenType[MAX_LEN];
    strcpy(chosenType, types[typeChoice - 1]);

    // === Step 2: Area ===
    printf("\n=== Step 2: Select Area ===\n");

    char areas[100][MAX_LEN];
    int areaCount = 0;
    for (int i = 0; i < total; i++) {
        if (strcmp(doctors[i].doctorType, chosenType) == 0 &&
            !existsInList(areas, areaCount, doctors[i].area)) {
            strcpy(areas[areaCount++], doctors[i].area);
        }
    }

    for (int i = 0; i < areaCount; i++)
        printf("%d. %s\n", i + 1, areas[i]);

    int areaChoice;
    printf("Enter choice: ");
    if (scanf("%d", &areaChoice) != 1 || areaChoice < 1 || areaChoice > areaCount) {
        printf("❌ Invalid choice.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    char chosenArea[MAX_LEN];
    strcpy(chosenArea, areas[areaChoice - 1]);

    // === Step 3: Hospital ===
    printf("\n=== Step 3: Select Hospital ===\n");

    char hospitals[100][MAX_LEN];
    int hospCount = 0;
    for (int i = 0; i < total; i++) {
        if (strcmp(doctors[i].doctorType, chosenType) == 0 &&
            strcmp(doctors[i].area, chosenArea) == 0 &&
            !existsInList(hospitals, hospCount, doctors[i].hospital)) {
            strcpy(hospitals[hospCount++], doctors[i].hospital);
        }
    }

    for (int i = 0; i < hospCount; i++)
        printf("%d. %s\n", i + 1, hospitals[i]);

    int hospChoice;
    printf("Enter choice: ");
    if (scanf("%d", &hospChoice) != 1 || hospChoice < 1 || hospChoice > hospCount) {
        printf("❌ Invalid choice.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    char chosenHospital[MAX_LEN];
    strcpy(chosenHospital, hospitals[hospChoice - 1]);

    // === Step 4: Doctor ===
    printf("\n=== Step 4: Select Doctor ===\n");

    Doctor available[100];
    int availCount = 0;
    for (int i = 0; i < total; i++) {
        if (strcmp(doctors[i].doctorType, chosenType) == 0 &&
            strcmp(doctors[i].area, chosenArea) == 0 &&
            strcmp(doctors[i].hospital, chosenHospital) == 0) {
            available[availCount++] = doctors[i];
        }
    }

    for (int i = 0; i < availCount; i++) {
        printf("%d. %s — %s — %s\n",
               i + 1, available[i].doctorName,
               available[i].workingDays, available[i].time);
    }

    int docChoice;
    printf("Enter choice: ");
    if (scanf("%d", &docChoice) != 1 || docChoice < 1 || docChoice > availCount) {
        printf("❌ Invalid choice.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    Doctor chosen = available[docChoice - 1];

    // === Step 5: Choose Day ===
    printf("\n=== Step 5: Select Day ===\n");
    char daysCopy[MAX_LEN];
    strcpy(daysCopy, chosen.workingDays);

    char *token = strtok(daysCopy, " ");
    char days[10][MAX_LEN];
    int dayCount = 0;
    while (token) {
        strcpy(days[dayCount++], token);
        token = strtok(NULL, " ");
    }

    for (int i = 0; i < dayCount; i++)
        printf("%d. %s\n", i + 1, days[i]);

    int dayChoice;
    printf("Enter choice: ");
    if (scanf("%d", &dayChoice) != 1 || dayChoice < 1 || dayChoice > dayCount) {
        printf("❌ Invalid choice.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    char chosenDay[MAX_LEN];
    strcpy(chosenDay, days[dayChoice - 1]);

    // === Step 6: Choose 30-min Time Slot ===
    printf("\n=== Step 6: Select Time Slot ===\n");

    int startHour, startMin, endHour, endMin;
    char ampm1[3], ampm2[3];
    sscanf(chosen.time, "%d:%d%2s-%d:%d%2s", &startHour, &startMin, ampm1, &endHour, &endMin, ampm2);

    // Convert to 24-hour format
    if (strcmp(ampm1, "PM") == 0 && startHour != 12) startHour += 12;
    if (strcmp(ampm2, "PM") == 0 && endHour != 12) endHour += 12;

    int slots[48][2]; // [startHour*60 + startMin, endHour*60 + endMin]
    int slotCount = 0;
    int start = startHour * 60 + startMin;
    int end = endHour * 60 + endMin;

    for (int t = start; t + 30 <= end; t += 30)
        slots[slotCount++][0] = t;

    // Read booked slots from log file
    FILE *log = fopen("appointments_log.txt", "r");
    char booked[200][MAX_LEN];
    int bookedCount = 0;
    if (log) {
        while (fgets(booked[bookedCount], sizeof(booked[0]), log)) {
            booked[bookedCount][strcspn(booked[bookedCount], "\n")] = '\0';
            bookedCount++;
        }
        fclose(log);
    }

    // Show only available slots
    int availableSlots[50];
    int availableCount = 0;
    for (int i = 0; i < slotCount; i++) {
        int slotStart = slots[i][0];
        int slotEnd = slotStart + 30;

        int hour1 = slotStart / 60;
        int min1 = slotStart % 60;
        int hour2 = slotEnd / 60;
        int min2 = slotEnd % 60;

        char slotStr[256];
        snprintf(slotStr, sizeof(slotStr),
         "%s,%s,%s,%s,%02d:%02d-%02d:%02d",
         chosen.doctorType, chosen.doctorName, chosenDay, chosen.hospital,
         hour1, min1, hour2, min2);

        int taken = 0;
        for (int j = 0; j < bookedCount; j++) {
            if (strcmp(booked[j], slotStr) == 0) {
                taken = 1;
                break;
            }
        }

        if (!taken) {
            printf("%d. %02d:%02d to %02d:%02d\n", availableCount + 1, hour1, min1, hour2, min2);
            availableSlots[availableCount++] = slotStart;
        }
    }

    if (availableCount == 0) {
        printf("⚠️ No slots available for this doctor on %s.\n", chosenDay);
        return;
    }

    int slotChoice;
    printf("Enter choice: ");
    if (scanf("%d", &slotChoice) != 1 || slotChoice < 1 || slotChoice > availableCount) {
        printf("❌ Invalid choice.\n");
        while (getchar() != '\n');
        return;
    }

    int chosenStart = availableSlots[slotChoice - 1];
    int chosenEnd = chosenStart + 30;

    int hour1 = chosenStart / 60, min1 = chosenStart % 60;
    int hour2 = chosenEnd / 60, min2 = chosenEnd % 60;

    // === Step 7: Confirm and Save ===
    printf("\n✅ Appointment booked successfully!\n");
    printf("-------------------------------------\n");
    printf("Doctor Type : %s\n", chosen.doctorType);
    printf("Doctor Name : %s\n", chosen.doctorName);
    printf("Area         : %s\n", chosen.area);
    printf("Hospital     : %s\n", chosen.hospital);
    printf("Day          : %s\n", chosenDay);
    printf("Time Slot    : %02d:%02d to %02d:%02d\n", hour1, min1, hour2, min2);

    // Save to user file
    char filename[MAX_LEN];
    sprintf(filename, "%s_appointments.txt", username);
    FILE *fa = fopen(filename, "a");
    if (fa) {
        fprintf(fa, "%s,%s,%s,%s,%s,%02d:%02d-%02d:%02d\n",
                chosen.doctorType, chosen.doctorName, chosen.area,
                chosen.hospital, chosenDay, hour1, min1, hour2, min2);
        fclose(fa);
    }

    // Save to global log
    FILE *logfile = fopen("appointments_log.txt", "a");
    if (logfile) {
        fprintf(logfile, "%s,%s,%s,%s,%02d:%02d-%02d:%02d\n",
                chosen.doctorType, chosen.doctorName, chosenDay,
                chosen.hospital, hour1, min1, hour2, min2);
        fclose(logfile);
    }
}


// ----------------------------------------
// Example Main Menu
// ----------------------------------------
// int main() {
//     int choice;
//     char username[50];
//     printf("Enter your username: ");
//     scanf("%49s", username);

//     while (1) {
//         printf("\n=== MAIN MENU ===\n");
//         printf("1. Book Appointment\n");
//         printf("2. Exit\n");
//         printf("Enter choice: ");
//         if (scanf("%d", &choice) != 1) {
//             printf("❌ Invalid input.\n");
//             while (getchar() != '\n');
//             continue;
//         }
//         while (getchar() != '\n');

//         if (choice == 1) {
//             bookAppointment(username);
//         } else if (choice == 2) {
//             printf("👋 Goodbye!\n");
//             break;
//         } else {
//             printf("❌ Invalid choice.\n");
//         }
//     }

//     return 0;
// }
