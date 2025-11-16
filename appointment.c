#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "appointment.h"
#include <stdbool.h>
#include <sys/file.h>
#include <unistd.h>

#define MAX_LEN 200
#define MAX_RECORDS 3000

// --- New mapping data structures (dynamic-array based) ---
typedef struct {
    int start; /* minutes */
    int end;   /* minutes */
} Slot;

typedef struct {
    char name[32];
    Slot *slots;
    int slot_count;
} DayNode;

typedef struct {
    char name[MAX_LEN]; /* doctor name */
    DayNode *days;
    int day_count;
} DoctorNode;

typedef struct {
    char name[MAX_LEN]; /* hospital name */
    DoctorNode *doctors;
    int doctor_count;
} HospitalNode;

typedef struct {
    char name[MAX_LEN]; /* location/area */
    HospitalNode *hospitals;
    int hospital_count;
} LocationNode;

typedef struct {
    char name[MAX_LEN]; /* disease */
    LocationNode *locations;
    int loc_count;
} DiseaseNode;

typedef struct {
    DiseaseNode *diseases;
    int disease_count;
} Catalog;

/* canonical record for each database line */
typedef struct {
    char disease[MAX_LEN];
    char location[MAX_LEN];
    char hospital[MAX_LEN];
    char doctor[MAX_LEN];
    char days[MAX_LEN];
    char time[MAX_LEN];
} Record;

/* Global booking record (7 fields from appointments_log.txt) */
typedef struct {
    char username[MAX_LEN];
    char disease[MAX_LEN];
    char doctor[MAX_LEN];
    char hospital[MAX_LEN];
    char area[MAX_LEN];
    char day[32];
    char slot[32];
} Booking;

/* In-memory index of all bookings (7-field records) */
typedef struct {
    Booking *bookings;
    int count;
    int capacity;
} BookingIndex;

/* Normalize in-place: trim and lowercase */
static void normalize(char *s) {
    /* trim */
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
    /* lowercase */
    for (char *p = s; *p; ++p) *p = (char) tolower((unsigned char)*p);
}

/* Trim in-place (no lowercase) */
static void trim(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
}

/* load all lines from database.txt into records array */
static int loadRecords(Record **out) {
    FILE *fp = fopen("database.txt", "r");
    if (!fp) return 0;
    Record *arr = NULL;
    int cap = 0, n = 0;
    char line[MAX_LEN * 3];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;
        char type[MAX_LEN], doctor[MAX_LEN], area[MAX_LEN], hospital[MAX_LEN], days[MAX_LEN], timebuf[MAX_LEN];
        /* expecting CSV: type,doctor,area,hospital,days,time */
        if (sscanf(line, "%199[^,],%199[^,],%199[^,],%199[^,],%199[^,],%199[^\n]", type, doctor, area, hospital, days, timebuf) < 6) {
            /* try fallback split by comma using strtok */
            char temp[MAX_LEN*3]; strncpy(temp, line, sizeof(temp)); temp[sizeof(temp)-1]='\0';
            char *t = strtok(temp, ",");
            if (!t) continue; strncpy(type, t, sizeof(type));
            t = strtok(NULL, ","); if (!t) continue; strncpy(doctor, t, sizeof(doctor));
            t = strtok(NULL, ","); if (!t) continue; strncpy(area, t, sizeof(area));
            t = strtok(NULL, ","); if (!t) continue; strncpy(hospital, t, sizeof(hospital));
            t = strtok(NULL, ","); if (!t) continue; strncpy(days, t, sizeof(days));
            t = strtok(NULL, "\n"); if (!t) continue; strncpy(timebuf, t, sizeof(timebuf));
        }
        if (n + 1 > cap) {
            cap = cap ? cap * 2 : 64;
            arr = realloc(arr, sizeof(Record) * cap);
        }
        strncpy(arr[n].disease, type, MAX_LEN-1); arr[n].disease[MAX_LEN-1] = '\0';
        strncpy(arr[n].doctor, doctor, MAX_LEN-1); arr[n].doctor[MAX_LEN-1] = '\0';
        strncpy(arr[n].location, area, MAX_LEN-1); arr[n].location[MAX_LEN-1] = '\0';
        strncpy(arr[n].hospital, hospital, MAX_LEN-1); arr[n].hospital[MAX_LEN-1] = '\0';
        strncpy(arr[n].days, days, MAX_LEN-1); arr[n].days[MAX_LEN-1] = '\0';
        strncpy(arr[n].time, timebuf, MAX_LEN-1); arr[n].time[MAX_LEN-1] = '\0';
        /* normalize keys for lookups */
        normalize(arr[n].disease);
        normalize(arr[n].location);
        n++;
    }
    fclose(fp);
    *out = arr;
    return n;
}

