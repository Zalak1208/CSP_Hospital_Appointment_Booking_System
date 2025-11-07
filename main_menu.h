// main_menu.h
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#define MAX_LEN 100

struct Appointment {
    char username[MAX_LEN];
    char doctorType[MAX_LEN];
    char date[20];
    char time[10];
};

// Function declarations
void mainMenu(char *username);
void bookAppointment(char *username);
void viewAppointments(char *username);
void deleteAppointment(char *username);
void editAppointment(char *username);

#endif
