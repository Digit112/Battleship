#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "battleship.hpp"
#include "str.cpp"

using namespace battleship;

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
	
	printf("Battleship!\nUse \"help\" to list commands.\n");
	
	while (true) {
		printf(">");
		fgets(buf, 256, stdin);
		argsn = toargs(clean_ws(lowercase(trim(buf))), args, 32);
		
		if (argsn == 0) {continue;}
		else if (str_equ(args[0], (char*) "quit")) {break;}
		
		else if (str_equ(args[0], (char*) "help")) {
			printf("help:\n  Shows help page.\n\nquit:\n  Exits the application.\n\nserver <port>:\n  Creates a server on the specified port and waits until a client attempts to connect.\n\nconnect <ip address> <port>:\n  Attempt to connect to a server running on the given IP address, over the given port.\n");
		}
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
			
			read(cli_socket, msgbuf, 512);
			
			if (str_equ(msgbuf, (char*) "battleship")) {
				printf("Connection established. Beginning game loop.\n");
			} else {
				printf("Erroneous confirmation message \"%s\"\n", msgbuf);
				continue;
			}
			
			send(cli_socket, "battleship", 11, 0);
			
			// Wait 10000 clock cycles for the client to read the confirmation message.
			int t_s = clock();
			while (clock() - t_s < 10000) {}
			
			// seed for the current time and send that seed to the client.
			int seed = time(NULL);
			srand(seed);
			
			printf("Sending seed %d\n", seed);
			*((int*) sndbuf) = seed;
			send(cli_socket, sndbuf, sizeof(int), 0);
			printf("Sent\n");
			
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
					
					if (str_equ(buf, (char*) "quit")) {
						printf("Are you sure you'd like to quit? (y/n):\n>");
						fgets(buf, 256, stdin);
						trim(lowercase(buf));
						
						if (buf[0] == 'y') {
							sprintf(sndbuf, (char*) "quit");
							send(cli_socket, sndbuf, 5, 0);
							game_over = true;
							printf("Quitting game.\n");
							break;
						} else {
							printf(">");
							continue;
						}
					}
					
					if (str_equ(buf, (char*) "help")) {
						printf("The game is played on two 10x10 grids, one containing your ships and another containing your opponent's ships. Only your ships are visible to you.\n");
						printf("Each grid square is two characters wide and one character tall in order to make them square shaped.\n");
						printf("Your grid is on the left and your opponent's is on the right. Your ships are indicated by grid squares containing numbers (the numbers themselves identify the ship).\n");
						printf("For each turn, you can supply a letter-number combination such as \"A4\" or \"G6\" which will specify a grid square of your oponent's to fire at. If that square contained a ship, it will be replaced by an \"XX\" to indicate the hit, or an \"OO\" if the square was empty.\n");
						printf("You and your oponent alternate turns, their hits will appear on your grid in the same way that yours appear on theirs.\n");
						printf("The goal is to destroy all of your oponent's ships before they destroy yours.\n");
						printf("You may run \"quit\" during your turn to end the game early.\n>");
						continue;
					}
					
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
				
				read(cli_socket, msgbuf, 512);
				
				if (str_equ(msgbuf, (char*) "quit")) {
					printf("Your oponent has quit the game.\n");
					game_over = true;
					break;
				}
				
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
			
			read(c_con, msgbuf, 512);
			
			if (str_equ(msgbuf, (char*) "battleship")) {
				printf("Connection established. Beginning game loop.\n");
			} else {
				printf("Erroneous confirmation message \"%s\"\n", msgbuf);
				continue;
			}
			
			printf("Waiting for seed...\n");
			read(c_con, msgbuf, 512);
			printf("Seed: %d\n", *((int*) msgbuf));
			
			srand(*((int*) msgbuf));
			
			game bsg;
			bsg.randomize();
			
			int x; int y; int s;
			bool game_over = false;
			while (true) {
				bsg.show(BLUE);
				printf("It's your oponent's turn.\n");
				
				read(c_con, msgbuf, 512);
				
				if (str_equ(msgbuf, (char*) "quit")) {
					printf("Your oponent has quit the game.\n");
					game_over = true;
					break;
				}
				
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
					
					if (str_equ(buf, (char*) "quit")) {
						printf("Are you sure you'd like to quit? (y/n):\n>");
						fgets(buf, 256, stdin);
						trim(lowercase(buf));
						
						if (buf[0] == 'y') {
							sprintf(sndbuf, (char*) "quit");
							send(c_con, sndbuf, 5, 0);
							game_over = true;
							printf("Quitting game.\n");
							break;
						} else {
							printf(">");
							continue;
						}
					}
					
					if (str_equ(buf, (char*) "help")) {
						printf("The game is played on two 10x10 grids, one containing your ships and another containing your opponent's ships. Only your ships are visible to you.\n");
						printf("Each grid square is two characters wide and one character tall in order to make them square shaped.\n");
						printf("Your grid is on the left and your opponent's is on the right. Your ships are indicated by grid squares containing numbers (the numbers themselves identify the ship).\n");
						printf("For each turn, you can supply a letter-number combination such as \"A4\" or \"G6\" which will specify a grid square of your oponent's to fire at. If that square contained a ship, it will be replaced by an \"XX\" to indicate the hit, or an \"OO\" if the square was empty.\n");
						printf("You and your oponent alternate turns, their hits will appear on your grid in the same way that yours appear on theirs.\n");
						printf("The goal is to destroy all of your oponent's ships before they destroy yours.\n");
						printf("You may run \"quit\" during your turn to end the game early.\n>");
						continue;
					}
					
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