/* Helpers: find or create nodes (linear search) */
static DiseaseNode *get_or_create_disease(Catalog *c, const char *name) {
    for (int i = 0; i < c->disease_count; ++i) if (strcmp(c->diseases[i].name, name) == 0) return &c->diseases[i];
    c->diseases = realloc(c->diseases, sizeof(DiseaseNode) * (c->disease_count + 1));
    DiseaseNode *dn = &c->diseases[c->disease_count++];
    strncpy(dn->name, name, MAX_LEN-1); dn->name[MAX_LEN-1] = '\0';
    dn->locations = NULL; dn->loc_count = 0;
    return dn;
}

static LocationNode *get_or_create_location(DiseaseNode *d, const char *name) {
    for (int i = 0; i < d->loc_count; ++i) if (strcmp(d->locations[i].name, name) == 0) return &d->locations[i];
    d->locations = realloc(d->locations, sizeof(LocationNode) * (d->loc_count + 1));
    LocationNode *ln = &d->locations[d->loc_count++];
    strncpy(ln->name, name, MAX_LEN-1); ln->name[MAX_LEN-1] = '\0';
    ln->hospitals = NULL; ln->hospital_count = 0;
    return ln;
}

static HospitalNode *get_or_create_hospital(LocationNode *l, const char *name) {
    for (int i = 0; i < l->hospital_count; ++i) if (strcmp(l->hospitals[i].name, name) == 0) return &l->hospitals[i];
    l->hospitals = realloc(l->hospitals, sizeof(HospitalNode) * (l->hospital_count + 1));
    HospitalNode *hn = &l->hospitals[l->hospital_count++];
    strncpy(hn->name, name, MAX_LEN-1); hn->name[MAX_LEN-1] = '\0';
    hn->doctors = NULL; hn->doctor_count = 0;
    return hn;
}

static DoctorNode *get_or_create_doctor(HospitalNode *h, const char *name) {
    for (int i = 0; i < h->doctor_count; ++i) if (strcmp(h->doctors[i].name, name) == 0) return &h->doctors[i];
    h->doctors = realloc(h->doctors, sizeof(DoctorNode) * (h->doctor_count + 1));
    DoctorNode *dn = &h->doctors[h->doctor_count++];
    strncpy(dn->name, name, MAX_LEN-1); dn->name[MAX_LEN-1] = '\0';
    dn->days = NULL; dn->day_count = 0;
    return dn;
}

static DayNode *get_or_create_day(DoctorNode *d, const char *name) {
    for (int i = 0; i < d->day_count; ++i) if (strcmp(d->days[i].name, name) == 0) return &d->days[i];
    d->days = realloc(d->days, sizeof(DayNode) * (d->day_count + 1));
    DayNode *dn = &d->days[d->day_count++];
    strncpy(dn->name, name, sizeof(dn->name)-1); dn->name[sizeof(dn->name)-1] = '\0';
    dn->slots = NULL; dn->slot_count = 0;
    return dn;
}

/* split time range into 30-minute slots */
static Slot *split_time_range(const char *range, int *out_count) {
    char tmp[MAX_LEN]; strncpy(tmp, range, sizeof(tmp)); tmp[sizeof(tmp)-1]='\0';
    char *dash = strchr(tmp, '-');
    if (!dash) { *out_count = 0; return NULL; }
    *dash = '\0';
    char *start = tmp; char *end = dash + 1;
    normalize(start); normalize(end);
    int s = parseTime(start);
    int e = parseTime(end);
    if (e <= s) { *out_count = 0; return NULL; }
    int cap = (e - s) / 30 + 1;
    Slot *arr = malloc(sizeof(Slot) * cap);
    int cnt = 0;
    for (int t = s; t + 30 <= e; t += 30) {
        arr[cnt].start = t; arr[cnt].end = t + 30; cnt++;
    }
    *out_count = cnt;
    return arr;
}

