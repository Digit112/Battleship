#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "battleship.hpp"

using namespace battleship;

// Attempts to find a copy of "substr" in "str" and returns the index of the first character in "str" where a match was found.
// The search can be limited to the range str[start] - str[end] (inclusive)
// Both strings are expected to be null-terminated.
int match(char* substr, char* str, int start, int end) {
	int chk = 0;
	for (int i = start; true; i++) {
		if (str[i] == '\0' || i > end) return -1;
		if (substr[chk] == '\0') return i-chk;
		if (str[i] == substr[chk]) chk++;
	}
}

// Converts all uppercase characters in the string to lowercase characters
char* lowercase(char* str) {
	for (int i = 0; true; i++) {
		if (str[i] == '\0') return str;
		
		if (str[i] >= 65 && str[i] <= 90) {
			str[i] = str[i] | 0x20;
		}
	}
}

// Removes leading and trailing spaces, tabs, and newlines from a string
char* trim(char* str) {
	int fc = -1;
	int lc = -1;
	
	// Find first char
	for (int i = 0; true; i++) {
		if (str[i] == '\0') break;
		
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
			fc = i;
			break;
		}
	}
	
	// If there is none and it is all whitespace, return an empty string.
	if (fc == -1) {
		str[0] = '\0';
		return str;
	}
	
	// Find last char
	lc = fc;
	for (int i = fc+1; true; i++) {
		if (str[i] == '\0') break;
		
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
			lc = i;
		}
	}
	
	int o = 0;
	for (int n = fc; n <= lc; n++, o++) {
		str[o] = str[n];
	}
	str[o] = '\0';
	
	return str;
}

// Replaces all sequences of whitespace characters with a single space
char* clean_ws(char* str) {
	bool is_in_ws = false;
	int o = 0;
	
	for (int i = 0; true; i++) {
		if (str[i] == '\0') {
			str[o] = '\0';
			return str;
		}
		
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
			is_in_ws = false;
			str[o] = str[i];
			o++;
		} else {
			if (is_in_ws) {
				continue;
			} else {
				is_in_ws = true;
				str[o] = ' ';
				o++;
			}
		}
	}
}

// Replace all spaces with null-terminating characters and return a pointer to each new string created by this process. The result is returned to argret, the function returns the number of args found.
// Will return after maxargs arguments are found or a null terminating character is found.
int toargs(char* str, char** argret, int maxargs) {
	if (maxargs == 0) return 0;
	
	int argretn = 1;
	argret[0] = str;
	
	for (int i = 0; true; i++) {
		if (str[i] == '\0') return argretn;
		
		if (str[i] == ' ') {
			str[i] = '\0';
			
			if (argretn == maxargs) {
				return argretn;
			}
			
			argret[argretn] = str+i+1;
			argretn++;
		}
	}
}

// Returns if these strings are identical in contents.
bool str_equ(char* a, char* b) {
	for (int i = 0; true; i++) {
		if (a[i] != b[i]) return false;
		if (a[i] == '\0') return true;
	}
}

// This is identical to match(). I don't want to delete it because I love it.
// int m(char* s,char* S,int b,int e){int c=0;for(int i=b;1;i++){if(S[i]==0||i>e)return -1;if(s[c]==0)return i-c;if(S[i]==s[c])c++;}}

