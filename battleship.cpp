#include <stdlib.h>

namespace battleship {
	game::game(int width, int height, int ships_n) : width(width), height(height), ships_n(ships_n) {
		ships_r = (ship*) malloc(sizeof(ship) * ships_n);
		ships_b = (ship*) malloc(sizeof(ship) * ships_n);
		grid = (unsigned char*) malloc(width*height);
		for (int i = 0; i < width*height; i++) {
			grid[i] = 0;
		}
	}
	
	game::game() : width(10), height(10), ships_n(5) {
		ships_r = (ship*) malloc(sizeof(ship) * 5);
		ships_b = (ship*) malloc(sizeof(ship) * 5);
		grid = (unsigned char*) malloc(100);
		for (int i = 0; i < 100; i++) {
			grid[i] = 0;
		}
		ships_r[0].size = 5; ships_b[0].size = 5;
		ships_r[1].size = 4; ships_b[1].size = 4;
		ships_r[2].size = 3; ships_b[2].size = 3;
		ships_r[3].size = 3; ships_b[3].size = 3;
		ships_r[4].size = 2; ships_b[4].size = 2;
		
		for (int i = 0; i < 5; i++) {
			ships_r[i].is_alive = true;
			ships_b[i].is_alive = true;
		}
	}
	
	int game::is_over_ship(int x, int y, bool team) {
		return is_over_ship(x, y, ships_n, team);
	}
	
	bool game::randomize() {
		int attempts;
		
		// place red ships
		attempts = 0;
		for (int s = 0; s < ships_n; s++) {
			if (attempts > 50) {
				return false;
			}
			
			// Determine orientation
			if (ships_r[s].size <= width) {
				if (ships_r[s].size <= height) {
					ships_r[s].orientation = rand() % 2;
				} else {
					ships_r[s].orientation = HORIZONTAL;
				}
			} else {
				if (ships_r[s].size <= height) {
					ships_r[s].orientation = VERTICAL;
				} else {
					return false;
				}
			}
			
			// Pick random location
			if (ships_r[s].orientation == HORIZONTAL) {
				ships_r[s].y = rand() % height;
				ships_r[s].x = rand() % (width - ships_r[s].size);
			} else {
				ships_r[s].y = rand() % (height - ships_r[s].size);
				ships_r[s].x = rand() % width;
			}
			
			// Check for collision
			for (int i = 0; i < s; i++) {
				if (colliding(ships_r[i], ships_r[s])) {
					s--;
					break;
				}
			}
		}
		
		// place blue ships
		attempts = 0;
		for (int s = 0; s < ships_n; s++) {
			if (attempts > 50) {
				return false;
			}
			
			// Determine orientation
			if (ships_b[s].size <= width) {
				if (ships_b[s].size <= height) {
					ships_b[s].orientation = rand() % 2;
				} else {
					ships_b[s].orientation = HORIZONTAL;
				}
			} else {
				if (ships_b[s].size <= height) {
					ships_b[s].orientation = VERTICAL;
				} else {
					return false;
				}
			}
			
			// Pick random location
			if (ships_b[s].orientation == HORIZONTAL) {
				ships_b[s].y = rand() % height;
				ships_b[s].x = rand() % (width - ships_b[s].size);
			} else {
				ships_b[s].y = rand() % (height - ships_b[s].size);
				ships_b[s].x = rand() % width;
			}
			
			// Check for collision
			for (int i = 0; i < s; i++) {
				if (colliding(ships_b[i], ships_b[s])) {
					s--;
					break;
				}
			}
		}
	}
	
	int game::fire(bool team, int x, int y) {
		int s;
		unsigned char bitf;
		if (team == RED) {
			bitf = 0x04;
		} else {
			bitf = 0x01;
		}
		
		grid[x + y*width] = grid[x + y*width] | bitf;
		
		s = is_over_ship(x, y, !team);
		if (s != -1) {
			grid[x + y*width] = grid[x + y*width] | bitf*2;
			
			ship* S;
			if (team == RED) {
				S = ships_b + s;
			} else {
				S = ships_r + s;
			}
			
			if (S->orientation == HORIZONTAL) {
				for (int i = S->x; i < S->size + S->x; i++) {
					if (!(grid[i + S->y*width] & bitf*2)) {
						return s;
					}
				}
			} else {
				for (int i = S->y; i < S->size + S->y; i++) {
					if (!(grid[S->x + i*width] & bitf*2)) {
						return s;
					}
				}
			}
			
			S->is_alive = false;
		}
		
		return s;
	}
	
