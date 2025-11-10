#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utensils.h"

/* auth-stdin mode:
   ./server auth-stdin
   The server reads two lines from stdin:
	 username\n
	 password\n
   and prints exactly one of:
	 AUTH_SUCCESS\n
	 AUTH_FAIL\n
*/
static int handle_auth_stdin(void) {
	char username[256];
	char password[256];

	if (!fgets(username, sizeof(username), stdin)) return 2;
	if (!fgets(password, sizeof(password), stdin)) return 2;

	/* trim newlines */
	username[strcspn(username, "\r\n")] = '\0';
	password[strcspn(password, "\r\n")] = '\0';

	if (authenticate_user(username, password)) {
		printf("AUTH_SUCCESS\n");
		return 0;
	} else {
		printf("AUTH_FAIL\n");
		return 1;
	}
}

/* signup-stdin mode:
	 ./server signup-stdin
	 The server reads two lines from stdin:
		 username\n
		 password\n
	 and prints exactly one of:
		 SIGNUP_SUCCESS\n
		 SIGNUP_FAIL\n
*/
static int handle_signup_stdin(void) {
		char username[256];
		char password[256];

		if (!fgets(username, sizeof(username), stdin)) return 2;
		if (!fgets(password, sizeof(password), stdin)) return 2;

		/* trim newlines */
		username[strcspn(username, "\r\n")] = '\0';
		password[strcspn(password, "\r\n")] = '\0';

		if (add_user(username, password)) {
				printf("SIGNUP_SUCCESS\n");
				return 0;
		} else {
				printf("SIGNUP_FAIL\n");
				return 1;
		}
}

int main(int argc, char *argv[]) {
	if (argc >= 2 && strcmp(argv[1], "auth-stdin") == 0) {
		return handle_auth_stdin();
	}
	if (argc >= 2 && strcmp(argv[1], "signup-stdin") == 0) {
		return handle_signup_stdin();
	}

	/* Normal server/demo mode (no-op for now) */
	puts("Server running in normal mode. Use './server auth-stdin' for auth checks.");
	return 0;
}
