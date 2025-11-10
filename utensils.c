#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utensils.h"

/* Trim leading/trailing whitespace (in-place). */
static void trim(char *s) {
	char *start = s;
	while (*start && isspace((unsigned char)*start)) start++;
	if (start != s) memmove(s, start, strlen(start) + 1);
	char *end = s + strlen(s) - 1;
	while (end >= s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
}

int authenticate_user(const char *username, const char *password) {
	if (!username || !password) return 0;

	FILE *fp = fopen("users.txt", "r");
	if (!fp) return 0;

	char line[512];
	int ok = 0;
	while (fgets(line, sizeof(line), fp)) {
		char *p = strchr(line, ':');
		if (!p) continue;
		*p = '\0';
		char userbuf[256];
		char passbuf[256];
		strncpy(userbuf, line, sizeof(userbuf)-1); userbuf[sizeof(userbuf)-1] = '\0';
		strncpy(passbuf, p+1, sizeof(passbuf)-1); passbuf[sizeof(passbuf)-1] = '\0';
		trim(userbuf);
		trim(passbuf);
		/* remove any trailing \r or \n from passbuf */
		char *nl = strpbrk(passbuf, "\r\n");
		if (nl) *nl = '\0';

		if (strcmp(userbuf, username) == 0 && strcmp(passbuf, password) == 0) {
			ok = 1;
			break;
		}
	}

	fclose(fp);
	return ok;
}

/* Add a new user (username:password) to users.txt. Returns 1 on success, 0 on failure. */
int add_user(const char *username, const char *password) {
	if (!username || !password) return 0;

	/* basic validation: non-empty, no ':' in username or password */
	if (username[0] == '\0' || password[0] == '\0') return 0;
	if (strchr(username, ':') || strchr(password, ':')) return 0;

	/* ensure user does not already exist */
	FILE *fp = fopen("users.txt", "r");
	if (fp) {
		char line[512];
		while (fgets(line, sizeof(line), fp)) {
			char *p = strchr(line, ':');
			if (!p) continue;
			*p = '\0';
			char userbuf[256];
			strncpy(userbuf, line, sizeof(userbuf)-1);
			userbuf[sizeof(userbuf)-1] = '\0';
			trim(userbuf);
			if (strcmp(userbuf, username) == 0) {
				fclose(fp);
				return 0; /* already exists */
			}
		}
		fclose(fp);
	}

	/* append new user */
	fp = fopen("users.txt", "a");
	if (!fp) return 0;
	/* write as username:password\n */
	if (fprintf(fp, "%s:%s\n", username, password) < 0) {
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}