int main() {
	char* buf = (char*) malloc(256);
	char** args = (char**) malloc(32 * sizeof(char**));
	int argsn;
	int m;
	
	// Used by server + client
	bool is_server;
	int addrlen = sizeof(sockaddr_in);
	double con_timer;
	bool timeout;
	
	char* msgbuf = (char*) malloc(512);
	char* sndbuf = (char*) malloc(512);
	
	// Used by server
	int s_con;
	int s_valread;
	struct sockaddr_in clientaddr;
	
	int cli_socket;
	int opt = 1;
	
	// Used by client
	int c_con;
	int c_valread;
	struct sockaddr_in server_addr;
	
	// used by client/server
	
	while (true) {
		printf(">");
		fgets(buf, 256, stdin);
		argsn = toargs(clean_ws(lowercase(trim(buf))), args, 32);
		
		if (argsn == 0) {continue;}
		else if (str_equ(args[0], (char*) "quit")) {break;}
		
		else if (str_equ(args[0], (char*) "server")) {
			if (argsn < 2) {printf("Usage: server <port>\n"); continue;}
			
			int port = atoi(args[1]);
			printf("Creating server on port %d.\n", port);
			
			s_con = socket(AF_INET, SOCK_STREAM, 0);
			if (s_con == 0) {
				printf("Failed to get socket descriptor.\n");
				continue;
			}
			
			if (setsockopt(s_con, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
				printf("Failed to set socket options.\n");
				continue;
			}
			
			clientaddr.sin_family = AF_INET;
			clientaddr.sin_addr.s_addr = INADDR_ANY;
			clientaddr.sin_port = htons(port);
			
			if (bind(s_con, (struct sockaddr*) &clientaddr, sizeof(clientaddr)) < 0) {
				printf("Failed to bind socket.\n");
				continue;
			}
			
			is_server = true;
			
			printf("Server creation Successful. Now awaiting incoming client...\n");
			
			if (listen(s_con, 3) < 0) {
				printf("\nListen failed.\n");
				is_server = false;
				continue;
			}
			
			// Get confirmation message
			if ((cli_socket = accept(s_con, (struct sockaddr*) &clientaddr, (socklen_t*) &addrlen)) < 0) {
				printf("Failed to accept connection.\n");
				is_server = false;
				continue;
			}
			
			s_valread = read(cli_socket, msgbuf, 512);
			
			if (str_equ(msgbuf, (char*) "battleship")) {
				printf("Connection established. Beginning game loop.\n");
			} else {
				printf("Erroneous confirmation message \"%s\"\n", msgbuf);
				continue;
			}
			
			send(cli_socket, "battleship", 11, 0);
			
			// seed for the current time and send that seed to the client.
			int seed = time(NULL);
			srand(seed);
			
			sprintf(sndbuf, "%d", seed);
			send(cli_socket, sndbuf, strlen(sndbuf), 0);
			
			game bsg;
			bsg.randomize();
			
			int x; int y; int s;
			bool game_over = false;
			while (true) {
				bsg.show(RED);
				printf("It's your turn.\n>");
				
				while (true) {
					fgets(buf, 256, stdin);
					trim(lowercase(buf));
					
					y = buf[0] - 97; // Convert from char lowercase letter
					x = buf[1] - 48; // Convert from char number
					
					if (x < 0 || x >= bsg.width || y < 0 || y >= bsg.height) {
						printf("Invalid Location.\n>");
						continue;
					}
					else if (bsg.grid[x + y*bsg.width] & 0x04) {
						printf("This location has already been hit!\n");
						continue;
					}  else {
						s = bsg.fire(RED, x, y);
						
						if (s > -1) {
							printf("Hit!\n");
							if (!bsg.ships_b[s].is_alive) {
								printf("You sunk a ship!\n");
								
								if (bsg.check_win(RED)) {
									game_over = true;
									printf("Game Over! You've won!\n");
								}
							}
						} else {
							printf("Miss!\n");
						}
						
						((int*) sndbuf)[0] = x;
						((int*) sndbuf)[1] = y;
						send(cli_socket, sndbuf, sizeof(int) * 2, 0);
						break;
					}
				}
				
				if (game_over) {
					break;
				}
				
				bsg.show(RED);
				printf("It's your oponent's turn.\n");
				
				c_valread = read(cli_socket, msgbuf, 512);
				x = ((int*) msgbuf)[0];
				y = ((int*) msgbuf)[1];
				
				printf("Your oponent fired at %c%c, and it ", y + 65, x + 48);
				s = bsg.fire(BLUE, x, y);
				if (s > -1) {
					printf("hit!\n");
					if (!bsg.ships_r[s].is_alive) {
						printf("Your oponent sunk a ship!\n");
						
						if (bsg.check_win(BLUE)) {
							game_over = true;
							printf("Game Over! Your oponent has won.\n");
							break;
						}
					}
				}
				else {
					printf("missed!\n");
				}
			}
			
			close(cli_socket);
			close(s_con);
		}
		else if (str_equ(args[0], (char*) "connect")) {
			if (argsn < 3) {printf("Usage: server <ip address> <port>\n"); continue;}
			
			int port = atoi(args[2]);
			
			if ((c_con = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				printf("Failed to get socket descriptor.\n");
				continue;
			}
			
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(port);
			
			if (inet_pton(AF_INET, args[1], &server_addr.sin_addr) <= 0) {
				printf("Invalid ip address \"%s\"\n", args[1]);
				continue;
			}
			
			if (connect(c_con, (struct sockaddr*) &server_addr, addrlen) < 0) {
				printf("Connection failed.\n");
				continue;
			}
			
			send(c_con, (char*) "battleship", 11, 0);
			
			c_valread = read(c_con, msgbuf, 512);
			
			if (str_equ(msgbuf, (char*) "battleship")) {
				printf("Connection established. Beginning game loop.\n");
			} else {
				printf("Erroneous confirmation message \"%s\"\n", msgbuf);
				continue;
			}
			
			c_valread = read(c_con, msgbuf, 512);
			
			srand(atoi(msgbuf));
			
			game bsg;
			bsg.randomize();
			
			int x; int y; int s;
			bool game_over = false;
			while (true) {
				bsg.show(BLUE);
				printf("It's your oponent's turn.\n");
				
				c_valread = read(c_con, msgbuf, 512);
				x = ((int*) msgbuf)[0];
				y = ((int*) msgbuf)[1];
				printf("Your oponent fired at %c%c, and it ", y + 65, x + 48);
				s = bsg.fire(RED, x, y);
				if (s > -1) {
					printf("hit!\n");
					if (!bsg.ships_b[s].is_alive) {
						printf("Your oponent sunk a ship!\n");
						
						if (bsg.check_win(RED)) {
							game_over = true;
							printf("Game Over! Your oponent has won.\n");
							break;
						}
					}
				} else {
					printf("missed!\n");
				}
				
				bsg.show(BLUE);
				printf("It's your turn.\n>");
				
				while (true) {
					fgets(buf, 256, stdin);
					trim(lowercase(buf));
					
					y = buf[0] - 97; // Convert from char lowercase letter
					x = buf[1] - 48; // Convert from char number
					
					if (x < 0 || x >= bsg.width || y < 0 || y >= bsg.height) {
						printf("Invalid Location.\n>");
						continue;
					}
					else if (bsg.grid[x + y*bsg.width] & 0x01) {
						printf("This location has already been hit!\n>");
						continue;
					} else {
						s = bsg.fire(BLUE, x, y);
						
						if (s > -1) {
							printf("Hit!\n");
							if (!bsg.ships_r[s].is_alive) {
								printf("You sunk a ship!\n");
								
								if (bsg.check_win(BLUE)) {
									game_over = true;
									printf("Game Over! You've Won!\n");
								}
							}
						}
						else {
							printf("Miss!\n");
						}
						
						((int*) sndbuf)[0] = x;
						((int*) sndbuf)[1] = y;
						send(c_con, sndbuf, sizeof(int) * 2, 0);
						break;
					}
				}
				
				if (game_over) {
					break;
				}
			}
			
			close(c_con);
		}
		else {
			printf("Unknown command. Got: {%s", args[0]);
			for (int i = 1; i < argsn; i++) {
				printf(", %s", args[i]);
			}
			printf("}\n");
		}
	}
	
	printf("Goodbye\n");
	
	free(msgbuf);
	free(sndbuf);
	
	free(buf);
	free(args);
}
