#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Simple client that prompts for username/password and sends them to
 * ./server auth-stdin via pipes. The server will respond with a single
 * line: AUTH_SUCCESS or AUTH_FAIL.
 */
int main(void) {
	char username[128];
	char *password;
	char buf[16];

	printf("Do you want to (l)ogin or (s)ignup? [l/s]: ");
	if (!fgets(buf, sizeof(buf), stdin)) return 1;
	char mode = buf[0];

	if (mode == 's' || mode == 'S') {
		/* Signup flow */
		printf("Choose a username: ");
		if (!fgets(username, sizeof(username), stdin)) return 1;
		username[strcspn(username, "\r\n")] = '\0';
		password = getpass("Choose a password: ");
		if (!password) return 1;

		/* spawn server signup-stdin */
		int p_to_c[2];
		int c_to_p[2];
		if (pipe(p_to_c) == -1) { perror("pipe"); return 1; }
		if (pipe(c_to_p) == -1) { perror("pipe"); return 1; }
		pid_t pid = fork();
		if (pid < 0) { perror("fork"); return 1; }
		if (pid == 0) {
			dup2(p_to_c[0], STDIN_FILENO);
			dup2(c_to_p[1], STDOUT_FILENO);
			close(p_to_c[0]); close(p_to_c[1]);
			close(c_to_p[0]); close(c_to_p[1]);
			execl("./server", "./server", "signup-stdin", (char *)NULL);
			perror("execl");
			_exit(127);
		}
		close(p_to_c[0]); close(c_to_p[1]);
		dprintf(p_to_c[1], "%s\n%s\n", username, password);
		close(p_to_c[1]);
		char resp[128];
		ssize_t n = read(c_to_p[0], resp, sizeof(resp)-1);
		if (n > 0) {
			resp[n] = '\0';
			resp[strcspn(resp, "\r\n")] = '\0';
		}
		close(c_to_p[0]);
		waitpid(pid, NULL, 0);
		if (n > 0 && strcmp(resp, "SIGNUP_SUCCESS") == 0) {
			printf("Signup successful. You can now login.\n");
			/* fall through to login prompt */
		} else {
			printf("Signup failed.\n");
			return 1;
		}
	}

	/* Login flow (either chosen initially or after successful signup)") */
	printf("Username: ");
	if (!fgets(username, sizeof(username), stdin)) return 1;
	username[strcspn(username, "\r\n")] = '\0';
	password = getpass("Password: ");
	if (!password) return 1;

	int p_to_c[2];
	int c_to_p[2];
	if (pipe(p_to_c) == -1) { perror("pipe"); return 1; }
	if (pipe(c_to_p) == -1) { perror("pipe"); return 1; }

	pid_t pid = fork();
	if (pid < 0) { perror("fork"); return 1; }

	if (pid == 0) {
		dup2(p_to_c[0], STDIN_FILENO);
		dup2(c_to_p[1], STDOUT_FILENO);
		close(p_to_c[0]); close(p_to_c[1]);
		close(c_to_p[0]); close(c_to_p[1]);
		execl("./server", "./server", "auth-stdin", (char *)NULL);
		perror("execl");
		_exit(127);
	} else {
		close(p_to_c[0]);
		close(c_to_p[1]);
		dprintf(p_to_c[1], "%s\n%s\n", username, password);
		close(p_to_c[1]);
		char resp[128];
		ssize_t n = read(c_to_p[0], resp, sizeof(resp)-1);
		if (n <= 0) {
			fprintf(stderr, "No response from server\n");
			close(c_to_p[0]);
			waitpid(pid, NULL, 0);
			return 1;
		}
		resp[n] = '\0';
		resp[strcspn(resp, "\r\n")] = '\0';
		close(c_to_p[0]);
		waitpid(pid, NULL, 0);
		if (strcmp(resp, "AUTH_SUCCESS") == 0) {
			printf("Login successful.\n");
			return 0;
		} else {
			printf("Login failed. Would you like to sign up? [y/n]: ");
			if (!fgets(buf, sizeof(buf), stdin)) return 1;
			if (buf[0] == 'y' || buf[0] == 'Y') {
				/* perform signup by reusing the signup flow (simple: call server directly) */
				int sp_to_c[2];
				int sc_to_p[2];
				if (pipe(sp_to_c) == -1) { perror("pipe"); return 1; }
				if (pipe(sc_to_p) == -1) { perror("pipe"); return 1; }
				pid_t spid = fork();
				if (spid < 0) { perror("fork"); return 1; }
				if (spid == 0) {
					dup2(sp_to_c[0], STDIN_FILENO);
					dup2(sc_to_p[1], STDOUT_FILENO);
					close(sp_to_c[0]); close(sp_to_c[1]);
					close(sc_to_p[0]); close(sc_to_p[1]);
					execl("./server", "./server", "signup-stdin", (char *)NULL);
					perror("execl");
					_exit(127);
				}
				close(sp_to_c[0]); close(sc_to_p[1]);
				dprintf(sp_to_c[1], "%s\n%s\n", username, password);
				close(sp_to_c[1]);
				char sresp[128];
				ssize_t sn = read(sc_to_p[0], sresp, sizeof(sresp)-1);
				if (sn > 0) { sresp[sn] = '\0'; sresp[strcspn(sresp, "\r\n")] = '\0'; }
				close(sc_to_p[0]);
				waitpid(spid, NULL, 0);
				if (sn > 0 && strcmp(sresp, "SIGNUP_SUCCESS") == 0) {
					printf("Signup successful. Try logging in again.\n");
					return 0;
				} else {
					printf("Signup failed.\n");
					return 1;
				}
			}
			return 1;
		}
	}
}
