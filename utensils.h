#ifndef UTENSILS_H
#define UTENSILS_H

#include <stdbool.h>

/* Authenticate a username/password against users.txt.
 * Returns 1 on success, 0 on failure.
 */
int authenticate_user(const char *username, const char *password);
/* Add a new user to users.txt. Returns 1 on success, 0 on failure
 * Fails if username already exists or input is invalid.
 */
int add_user(const char *username, const char *password);

#endif // UTENSILS_H
