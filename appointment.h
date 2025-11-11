#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#define MAX_LEN 200

typedef struct Appointment {
    char doctorType[MAX_LEN];
    char doctorName[MAX_LEN];
    char area[MAX_LEN];
    char hospital[MAX_LEN];
    char workingDays[MAX_LEN];
    char time[MAX_LEN];
} Appointment;

void bookAppointment(const char *username);
void viewAppointments(const char *username);
void editAppointment(const char *username);
void deleteAppointment(const char *username);

#endif // APPOINTMENT_H
