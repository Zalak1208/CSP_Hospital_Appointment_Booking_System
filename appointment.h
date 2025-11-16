#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#define MAX_LEN 200
#define MAX_RECORDS 5000

// ---------- Doctor structure ----------
typedef struct {
    char doctorType[MAX_LEN];
    char doctorName[MAX_LEN];
    char area[MAX_LEN];
    char hospital[MAX_LEN];
    char workingDays[50];
    char time[50];
} Doctor;

// ---------- Appointment structure ----------
typedef struct {
    char doctorType[MAX_LEN];
    char doctorName[MAX_LEN];
    char area[MAX_LEN];
    char hospital[MAX_LEN];
    char date[20];      
    char slot[50];      
} Appointment;

// ---------- Public function ----------
void bookAppointment(const char *username);

// ---------- Helper functions ----------
int parseTime(const char *t);
void formatTime(int mins, char *out, size_t outsz);
int loadDoctors(Doctor doctors[]);
int isSlotBooked(const char *username, const char *doctorName,
                 const char *day, const char *slot);
int add_unique(char list[][200], int *count, const char *value, int max);

int selectDoctorType(Doctor doctors[], int total, int prevChoice);
int selectArea(Doctor doctors[], int total, const char *currentType, int prevChoice);
int selectHospital(Doctor doctors[], int total, const char *currentType,
                   const char *currentArea, int prevChoice, char hospitals[][200]);
int selectDoctor(Doctor doctors[], int total, const char *currentType,
                 const char *currentArea, const char *currentHospital,
                 int prevChoice, Doctor available[]);
int selectDay(const Doctor *doctor, int prevChoice, char daysOut[][20]);
int selectTimeSlot(const Doctor *doctor, const char *day, int prevChoice,
                   char slotsOut[][32], const char *username);

#endif
