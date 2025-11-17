#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appointment.h"
#include "main_menu.h"

void waitForEnter(void)
{
    int c;

    // Flush any previous input
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }

    printf("Press Enter to return to the main menu...");
    fflush(stdout);

    // Wait for ONLY Enter
    while ((c = getchar()) != '\n')
    {
        // Clear junk input
        while (c != '\n' && c != EOF)
        {
            c = getchar();
        }
        printf("Press Enter to Continue...");
        fflush(stdout);
    }
}

// ---------- MAIN MENU ----------
void mainMenu(const char *username)
{
    int choice;
    do
    {
        printf("\n=== Main Menu ===\n");
        printf("1. Book Appointment\n");
        printf("2. View Appointments\n");
        printf("3. Delete Appointment\n");
        printf("4. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
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
            printf("Logging out...\n");
            return;
        default:
            printf("Invalid choice! Try again.\n");
        }

    } while (choice != 4);
}

void viewAppointments(const char *username)
{
    char filename[MAX_LEN + 20];
    sprintf(filename, "%s_appointments.txt", username);

    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("❌ No appointments found for %s.\n", username);
        return;
    }

    printf("\n=== Your Appointments ===\n");

    char line[512];
    int count = 0;

    // Updated fields: date instead of day
    while (fgets(line, sizeof(line), fp))
    {
        char doctorType[100], doctorName[100], area[100], hospital[100], date[20], slot[50];

        if (sscanf(line, "%99[^,],%99[^,],%99[^,],%99[^,],%19[^,],%49[^\n]",
                   doctorType, doctorName, area, hospital, date, slot) == 6)
        {
            printf("\n-------------------------------------\n");
            printf("✅ Appointment #%d\n", ++count);
            printf("Doctor Type : %s\n", doctorType);
            printf("Doctor Name : %s\n", doctorName);
            printf("Area        : %s\n", area);
            printf("Hospital    : %s\n", hospital);
            printf("Date        : %s\n", date);
            printf("Time Slot   : %s\n", slot);
            printf("-------------------------------------\n");
        }
    }

    if (count == 0)
        printf("⚠️ No appointments booked yet.\n");

    fclose(fp);
    waitForEnter();
}

void deleteAppointment(const char *username)
{
    char filename[MAX_LEN + 30];
    sprintf(filename, "%s_appointments.txt", username);

    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("❌ No appointments found.\n");
        return;
    }

    Appointment list[500];
    char line[512];
    int count = 0;

    // Load appointments
    while (fgets(line, sizeof(line), fp))
    {
        if (sscanf(line, "%199[^,],%199[^,],%199[^,],%199[^,],%19[^,],%49[^\n]",
                   list[count].doctorType,
                   list[count].doctorName,
                   list[count].area,
                   list[count].hospital,
                   list[count].date,
                   list[count].slot) == 6)

            count++;
    }
    fclose(fp);

    if (count == 0)
    {
        printf("⚠️ No appointments booked.\n");
        return;
    }

    // Display formatted list with red ❌ icon
    printf("\n=== ❌ Delete Appointment ===\n");

    for (int i = 0; i < count; i++)
    {
        printf("\n-------------------------------------\n");
        printf("❌  [%d]\n", i + 1);
        printf("Doctor Type : %s\n", list[i].doctorType);
        printf("Doctor Name : %s\n", list[i].doctorName);
        printf("Area        : %s\n", list[i].area);
        printf("Hospital    : %s\n", list[i].hospital);
        printf("Date        : %s\n", list[i].date);
        printf("Time Slot   : %s\n", list[i].slot);
        printf("-------------------------------------\n");
    }

    // Choice
    int choice = -1;
    char buffer[50];

    while (1)
    {
        printf("\nEnter appointment number to delete: ");
        scanf("%49s", buffer);

        int isValid = 1;

        // check all characters are digits
        for (int i = 0; buffer[i] != '\0'; i++)
        {
            if (buffer[i] < '0' || buffer[i] > '9')
            {
                isValid = 0;
                break;
            }
        }

        if (!isValid)
        {
            printf("❌ Invalid input! Please enter a valid number.\n");
            continue; // ask again
        }

        choice = atoi(buffer);

        if (choice < 1 || choice > count)
        {
            printf("❌ Invalid choice! Choose a number from 1 to %d.\n", count);
            continue; // ask again
        }

        break; // valid!
    }

    // if (choice < 1 || choice > count)
    // {
    //     printf("❌ Invalid choice.\n");
    //     return;
    // }

    int delIndex = choice - 1;

    // Confirmation step
    char confirm;
    printf("\nAre you sure you want to delete this appointment? (y/n): ");
    scanf(" %c", &confirm);

    if (confirm == 'n' || confirm == 'N')
    {
        printf("\n❌ Deletion cancelled. Returning to main menu...\n");
        return;
    }

    // Personal File Delete
    else if (confirm == 'y' || confirm == 'Y')
    {
        char tempFile[MAX_LEN + 30];
        sprintf(tempFile, "%s_temp.txt", username);

        fp = fopen(filename, "r");
        FILE *temp = fopen(tempFile, "w");

        while (fgets(line, sizeof(line), fp))
        {
            Appointment a;

            if (sscanf(line, "%199[^,],%199[^,],%199[^,],%199[^,],%19[^,],%49[^\n]",
                       a.doctorType, a.doctorName, a.area, a.hospital, a.date, a.slot) != 6)
                continue;

            int match =
                strcmp(a.doctorType, list[delIndex].doctorType) == 0 &&
                strcmp(a.doctorName, list[delIndex].doctorName) == 0 &&
                strcmp(a.date, list[delIndex].date) == 0 &&
                strcmp(a.slot, list[delIndex].slot) == 0;

            if (!match)
            {
                fprintf(temp, "%s,%s,%s,%s,%s,%s\n",
                        a.doctorType, a.doctorName, a.area, a.hospital, a.date, a.slot);
            }
        }

        fclose(fp);
        fclose(temp);
        remove(filename);
        rename(tempFile, filename);

        // Global File Delete
        FILE *fg = fopen("all_appointments.txt", "r");
        FILE *fgTemp = fopen("all_appointments_tmp.txt", "w");

        while (fgets(line, sizeof(line), fg))
        {
            char user[200], t[200], n[200], ar[200], h[200], d[20], s[50];

            if (sscanf(line, "%199[^,],%199[^,],%199[^,],%199[^,],%199[^,],%19[^,],%49[^\n]",
                       user, t, n, ar, h, d, s) != 7)
                continue;

            int match =
                strcmp(user, username) == 0 &&
                strcmp(t, list[delIndex].doctorType) == 0 &&
                strcmp(n, list[delIndex].doctorName) == 0 &&
                strcmp(d, list[delIndex].date) == 0 &&
                strcmp(s, list[delIndex].slot) == 0;

            if (!match)
            {
                fprintf(fgTemp, "%s,%s,%s,%s,%s,%s,%s\n",
                        user, t, n, ar, h, d, s);
            }
        }

        fclose(fg);
        fclose(fgTemp);
        remove("all_appointments.txt");
        rename("all_appointments_tmp.txt", "all_appointments.txt");

        printf("\n✅ Appointment deleted successfully!\n");
    }
    else { 
        printf("\n❌ Invalid input. Deletion cancelled.\n"); 
    }
}
