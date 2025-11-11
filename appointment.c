#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "appointment.h"

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

// --- Function prototypes ---
int loadDoctors(Doctor doctors[]);

// --- Utility: convert "HH:MMAM" to minutes since midnight ---
int parseTime(const char *t) {
    int hour, min;
    char ampm[3];
    sscanf(t, "%d:%d%2s", &hour, &min, ampm);
    for (int i = 0; ampm[i]; i++) ampm[i] = toupper(ampm[i]);
    if (strcmp(ampm, "PM") == 0 && hour != 12) hour += 12;
    if (strcmp(ampm, "AM") == 0 && hour == 12) hour = 0;
    return hour * 60 + min;
}

// --- Utility: format minutes as HH:MM ---
void formatTime(int mins, char *out) {
    int hour = mins / 60;
    int min = mins % 60;
    sprintf(out, "%02d:%02d", hour, min);
}

// --- Utility: check if slot already booked ---
int isSlotBooked(const char *username, const char *doctorName, const char *day, const char *slot) {
    char filename[MAX_LEN];
    sprintf(filename, "%s_appointments.txt", username);
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    char line[MAX_LEN];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, doctorName) && strstr(line, day) && strstr(line, slot)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

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

    fgets(line, sizeof(line), fp); // skip header
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (sscanf(line, "%199[^,],%199[^,],%199[^,],%199[^,],%199[^,],%199[^\n]",
                   doctors[count].doctorType, doctors[count].doctorName,
                   doctors[count].area, doctors[count].hospital,
                   doctors[count].workingDays, doctors[count].time) == 6) {
            count++;
            if (count >= MAX_RECORDS) break;
        }
    }
    fclose(fp);
    return count;
}

