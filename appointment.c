// appointment.c
// Calendar-grid booking (C1) with DD-MM-YYYY date format and global availability
// Saves into: <username>_appointments.txt  AND  all_appointments.txt
// all_appointments.txt format (CSV):
// username,doctorType,doctorName,area,hospital,DD-MM-YYYY,slot

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "appointment.h"

#define MAX_LEN 200
#define MAX_RECORDS 5000

/* ---------- utility: clear stdin ---------- */
static void flush_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

/* ---------- time parsing / formatting ---------- */
int parseTime(const char *t)
{
    int hour = 0, min = 0;
    char ampm[4] = {0};
    if (sscanf(t, "%d:%d%2s", &hour, &min, ampm) < 2)
        return -1;
    for (int i = 0; ampm[i]; ++i)
        ampm[i] = toupper((unsigned char)ampm[i]);
    if (strcmp(ampm, "PM") == 0 && hour != 12)
        hour += 12;
    if (strcmp(ampm, "AM") == 0 && hour == 12)
        hour = 0;
    return hour * 60 + min;
}

void formatTime(int mins, char *out, size_t outsz)
{
    int hour = mins / 60;
    int min = mins % 60;
    snprintf(out, outsz, "%02d:%02d", hour, min);
}

/* ---------- global availability check ---------- */
/* all_appointments.txt each line:
   username,doctorType,doctorName,area,hospital,DD-MM-YYYY,slot
   Return 1 if slot already booked by ANY user, else 0.
*/
int isSlotBookedGlobal(const char *doctorName, const char *dateStr, const char *slot)
{
    FILE *fp = fopen("all_appointments.txt", "r");
    if (!fp)
        return 0; // no global file yet => free

    char line[1024];
    while (fgets(line, sizeof(line), fp))
    {
        char u[200], dt[200], dn[200], area[200], hosp[200], date[64], fslot[64];
        if (sscanf(line, "%199[^,],%199[^,],%199[^,],%199[^,],%199[^,],%63[^,],%63[^\n]",
                   u, dt, dn, area, hosp, date, fslot) == 7)
        {
            if (strcmp(dn, doctorName) == 0 && strcmp(date, dateStr) == 0 && strcmp(fslot, slot) == 0)
            {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

/* ---------- load doctors ---------- */
int loadDoctors(Doctor doctors[])
{
    FILE *fp = fopen("database.txt", "r");
    if (!fp)
    {
        fp = fopen("doctors.txt", "r"); // fallback
        if (!fp)
        {
            printf("❌ database.txt / doctors.txt not found.\n");
            return 0;
        }
    }

    char line[600];
    int count = 0;

    if (fgets(line, sizeof(line), fp))
    {
        if (strstr(line, "doctor") || strstr(line, "DoctorType") || strstr(line, "doctorName"))
        {
            /* header detected -> skip */
        }
        else
        {
            /* first line is data -> rewind */
            fseek(fp, 0, SEEK_SET);
        }
    }
    else
    {
        fclose(fp);
        return 0;
    }

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;
        if (sscanf(line, "%199[^,],%199[^,],%199[^,],%199[^,],%199[^,],%199[^\n]",
                   doctors[count].doctorType,
                   doctors[count].doctorName,
                   doctors[count].area,
                   doctors[count].hospital,
                   doctors[count].workingDays,
                   doctors[count].time) == 6)
        {
            count++;
            if (count >= MAX_RECORDS)
                break;
        }
    }
    fclose(fp);
    return count;
}

/* ---------- helpers ---------- */
int add_unique(char list[][MAX_LEN], int *count, const char *value, int max)
{
    for (int i = 0; i < *count; ++i)
        if (strcmp(list[i], value) == 0)
            return 0;
    if (*count < max)
    {
        strncpy(list[*count], value, MAX_LEN - 1);
        list[*count][MAX_LEN - 1] = '\0';
        (*count)++;
        return 1;
    }
    return 0;
}

int days_in_month(int year, int month)
{
    static const int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int d = mdays[month - 1];
    if (month == 2)
    {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
            d = 29;
    }
    return d;
}

/* weekday for date: 0=Sun..6=Sat */
int weekday_of_date(int year, int month, int day)
{
    struct tm tmdate;
    memset(&tmdate, 0, sizeof(tmdate));
    tmdate.tm_year = year - 1900;
    tmdate.tm_mon = month - 1;
    tmdate.tm_mday = day;
    tmdate.tm_hour = 12; /* midday safe from DST */
    mktime(&tmdate);
    return tmdate.tm_wday;
}

/* compare dates: returns -1 if a < b, 0 equal, 1 if a > b */
int cmp_date(int ay, int am, int ad, int by, int bm, int bd)
{
    if (ay != by)
        return ay < by ? -1 : 1;
    if (am != bm)
        return am < bm ? -1 : 1;
    if (ad != bd)
        return ad < bd ? -1 : (ad > bd ? 1 : 0);
    return 0;
}

/* format date DD-MM-YYYY */
void format_date_ddmmyyyy(int d, int m, int y, char *out, size_t outsz)
{
    snprintf(out, outsz, "%02d-%02d-%04d", d, m, y);
}

/* parse doctor working days like "Sat Sun" into weekdayAvailable[0..6] (Sun..Sat) */
void parseWorkingDays(const char *working, int weekdayAvailable[7])
{
    for (int i = 0; i < 7; ++i)
        weekdayAvailable[i] = 0;
    if (!working || working[0] == '\0')
        return;
    char copy[MAX_LEN];
    strncpy(copy, working, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
    char *tok = strtok(copy, " ,");
    while (tok)
    {
        char t3[4] = {0};
        for (int i = 0; i < 3 && tok[i]; ++i)
            t3[i] = tolower((unsigned char)tok[i]);
        if (strncmp(t3, "sun", 3) == 0)
            weekdayAvailable[0] = 1;
        else if (strncmp(t3, "mon", 3) == 0)
            weekdayAvailable[1] = 1;
        else if (strncmp(t3, "tue", 3) == 0)
            weekdayAvailable[2] = 1;
        else if (strncmp(t3, "wed", 3) == 0)
            weekdayAvailable[3] = 1;
        else if (strncmp(t3, "thu", 3) == 0)
            weekdayAvailable[4] = 1;
        else if (strncmp(t3, "fri", 3) == 0)
            weekdayAvailable[5] = 1;
        else if (strncmp(t3, "sat", 3) == 0)
            weekdayAvailable[6] = 1;
        tok = strtok(NULL, " ,");
    }
}

/* ---------- Calendar grid (month view) ---------- */
/* returns: 1 and writes selectedDate (DD-MM-YYYY) if a date chosen
           0 if go back
          -2 on input error */
int selectDateCalendar(const Doctor *doctor, const char *username, char *selectedDateBuf, size_t bufsize)
{
    time_t nowt = time(NULL);
    struct tm now;
    localtime_r(&nowt, &now);
    int curD = now.tm_mday;
    int curM = now.tm_mon + 1;
    int curY = now.tm_year + 1900;

    int weekdayAvailable[7];
    parseWorkingDays(doctor->workingDays, weekdayAvailable);

    int maxAdvanceMonths = 3; /* inclusive */
    int displayY = curY, displayM = curM;

    int endY = curY, endM = curM + maxAdvanceMonths;
    while (endM > 12)
    {
        endM -= 12;
        endY++;
    }

    char cmd[8];
    while (1)
    {
        printf("\n=== Appointment Calendar ===\n");
        printf("Doctor works on: ");
        int printed = 0;
        const char *names[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        for (int i = 0; i < 7; ++i)
        {
            if (weekdayAvailable[i])
            {
                if (printed)
                    printf(" ");
                printf("%s", names[i]);
                printed = 1;
            }
        }
        if (!printed)
            printf("No regular working days listed.");
        printf("\n\n");

        const char *months[] = {"", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        printf("      %s %d\n", months[displayM], displayY);
        printf(" Su  Mo  Tu  We  Th  Fr  Sa\n");

        int firstW = weekday_of_date(displayY, displayM, 1);
        int dim = days_in_month(displayY, displayM);

        int col = 0;
        for (int i = 0; i < firstW; ++i)
        {
            printf("    ");
            col++;
        }

        for (int day = 1; day <= dim; ++day)
        {
            int wday = (firstW + (day - 1)) % 7;
            int cmp = cmp_date(displayY, displayM, day, curY, curM, curD);
            int isPast = (cmp == -1);
            int works = weekdayAvailable[wday];
            if (isPast)
                printf(" %2d ", day);
            else if (works)
                printf(" [%2d]", day);
            else
                printf(" %2d ", day);
            col++;
            if (col == 7)
            {
                printf("\n");
                col = 0;
            }
        }
        if (col != 0)
            printf("\n");

        printf("\nOptions:\n");
        printf("D - Select date   N - Next month   P - Previous month   0 - Go Back\n");
        printf("(You can browse up to %d months ahead)\n", maxAdvanceMonths);
        printf("Enter choice: ");
        if (scanf(" %7s", cmd) != 1)
        {
            flush_stdin();
            return -2;
        }
        for (int i = 0; cmd[i]; ++i)
            cmd[i] = toupper((unsigned char)cmd[i]);

        if (strcmp(cmd, "0") == 0)
            return 0;
        if (strcmp(cmd, "N") == 0)
        {
            int ny = displayY, nm = displayM + 1;
            if (nm > 12)
            {
                nm = 1;
                ny++;
            }
            if (cmp_date(ny, nm, 1, endY, endM, 1) == 1)
            {
                printf("❌ Cannot go beyond %02d-%04d\n", endM, endY);
                continue;
            }
            displayY = ny;
            displayM = nm;
            continue;
        }
        if (strcmp(cmd, "P") == 0)
        {
            int py = displayY, pm = displayM - 1;
            if (pm < 1)
            {
                pm = 12;
                py--;
            }
            if (cmp_date(py, pm, 1, curY, curM, 1) == -1)
            {
                printf("❌ Cannot go before current month.\n");
                continue;
            }
            displayY = py;
            displayM = pm;
            continue;
        }
        if (strcmp(cmd, "D") == 0)
        {
            int dayChoice;
            printf("Enter day number: ");
            if (scanf("%d", &dayChoice) != 1)
            {
                flush_stdin();
                printf("❌ Invalid input.\n");
                continue;
            }
            if (dayChoice < 1 || dayChoice > days_in_month(displayY, displayM))
            {
                printf("❌ Invalid day for this month.\n");
                continue;
            }
            if (cmp_date(displayY, displayM, dayChoice, curY, curM, curD) == -1)
            {
                printf("❌ Can't choose past date.\n");
                continue;
            }
            int wd = weekday_of_date(displayY, displayM, dayChoice);
            if (!weekdayAvailable[wd])
            {
                const char *fullnames[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
                printf("❌ Doctor does not work on %s. Please select a valid date.\n", fullnames[wd]);
                continue;
            }
            format_date_ddmmyyyy(dayChoice, displayM, displayY, selectedDateBuf, bufsize);
            flush_stdin();
            return 1;
        }

        printf("❌ Invalid option.\n");
    }
    /* unreachable */
    return -2;
}

/* ---------- select time slot for chosen date ---------- */
/* returns:
     >0 : chosen slot index among available (1-based)
      0 : go back
     -2 : input error
     -3 : no slots available
*/
int selectTimeSlotForDate(const Doctor *doctor, const char *dateStr, char slotsOut[][32])
{
    // ----- Build all 30-min slots -----
    char copy[MAX_LEN];
    strncpy(copy, doctor->time, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *s1 = strtok(copy, "-");
    char *s2 = strtok(NULL, "-");
    if (!s1 || !s2)
    {
        printf("Invalid doctor time format.\n");
        return -2;
    }

    int start = parseTime(s1);
    int end = parseTime(s2);
    if (start < 0 || end <= start)
    {
        printf("Invalid time range.\n");
        return -2;
    }

    int scount = 0;
    char t1[16], t2[16];

    for (int t = start; t + 30 <= end && scount < 50; t += 30)
    {
        formatTime(t, t1, sizeof(t1));
        formatTime(t + 30, t2, sizeof(t2));
        snprintf(slotsOut[scount++], 32, "%s-%s", t1, t2);
    }

    if (scount == 0)
    {
        printf("No slots generated.\n");
        return -2;
    }

RESELECT_SLOTS:

    // ----- Get today's date/time -----
    time_t nowt = time(NULL);
    struct tm now;
    localtime_r(&nowt, &now);

    char todayStr[20];
    format_date_ddmmyyyy(now.tm_mday, now.tm_mon + 1, now.tm_year + 1900, todayStr, sizeof(todayStr));

    int currentMinutes = now.tm_hour * 60 + now.tm_min;

    // ----- Build available slots -----
    int indexMap[50];
    int availCount = 0;

    printf("\n=== Available slots on %s ===\n", dateStr);

    for (int i = 0; i < scount; i++)
    {
        int sh, sm, eh, em;
        sscanf(slotsOut[i], "%d:%d-%d:%d", &sh, &sm, &eh, &em);

        int slotStart = sh * 60 + sm;
        int slotEnd = eh * 60 + em;

        // ❌ FIX: Prevent booking if slot **start** time has passed today
        if (strcmp(dateStr, todayStr) == 0)
        {
            if (slotStart <= currentMinutes)
                continue;
        }

        // Global availability check
        if (!isSlotBookedGlobal(doctor->doctorName, dateStr, slotsOut[i]))
        {
            printf("%d. %s\n", availCount + 1, slotsOut[i]);
            indexMap[availCount++] = i;
        }
    }

    if (availCount == 0)
    {
        printf("⚠️ No slots available on %s.\n", dateStr);
        return -3;
    }

    printf("0. Go Back\n");
    printf("Enter slot number: ");
    int choice;

    if (scanf("%d", &choice) != 1)
    {
        flush_stdin();
        return -2;
    }
    flush_stdin();

    if (choice == 0)
        return 0;

    if (choice < 1 || choice > availCount)
    {
        printf("❌ Invalid choice.\n");
        return -2;
    }

    // ----- Save what user THINKS they selected -----
    int userAssumedSlotIndex = indexMap[choice - 1];

    char assumedSlot[32];
    strcpy(assumedSlot, slotsOut[userAssumedSlotIndex]);

    // ----- Recompute availability again to avoid race conditions -----
    int newIndexMap[50];
    int newCount = 0;

    for (int i = 0; i < scount; i++)
    {
        int sh, sm, eh, em;
        sscanf(slotsOut[i], "%d:%d-%d:%d", &sh, &sm, &eh, &em);

        int slotStart = sh * 60 + sm;
        int slotEnd = eh * 60 + em;

        if (strcmp(dateStr, todayStr) == 0)
        {
            if (slotStart <= currentMinutes)
                continue;
        }

        if (!isSlotBookedGlobal(doctor->doctorName, dateStr, slotsOut[i]))
        {
            newIndexMap[newCount++] = i;
        }
    }

    // ----- Validate race condition -----

    // If total available slots changed
    if (choice > newCount)
    {
        printf("\n⚠ Slot availability changed due to another booking.\n");
        printf("Please select again.\n");
        goto RESELECT_SLOTS;
    }

    int newRealIndex = newIndexMap[choice - 1];

    // If the actual slot text changed (someone booked)
    if (strcmp(assumedSlot, slotsOut[newRealIndex]) != 0)
    {
        printf("\n⚠ Slot was just booked by another user.\n");
        printf("Updated slots available. Please select again.\n");
        goto RESELECT_SLOTS;
    }

    // ----- VALID final slot -----
    return choice;
}

/* ---------- Selection helpers (type / area / hospital / doctor) ---------- */
int selectDoctorType(Doctor doctors[], int total, int prevChoice)
{
    char types[200][MAX_LEN];
    int tcount = 0;
    for (int i = 0; i < total; ++i)
        add_unique(types, &tcount, doctors[i].doctorType, 200);
    if (tcount == 0)
        return -2;
    printf("\n=== Step 1: Select Doctor Type ===\n");
    for (int i = 0; i < tcount; ++i)
        printf("%d. %s\n", i + 1, types[i]);
    printf("0. Return to Main Menu\n");
    if (prevChoice >= 1 && prevChoice <= tcount)
        printf("Previously selected: %s (%d)\n", types[prevChoice - 1], prevChoice);
    int choice;
    printf("Enter choice: ");
    if (scanf("%d", &choice) != 1)
    {
        flush_stdin();
        return -2;
    }
    if (choice == 0)
    {
        flush_stdin();
        return 0;
    }
    if (choice < 1 || choice > tcount)
    {
        flush_stdin();
        printf("❌ Invalid choice.\n");
        return -2;
    }
    flush_stdin();
    return choice;
}

int selectArea(Doctor doctors[], int total, const char *doctorType, int prevChoice)
{
    char areas[400][MAX_LEN];
    int ac = 0;
    for (int i = 0; i < total; ++i)
        if (strcmp(doctors[i].doctorType, doctorType) == 0)
            add_unique(areas, &ac, doctors[i].area, 400);
    if (ac == 0)
        return -2;
    printf("\n=== Step 2: Select Area ===\n");
    for (int i = 0; i < ac; ++i)
        printf("%d. %s\n", i + 1, areas[i]);
    printf("0. Go Back\n");
    if (prevChoice >= 1 && prevChoice <= ac)
        printf("Previously selected: %s (%d)\n", areas[prevChoice - 1], prevChoice);
    int choice;
    printf("Enter choice: ");
    if (scanf("%d", &choice) != 1)
    {
        flush_stdin();
        return -2;
    }
    if (choice == 0)
    {
        flush_stdin();
        return 0;
    }
    if (choice < 1 || choice > ac)
    {
        flush_stdin();
        printf("❌ Invalid choice.\n");
        return -2;
    }
    flush_stdin();
    return choice;
}

int selectHospital(Doctor doctors[], int total, const char *type, const char *area, int prevChoice, char hospitals[][MAX_LEN])
{
    int hc = 0;
    for (int i = 0; i < total; ++i)
        if (strcmp(doctors[i].doctorType, type) == 0 && strcmp(doctors[i].area, area) == 0)
            add_unique(hospitals, &hc, doctors[i].hospital, 200);
    if (hc == 0)
        return -2;
    printf("\n=== Step 3: Select Hospital ===\n");
    for (int i = 0; i < hc; ++i)
        printf("%d. %s\n", i + 1, hospitals[i]);
    printf("0. Go Back\n");
    if (prevChoice >= 1 && prevChoice <= hc)
        printf("Previously selected: %s (%d)\n", hospitals[prevChoice - 1], prevChoice);
    int choice;
    printf("Enter choice: ");
    if (scanf("%d", &choice) != 1)
    {
        flush_stdin();
        return -2;
    }
    if (choice == 0)
    {
        flush_stdin();
        return 0;
    }
    if (choice < 1 || choice > hc)
    {
        flush_stdin();
        printf("❌ Invalid choice.\n");
        return -2;
    }
    flush_stdin();
    return choice;
}

int selectDoctor(Doctor doctors[], int total, const char *type, const char *area, const char *hospital, int prevChoice, Doctor available[])
{
    int ac = 0;
    for (int i = 0; i < total; ++i)
    {
        if (strcmp(doctors[i].doctorType, type) == 0 && strcmp(doctors[i].area, area) == 0 && strcmp(doctors[i].hospital, hospital) == 0)
        {
            if (ac < 200)
                available[ac++] = doctors[i];
        }
    }
    if (ac == 0)
        return -2;
    printf("\n=== Step 4: Select Doctor ===\n");
    for (int i = 0; i < ac; ++i)
        printf("%d. %s — %s — %s\n", i + 1, available[i].doctorName, available[i].workingDays, available[i].time);
    printf("0. Go Back\n");
    if (prevChoice >= 1 && prevChoice <= ac)
        printf("Previously selected: %s (%d)\n", available[prevChoice - 1].doctorName, prevChoice);
    int choice;
    printf("Enter choice: ");
    if (scanf("%d", &choice) != 1)
    {
        flush_stdin();
        return -2;
    }
    if (choice == 0)
    {
        flush_stdin();
        return 0;
    }
    if (choice < 1 || choice > ac)
    {
        flush_stdin();
        printf("❌ Invalid choice.\n");
        return -2;
    }
    flush_stdin();
    return choice;
}

/* ---------- Main booking flow (calendar-based) ---------- */
void bookAppointment(const char *username)
{
    Doctor doctors[MAX_RECORDS];
    int total = loadDoctors(doctors);
    if (total == 0)
    {
        printf("⚠️ No doctor data loaded.\n");
        return;
    }

    int step = 1;
    int typePrev = -1, areaPrev = -1, hospPrev = -1, docPrev = -1, slotPrev = -1;

    char selectedType[MAX_LEN] = {0};
    char selectedArea[MAX_LEN] = {0};
    char hospitals[200][MAX_LEN];
    Doctor availDoctors[200];
    char selectedDate[20] = {0};
    char slotsAll[50][32];

    Doctor chosenDoctor;
    char chosenHospital[MAX_LEN] = {0};

    while (1)
    {
        if (step == 1)
        {
            int r = selectDoctorType(doctors, total, typePrev);
            if (r == 0)
            {
                printf("Returning to main menu.\n");
                return;
            }
            if (r > 0)
            {
                char types[200][MAX_LEN];
                int tcount = 0;
                for (int i = 0; i < total; ++i)
                    add_unique(types, &tcount, doctors[i].doctorType, 200);
                typePrev = r;
                strncpy(selectedType, types[r - 1], MAX_LEN - 1);
                areaPrev = hospPrev = docPrev = slotPrev = -1;
                step = 2;
            }
            else
                continue;
        }

        if (step == 2)
        {
            int r = selectArea(doctors, total, selectedType, areaPrev);
            if (r == 0)
            {
                step = 1;
                continue;
            }
            if (r > 0)
            {
                char areas[400][MAX_LEN];
                int ac = 0;
                for (int i = 0; i < total; ++i)
                    if (strcmp(doctors[i].doctorType, selectedType) == 0)
                        add_unique(areas, &ac, doctors[i].area, 400);
                areaPrev = r;
                strncpy(selectedArea, areas[r - 1], MAX_LEN - 1);
                hospPrev = docPrev = slotPrev = -1;
                step = 3;
            }
            else
                continue;
        }

        if (step == 3)
        {
            int r = selectHospital(doctors, total, selectedType, selectedArea, hospPrev, hospitals);
            if (r == 0)
            {
                step = 2;
                continue;
            }
            if (r > 0)
            {
                int hc = 0;
                for (int i = 0; i < total; ++i)
                    if (strcmp(doctors[i].doctorType, selectedType) == 0 && strcmp(doctors[i].area, selectedArea) == 0)
                        add_unique(hospitals, &hc, doctors[i].hospital, 200);
                hospPrev = r;
                strncpy(chosenHospital, hospitals[r - 1], MAX_LEN - 1);
                docPrev = slotPrev = -1;
                step = 4;
            }
            else
                continue;
        }

        if (step == 4)
        {
            int r = selectDoctor(doctors, total, selectedType, selectedArea, chosenHospital, docPrev, availDoctors);
            if (r == 0)
            {
                step = 3;
                continue;
            }
            if (r > 0)
            {
                docPrev = r;
                chosenDoctor = availDoctors[r - 1];
                slotPrev = -1;
                step = 5;
            }
            else
                continue;
        }

        if (step == 5)
        {
            int sd = selectDateCalendar(&chosenDoctor, username, selectedDate, sizeof(selectedDate));
            if (sd == 0)
            {
                step = 4;
                continue;
            }
            if (sd == -2)
            {
                printf("❌ Date selection error.\n");
                return;
            }
            if (sd == 1)
            {
                int slotChoice = selectTimeSlotForDate(&chosenDoctor, selectedDate, slotsAll);
                if (slotChoice == 0)
                {
                    step = 5;
                    continue;
                }
                if (slotChoice == -3)
                {
                    step = 5;
                    continue;
                }
                if (slotChoice <= -2)
                {
                    printf("❌ Slot selection error.\n");
                    return;
                }

                /* reconstruct all slots & same filtering to map choice -> final slot */
                char copy[MAX_LEN];
                strncpy(copy, chosenDoctor.time, sizeof(copy) - 1);
                copy[sizeof(copy) - 1] = 0;
                char *a = strtok(copy, "-");
                char *b = strtok(NULL, "-");
                int start = parseTime(a), end = parseTime(b);
                int scount = 0;
                char allslots[50][32];
                char t1[16], t2[16];
                for (int tt = start; tt + 30 <= end && scount < 50; tt += 30)
                {
                    formatTime(tt, t1, sizeof(t1));
                    formatTime(tt + 30, t2, sizeof(t2));
                    snprintf(allslots[scount++], sizeof(allslots[0]), "%s-%s", t1, t2);
                }

                time_t nowt = time(NULL);
                struct tm now;
                localtime_r(&nowt, &now);
                char todayStr[20];
                format_date_ddmmyyyy(now.tm_mday, now.tm_mon + 1, now.tm_year + 1900, todayStr, sizeof(todayStr));
                int currentMinutes = now.tm_hour * 60 + now.tm_min;

                int indexMap[50];
                int availCount = 0;
                for (int i = 0; i < scount; ++i)
                {
                    int sh = 0, sm = 0, eh = 0, em = 0;
                    if (sscanf(allslots[i], "%d:%d-%d:%d", &sh, &sm, &eh, &em) != 4)
                        continue;
                    int slotStart = sh * 60 + sm;
                    if (strcmp(selectedDate, todayStr) == 0)
                    {
                        if (slotStart <= currentMinutes)
                            continue; /* IMPORTANT: skip if start passed */
                    }
                    if (!isSlotBookedGlobal(chosenDoctor.doctorName, selectedDate, allslots[i]))
                    {
                        indexMap[availCount++] = i;
                    }
                }

                if (slotChoice < 1 || slotChoice > availCount)
                {
                    printf("❌ Invalid final choice.\n");
                    continue;
                }
                int chosenIdx = indexMap[slotChoice - 1];
                char finalSlot[32];
                strncpy(finalSlot, allslots[chosenIdx], sizeof(finalSlot) - 1);
                finalSlot[31] = '\0';

                /* write user file */
                char userfile[MAX_LEN];
                snprintf(userfile, sizeof(userfile), "%s_appointments.txt", username);
                FILE *fu = fopen(userfile, "a");
                if (fu)
                {
                    fprintf(fu, "%s,%s,%s,%s,%s,%s\n",
                            chosenDoctor.doctorType,
                            chosenDoctor.doctorName,
                            chosenDoctor.area,
                            chosenDoctor.hospital,
                            selectedDate,
                            finalSlot);
                    fclose(fu);
                }
                else
                {
                    printf("⚠️ Could not write to user file %s\n", userfile);
                }

                /* write global file */
                FILE *fg = fopen("all_appointments.txt", "a");
                if (fg)
                {
                    fprintf(fg, "%s,%s,%s,%s,%s,%s,%s\n",
                            username,
                            chosenDoctor.doctorType,
                            chosenDoctor.doctorName,
                            chosenDoctor.area,
                            chosenDoctor.hospital,
                            selectedDate,
                            finalSlot);
                    fclose(fg);
                }
                else
                {
                    printf("⚠️ Could not write to global file all_appointments.txt\n");
                }

                /* Confirmation and wait for Enter */
                printf("\n✔ Appointment Booked Successfully!\n");
                printf("----------------------------------------\n");
                printf("Doctor:   %s (%s)\n", chosenDoctor.doctorName, chosenDoctor.doctorType);
                printf("Hospital: %s\n", chosenDoctor.hospital);
                printf("Area:     %s\n", chosenDoctor.area);
                printf("Date:     %s\n", selectedDate);
                printf("Slot:     %s\n", finalSlot);
                printf("----------------------------------------\n");
                printf("\nPress Enter to return to the Main Menu...");

                int c = getchar();

                if (c != '\n')
                    getchar();

                return;
            }
        }
    }
}