/* build catalog from records */
static void build_catalog_from_records(Record *recs, int n, Catalog *cat) {
    memset(cat, 0, sizeof(*cat));
    for (int i = 0; i < n; ++i) {
        DiseaseNode *dn = get_or_create_disease(cat, recs[i].disease);
        LocationNode *ln = get_or_create_location(dn, recs[i].location);
        HospitalNode *hn = get_or_create_hospital(ln, recs[i].hospital);
        DoctorNode *docn = get_or_create_doctor(hn, recs[i].doctor);
        char dayscopy[MAX_LEN]; strncpy(dayscopy, recs[i].days, sizeof(dayscopy)); dayscopy[sizeof(dayscopy)-1]='\0';
        char *tok = strtok(dayscopy, " ");
        while (tok) {
            DayNode *dayn = get_or_create_day(docn, tok);
            int sc = 0; Slot *slots = split_time_range(recs[i].time, &sc);
            if (slots) {
                for (int si = 0; si < sc; ++si) {
                    int exists = 0;
                    for (int k = 0; k < dayn->slot_count; ++k) if (dayn->slots[k].start == slots[si].start && dayn->slots[k].end == slots[si].end) { exists = 1; break; }
                    if (!exists) {
                        dayn->slots = realloc(dayn->slots, sizeof(Slot) * (dayn->slot_count + 1));
                        dayn->slots[dayn->slot_count++] = slots[si];
                    }
                }
                free(slots);
            }
            tok = strtok(NULL, " ");
        }
    }
}

/* ========== Global Booking Functions ========== */

/* Load appointments_log.txt into BookingIndex (7 fields: username,disease,doctor,hospital,area,day,slot) */
static int loadAppointmentsLog(const char *path, BookingIndex *idx) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        idx->bookings = NULL;
        idx->count = 0;
        idx->capacity = 0;
        return 0;
    }
    idx->bookings = NULL;
    idx->count = 0;
    idx->capacity = 0;
    char line[MAX_LEN * 10];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;
        char buf[MAX_LEN * 10]; strncpy(buf, line, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
        char *tok[7] = {0};
        tok[0] = strtok(buf, ",");
        for (int i = 1; i < 7 && tok[i-1]; ++i) tok[i] = strtok(NULL, ",");
        if (!tok[6]) continue;
        if (idx->count >= idx->capacity) {
            idx->capacity = idx->capacity ? idx->capacity * 2 : 64;
            idx->bookings = realloc(idx->bookings, sizeof(Booking) * idx->capacity);
        }
        Booking *b = &idx->bookings[idx->count];
        strncpy(b->username, tok[0] ? tok[0] : "", MAX_LEN-1); b->username[MAX_LEN-1] = '\0'; trim(b->username);
        strncpy(b->disease, tok[1] ? tok[1] : "", MAX_LEN-1); b->disease[MAX_LEN-1] = '\0'; trim(b->disease);
        strncpy(b->doctor, tok[2] ? tok[2] : "", MAX_LEN-1); b->doctor[MAX_LEN-1] = '\0'; trim(b->doctor);
        strncpy(b->hospital, tok[3] ? tok[3] : "", MAX_LEN-1); b->hospital[MAX_LEN-1] = '\0'; trim(b->hospital);
        strncpy(b->area, tok[4] ? tok[4] : "", MAX_LEN-1); b->area[MAX_LEN-1] = '\0'; trim(b->area);
        strncpy(b->day, tok[5] ? tok[5] : "", 31); b->day[31] = '\0'; trim(b->day);
        strncpy(b->slot, tok[6] ? tok[6] : "", 31); b->slot[31] = '\0'; trim(b->slot);
        idx->count++;
    }
    fclose(fp);
    return idx->count;
}

/* Free BookingIndex memory */
static void freeBookingIndex(BookingIndex *idx) {
    if (idx->bookings) free(idx->bookings);
    idx->bookings = NULL;
    idx->count = 0;
    idx->capacity = 0;
}