// ----------------------------------------
// Book Appointment Function
// ----------------------------------------
void bookAppointment(const char *username) {
    Doctor doctors[MAX_RECORDS];
    int total = loadDoctors(doctors);
    if (total == 0) {
        printf("⚠️ No doctor data loaded.\n");
        return;
    }

    char doctorType[MAX_LEN];

    // --- Step 1: Select Doctor Type ---
    const char *types[] = {
        "General Physician","Pediatrician","Cardiologist","Dermatologist","Neurologist",
        "Orthopedic Surgeon","Gynecologist","Ophthalmologist","Dentist","Psychiatrist"
    };

    int typeChoice;
    while (1) {
        printf("\n=== Step 1: Select Doctor Type ===\n");
        for (int i = 0; i < 10; i++) printf("%d. %s\n", i + 1, types[i]);
        printf("Enter choice: ");
        if (scanf("%d", &typeChoice) == 1 && typeChoice >= 1 && typeChoice <= 10) {
            strcpy(doctorType, types[typeChoice - 1]);
            break;
        }
        printf("❌ Invalid choice. Please try again.\n");
        while(getchar()!='\n');
    }

    // --- Step 2: Select Area ---
    const char *areas[] = {
        "Paldi","Maninagar","Navrangpura","Satellite","SG Highway","Vastrapur","Thaltej","Bodakdev",
        "Naranpura","Ellisbridge","Gota","Isanpur","Vejalpur","Shahibaug","Vastral","Naranpura East",
        "Odhav","Ramnagar","Nehru Park","Gandhinagar Road"
    };

    char area[MAX_LEN];
    int areaChoice;
    while (1) {
        printf("\n=== Step 2: Select Area ===\n");
        for (int i = 0; i < 20; i++) printf("%d. %s\n", i + 1, areas[i]);
        printf("Enter choice: ");
        if (scanf("%d", &areaChoice) == 1 && areaChoice >= 1 && areaChoice <= 20) {
            strcpy(area, areas[areaChoice - 1]);
            break;
        }
        printf("❌ Invalid choice. Please try again.\n");
        while(getchar()!='\n');
    }

    // --- Step 3: Select Hospital ---
    char hospitals[50][MAX_LEN];
    int hospitalCount = 0;
    for (int i = 0; i < total; i++) {
        if (strcmp(doctors[i].doctorType, doctorType) == 0 &&
            strcmp(doctors[i].area, area) == 0) {
            int found = 0;
            for (int j = 0; j < hospitalCount; j++)
                if (strcmp(hospitals[j], doctors[i].hospital) == 0) found = 1;
            if (!found && hospitalCount < 50) strcpy(hospitals[hospitalCount++], doctors[i].hospital);
        }
    }
    if (hospitalCount == 0) {
        printf("⚠️ No hospitals found for %s in %s.\n", doctorType, area);
        return;
    }

    char hospital[MAX_LEN];
    int hospChoice;
    while (1) {
        printf("\n=== Step 3: Select Hospital ===\n");
        for (int i = 0; i < hospitalCount; i++) printf("%d. %s\n", i + 1, hospitals[i]);
        printf("Enter choice: ");
        if (scanf("%d", &hospChoice) == 1 && hospChoice >= 1 && hospChoice <= hospitalCount) {
            strcpy(hospital, hospitals[hospChoice - 1]);
            break;
        }
        printf("❌ Invalid choice. Please try again.\n");
        while(getchar()!='\n');
    }

    // --- Step 4: Select Doctor ---
    Doctor available[100];
    int availCount = 0;
    for (int i = 0; i < total; i++) {
        if (strcmp(doctors[i].doctorType, doctorType) == 0 &&
            strcmp(doctors[i].area, area) == 0 &&
            strcmp(doctors[i].hospital, hospital) == 0) {
            available[availCount++] = doctors[i];
        }
    }
    if (availCount == 0) {
        printf("⚠️ No doctors available in this hospital.\n");
        return;
    }

    Doctor chosenDoctor;
    int docChoice;
    while (1) {
        printf("\n=== Step 4: Select Doctor ===\n");
        for (int i = 0; i < availCount; i++)
            printf("%d. %s — %s — %s\n", i+1, available[i].doctorName,
                   available[i].workingDays, available[i].time);
        printf("Enter choice: ");
        if (scanf("%d", &docChoice) == 1 && docChoice >= 1 && docChoice <= availCount) {
            chosenDoctor = available[docChoice - 1];
            break;
        }
        printf("❌ Invalid choice. Please try again.\n");
        while(getchar()!='\n');
    }

    // --- Step 5: Select Day ---
    char days[10][20];
    char workingDaysCopy[MAX_LEN];
    strcpy(workingDaysCopy, chosenDoctor.workingDays);
    char *token = strtok(workingDaysCopy, " ");
    int dayCount = 0;
    while (token && dayCount < 10) {
        strcpy(days[dayCount++], token);
        token = strtok(NULL, " ");
    }

    char selectedDay[20];
    int dayChoice;
    while (1) {
        printf("\n=== Step 5: Select Day ===\n");
        for (int i = 0; i < dayCount; i++) printf("%d. %s\n", i+1, days[i]);
        printf("Enter day choice: ");
        if (scanf("%d", &dayChoice) == 1 && dayChoice >= 1 && dayChoice <= dayCount) {
            strcpy(selectedDay, days[dayChoice-1]);
            break;
        }
        printf("❌ Invalid choice. Try again.\n");
        while(getchar()!='\n');
    }

    // --- Step 6: Time slot selection ---
    char timeCopy[MAX_LEN];
    strcpy(timeCopy, chosenDoctor.time);
    char *startStr = strtok(timeCopy, "-");
    char *endStr = strtok(NULL, "-");
    if (!startStr || !endStr) {
        printf("⚠️ Invalid doctor time format.\n");
        return;
    }

    int start = parseTime(startStr);
    int end = parseTime(endStr);
    char slots[50][20];
    int slotCount = 0;
    for (int t = start; t + 30 <= end; t += 30) {
        char t1[10], t2[10];
        formatTime(t, t1);
        formatTime(t + 30, t2);
        sprintf(slots[slotCount++], "%s-%s", t1, t2);
    }

    int availableSlotCount = 0;
    int slotIndices[50];
    for (int i = 0; i < slotCount; i++) {
        if (!isSlotBooked(username, chosenDoctor.doctorName, selectedDay, slots[i])) {
            slotIndices[availableSlotCount] = i;
            printf("%d. %s\n", availableSlotCount+1, slots[i]);
            availableSlotCount++;
        }
    }
    if (availableSlotCount == 0) {
        printf("⚠️ No slots available for this doctor on %s.\n", selectedDay);
        return;
    }

    char selectedSlot[20];
    int slotChoice;
    while (1) {
        printf("Enter slot number: ");
        if (scanf("%d", &slotChoice) == 1 && slotChoice >= 1 && slotChoice <= availableSlotCount) {
            strcpy(selectedSlot, slots[slotIndices[slotChoice-1]]);
            break;
        }
        printf("❌ Invalid choice. Try again.\n");
        while(getchar()!='\n');
    }

    // --- Step 7: Save Appointment ---
    printf("\n✅ Appointment booked successfully!\n");
    printf("-------------------------------------\n");
    printf("Doctor : %s (%s)\n", chosenDoctor.doctorName, chosenDoctor.doctorType);
    printf("Hospital: %s\n", chosenDoctor.hospital);
    printf("Area    : %s\n", chosenDoctor.area);
    printf("Day     : %s\n", selectedDay);
    printf("Slot    : %s\n", selectedSlot);

    char filename[MAX_LEN];
    sprintf(filename, "%s_appointments.txt", username);
    FILE *fa = fopen(filename, "a");
    if (fa) {
        fprintf(fa, "%s,%s,%s,%s,%s,%s\n", chosenDoctor.doctorType,
                chosenDoctor.doctorName, chosenDoctor.area, chosenDoctor.hospital,
                selectedDay, selectedSlot);
        fclose(fa);
    } else {
        printf("⚠️ Could not save appointment.\n");
    }
}