	void game::show(bool team) {
		int s;
		char out;
		
		unsigned char bitf0;
		unsigned char bitf1;
		if (team == RED) {
			bitf0 = 0x01;
			bitf1 = 0x04;
		} else {
			bitf0 = 0x04;
			bitf1 = 0x01;
		}
		
		printf("  ");
		for (int x = 0; x < width; x++) {
			printf("%d ", x);
		}
		printf("    ");
		for (int x = 0; x < width; x++) {
			printf("%d ", x);
		}
		printf("\n");
		
		for (int y = 0; y < height; y++) {
			printf("%c ", y + 65);
			for (int x = 0; x < width; x++) {
				// print left character
				s = is_over_ship(x, y, team);
				if (s < 0) {
					out = '~';
				} else {
					out = s + 48; // Converts number to corresponding char.
				}
				
				if (grid[x + y*width] & bitf0) {
					if (grid[x + y*width] & bitf0*2) {
						out = 'X';
					} else {
						out = 'O';
					}
				}
				
				printf("%c%c", out, out);
			}
				
			printf("  %c ", y + 65);
			for (int x = 0; x < width; x++) {
				// print right character
				out = '~';
				
				if (grid[x + y*width] & bitf1) {
					if (grid[x + y*width] & bitf1*2) {
						out = 'X';
					} else {
						out = 'O';
					}
				}
				
				printf("%c%c", out, out);
			}
			printf("\n");
		}
	}
	
	bool game::check_win(bool team) {
		if (team == RED) {
			for (int i = 0; i < ships_n; i++) {
				if (ships_b[i].is_alive) {
					return false;
				}
			}
			
			return true;
		} else {
			for (int i = 0; i < ships_n; i++) {
				if (ships_r[i].is_alive) {
					return false;
				}
			}
			
			return true;
		}
	}
			
	game::~game() {
		free(ships_r);
		free(ships_b);
		free(grid);
	}
	
	int game::is_over_ship(int x, int y, int n, bool team) {
		if (team == RED) {
			for (int s = 0; s < n; s++) {
				if (ships_r[s].orientation == HORIZONTAL && ships_r[s].y == y && ships_r[s].x <= x && ships_r[s].x > x - ships_r[s].size) {
					return s;
				}
				if (ships_r[s].orientation == VERTICAL   && ships_r[s].x == x && ships_r[s].y <= y && ships_r[s].y > y - ships_r[s].size) {
					return s;
				}
			}
			return -1;
			
		} else /* team == BLUE */ {
			for (int s = 0; s < n; s++) {
				if (ships_b[s].orientation == HORIZONTAL && ships_b[s].y == y && ships_b[s].x <= x && ships_b[s].x > x - ships_b[s].size) {
					return s;
				}
				if (ships_b[s].orientation == VERTICAL   && ships_b[s].x == x && ships_b[s].y <= y && ships_b[s].y > y - ships_b[s].size) {
					return s;
				}
			}
			return -1;
		}
	}
	
	bool game::colliding(ship a, ship b) {
		ship t;
		
		if (a.orientation == HORIZONTAL && b.orientation == HORIZONTAL && a.y == b.y) {
			if (a.x >= b.x) {
				if (a.x - b.x < b.size) {
					return true;
				}
			} else {
				if (b.x - a.x < a.size) {
					return true;
				}
			}
		}
		else if (a.orientation == VERTICAL && b.orientation == VERTICAL && a.x == b.x) {
			if (a.y >= b.y) {
				if (a.y - b.y < b.size) {
					return true;
				}
			} else {
				if (b.y - a.y < a.size) {
					return true;
				}
			}
		}
		else if (b.orientation == HORIZONTAL && a.orientation == VERTICAL) {
			t = a; a = b; b = t;
		}
		
		if (a.orientation == HORIZONTAL && b.orientation == VERTICAL) {
			if (a.x <= b.x && a.x > b.x - a.size && b.y <= a.y && b.y > a.y - b.size) {
				return true;
			}
		}
		
		return false;
	}
			
	ship::ship() : x(0), y(0), is_alive(true), orientation(HORIZONTAL), size(2) {}
	
	ship::ship(int x, int y, bool orientation, int size) : x(x), y(y), is_alive(true), orientation(orientation), size(size) {}
}