/* Check if a slot is booked globally (case-insensitive for doctor/day, exact for slot) */
static int isSlotBookedGlobal(const BookingIndex *idx, const char *doctor, const char *day, const char *slot) {
    if (!idx || !idx->bookings) return 0;
    char d_lower[MAX_LEN], day_lower[32], s_lower[32];
    strncpy(d_lower, doctor ? doctor : "", sizeof(d_lower)-1); d_lower[sizeof(d_lower)-1] = '\0';
    strncpy(day_lower, day ? day : "", sizeof(day_lower)-1); day_lower[sizeof(day_lower)-1] = '\0';
    strncpy(s_lower, slot ? slot : "", sizeof(s_lower)-1); s_lower[sizeof(s_lower)-1] = '\0';
    for (char *p = d_lower; *p; ++p) *p = tolower((unsigned char)*p);
    for (char *p = day_lower; *p; ++p) *p = tolower((unsigned char)*p);
    for (int i = 0; i < idx->count; ++i) {
        char b_doc[MAX_LEN], b_day[32];
        strncpy(b_doc, idx->bookings[i].doctor, sizeof(b_doc)-1); b_doc[sizeof(b_doc)-1] = '\0';
        strncpy(b_day, idx->bookings[i].day, sizeof(b_day)-1); b_day[sizeof(b_day)-1] = '\0';
        for (char *p = b_doc; *p; ++p) *p = tolower((unsigned char)*p);
        for (char *p = b_day; *p; ++p) *p = tolower((unsigned char)*p);
        if (strcmp(b_doc, d_lower) == 0 && strcmp(b_day, day_lower) == 0 && strcmp(idx->bookings[i].slot, s_lower) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Record a booking to appointments_log.txt with file locking (append-only, atomic) */
static int recordGlobalBooking(const char *path, BookingIndex *idx, const Booking *b) {
    if (!b) return -1;
    FILE *fp = fopen(path, "a");
    if (!fp) {
        perror("fopen appointments_log.txt");
        return -1;
    }
    if (flock(fileno(fp), LOCK_EX) == -1) {
        perror("flock LOCK_EX");
        fclose(fp);
        return -1;
    }

    loadAppointmentsLog(path, idx);
    
    if (isSlotBookedGlobal(idx, b->doctor, b->day, b->slot)) {
        printf("⚠️ Slot was just booked by another user. Please select a different slot.\n");
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return -1;
    }
    fprintf(fp, "%s,%s,%s,%s,%s,%s,%s\n", b->username, b->disease, b->doctor, b->hospital, b->area, b->day, b->slot);
    fflush(fp);
    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    if (idx->count >= idx->capacity) {
        idx->capacity = idx->capacity ? idx->capacity * 2 : 64;
        idx->bookings = realloc(idx->bookings, sizeof(Booking) * idx->capacity);
    }
    idx->bookings[idx->count++] = *b;
    return 0;
}

/* ========== End Global Booking Functions ========== */

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

// --- Utility: check if slot already booked (per-user cache check) ---
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

/* Helper: read a line and parse integer; returns:
   >=0 : parsed integer (0 allowed to indicate "back")
   -1  : invalid input / parse error
   -2  : EOF or interrupt (treat as cancel)
*/
static int read_choice(void) {
    char buf[128];
    if (!fgets(buf, sizeof(buf), stdin)) return -2;
    buf[strcspn(buf, "\n")] = '\0';
    trim(buf);
    if (strlen(buf) == 0) return -1;
    if (strcmp(buf, "0") == 0) return 0;
    char *endptr = NULL;
    long v = strtol(buf, &endptr, 10);
    if (endptr == buf || *endptr != '\0') return -1;
    return (int)v;
}

// ----------------------------------------
// Book Appointment Function (mapping-based)
// ----------------------------------------
void bookAppointment(const char *username) {
    Record *recs = NULL;
    int n = loadRecords(&recs);
    if (n == 0) {
        printf("⚠️ No doctor data loaded.\n");
        return;
    }

    Catalog cat = {0};
    build_catalog_from_records(recs, n, &cat);

    if (cat.disease_count == 0) {
        printf("⚠️ No diseases found in catalog.\n");
        free(recs);
        return;
    }

    int step = 0; /* 0=disease,1=location,2=hospital,3=doctor,4=day,5=slot */
    int di = -1, li = -1, hi = -1, dpi = -1, dayi = -1;
    BookingIndex global_booking_index = {NULL, 0, 0};

    for (;;) {
        if (step == 0) {
            printf("\n=== Diseases ===\n");
            for (int i = 0; i < cat.disease_count; ++i) printf("%d. %s\n", i+1, cat.diseases[i].name);
            printf("Enter disease number (0 to return to main menu): ");
            int choice = read_choice();
            if (choice == -2) { /* EOF */ break; }
            if (choice == -1) { printf("Invalid choice\n"); continue; }
            if (choice == 0) { /* return to main menu */ break; }
            if (choice < 1 || choice > cat.disease_count) { printf("Invalid choice\n"); continue; }
            di = choice - 1; step = 1; continue;
        }

        if (step == 1) {
            DiseaseNode *dnode = &cat.diseases[di];
            if (dnode->loc_count == 0) { printf("No locations for that disease\n"); step = 0; continue; }
            printf("\n=== Locations for %s ===\n", dnode->name);
            for (int i = 0; i < dnode->loc_count; ++i) printf("%d. %s\n", i+1, dnode->locations[i].name);
            printf("Enter location number (0 to go back): ");
            int choice = read_choice();
            if (choice == -2) break;
            if (choice == -1) { printf("Invalid choice\n"); continue; }
            if (choice == 0) { step = 0; continue; }
            if (choice < 1 || choice > dnode->loc_count) { printf("Invalid choice\n"); continue; }
            li = choice - 1; step = 2; continue;
        }

        if (step == 2) {
            DiseaseNode *dnode = &cat.diseases[di];
            LocationNode *lnode = &dnode->locations[li];
            if (lnode->hospital_count == 0) { printf("No hospitals for that location\n"); step = 1; continue; }
            printf("\n=== Hospitals in %s for %s ===\n", lnode->name, dnode->name);
            for (int i = 0; i < lnode->hospital_count; ++i) printf("%d. %s\n", i+1, lnode->hospitals[i].name);
            printf("Enter hospital number (0 to go back): ");
            int choice = read_choice();
            if (choice == -2) break;
            if (choice == -1) { printf("Invalid choice\n"); continue; }
            if (choice == 0) { step = 1; continue; }
            if (choice < 1 || choice > lnode->hospital_count) { printf("Invalid choice\n"); continue; }
            hi = choice - 1; step = 3; continue;
        }

        if (step == 3) {
            DiseaseNode *dnode = &cat.diseases[di];
            LocationNode *lnode = &dnode->locations[li];
            HospitalNode *hnode = &lnode->hospitals[hi];
            if (hnode->doctor_count == 0) { printf("No doctors at that hospital\n"); step = 2; continue; }
            printf("\n=== Doctors at %s ===\n", hnode->name);
            for (int i = 0; i < hnode->doctor_count; ++i) printf("%d. %s\n", i+1, hnode->doctors[i].name);
            printf("Enter doctor number (0 to go back): ");
            int choice = read_choice();
            if (choice == -2) break;
            if (choice == -1) { printf("Invalid choice\n"); continue; }
            if (choice == 0) { step = 2; continue; }
            if (choice < 1 || choice > hnode->doctor_count) { printf("Invalid choice\n"); continue; }
            dpi = choice - 1; step = 4; continue;
        }

        if (step == 4) {
            DiseaseNode *dnode = &cat.diseases[di];
            LocationNode *lnode = &dnode->locations[li];
            HospitalNode *hnode = &lnode->hospitals[hi];
            DoctorNode *docnode = &hnode->doctors[dpi];
            if (docnode->day_count == 0) { printf("No schedule for that doctor\n"); step = 3; continue; }
            printf("\n=== Days for Dr. %s ===\n", docnode->name);
            for (int i = 0; i < docnode->day_count; ++i) printf("%d. %s\n", i+1, docnode->days[i].name);
            printf("Enter day number (0 to go back): ");
            int choice = read_choice();
            if (choice == -2) break;
            if (choice == -1) { printf("Invalid choice\n"); continue; }
            if (choice == 0) { step = 3; continue; }
            if (choice < 1 || choice > docnode->day_count) { printf("Invalid choice\n"); continue; }
            dayi = choice - 1; step = 5; continue;
        }

        if (step == 5) {
            DiseaseNode *dnode = &cat.diseases[di];
            LocationNode *lnode = &dnode->locations[li];
            HospitalNode *hnode = &lnode->hospitals[hi];
            DoctorNode *docnode = &hnode->doctors[dpi];
            DayNode *daynode = &docnode->days[dayi];

            loadAppointmentsLog("appointments_log.txt", &global_booking_index);

            if (daynode->slot_count == 0) { printf("No slots available on that day\n"); step = 4; continue; }
            printf("\n=== Available Slots for Dr. %s on %s ===\n", docnode->name, daynode->name);
            int *slotMap = malloc(sizeof(int) * daynode->slot_count);
            if (!slotMap) { printf("Memory error\n"); break; }
            int availCount = 0;
            for (int i = 0; i < daynode->slot_count; ++i) {
                char s1[6], s2[6], slotstr[32];
                formatTime(daynode->slots[i].start, s1);
                formatTime(daynode->slots[i].end, s2);
                snprintf(slotstr, sizeof(slotstr), "%s-%s", s1, s2);
                if (!isSlotBookedGlobal(&global_booking_index, docnode->name, daynode->name, slotstr)) {
                    printf("%d. %s\n", availCount+1, slotstr);
                    slotMap[availCount] = i;
                    availCount++;
                }
            }
            if (availCount == 0) { printf("No available slots (all booked)\n"); free(slotMap); step = 4; continue; }
            printf("Enter slot number (0 to go back): ");
            int choice = read_choice();
            if (choice == -2) { free(slotMap); break; }
            if (choice == -1) { printf("Invalid choice\n"); free(slotMap); continue; }
            if (choice == 0) { free(slotMap); step = 4; continue; }
            if (choice < 1 || choice > availCount) { printf("Invalid choice\n"); free(slotMap); continue; }
            int sloti = slotMap[choice - 1];

            char s1[6], s2[6], slotstr[32];
            formatTime(daynode->slots[sloti].start, s1);
            formatTime(daynode->slots[sloti].end, s2);
            snprintf(slotstr, sizeof(slotstr), "%s-%s", s1, s2);

            Booking b = {0};
            strncpy(b.username, username, MAX_LEN-1); b.username[MAX_LEN-1] = '\0';
            strncpy(b.disease, cat.diseases[di].name, MAX_LEN-1); b.disease[MAX_LEN-1] = '\0';
            strncpy(b.doctor, docnode->name, MAX_LEN-1); b.doctor[MAX_LEN-1] = '\0';
            strncpy(b.hospital, hnode->name, MAX_LEN-1); b.hospital[MAX_LEN-1] = '\0';
            strncpy(b.area, lnode->name, MAX_LEN-1); b.area[MAX_LEN-1] = '\0';
            strncpy(b.day, daynode->name, 31); b.day[31] = '\0';
            strncpy(b.slot, slotstr, 31); b.slot[31] = '\0';

            if (recordGlobalBooking("appointments_log.txt", &global_booking_index, &b) != 0) {
                printf("Could not record booking to global log\n");
                free(slotMap); break;
            }

            char filename[MAX_LEN]; sprintf(filename, "%s_appointments.txt", username);
            FILE *fa = fopen(filename, "a");
            if (fa) {
                fprintf(fa, "%s,%s,%s,%s,%s,%s\n",
                        cat.diseases[di].name,
                        docnode->name,
                        hnode->name,
                        lnode->name,
                        daynode->name,
                        slotstr);
                fclose(fa);
            }

            printf("\n✅ Appointment booked successfully!\n");
            printf("Doctor: %s\nHospital: %s\nArea: %s\nDay: %s\nSlot: %s\n",
                   docnode->name, hnode->name, lnode->name, daynode->name, slotstr);

            free(slotMap);
            break; /* done */
        }
    }

    free(recs);
    freeBookingIndex(&global_booking_index);
}
