#define HORIZONTAL true
#define VERTICAL false

#define RED true
#define BLUE false

namespace battleship {
	class ship;
	class game;
	
	class game {
	public:
		// Size of the board
		int width;
		int height;
		
		// bit 1: Whether this square on red's board has been attacked.
		// bit 2: Whether this square, on red's board, was a hit or a miss (This value is 0 until an attack is attmpted on this grid square).
		// bit 3: The same as bit 1, but for blue's board.
		// bit 4: The same as bit 2, but for blue's board.
		// bits 5-8 are always 0.
		unsigned char* grid;
		
		int ships_n;
		ship* ships_r;
		ship* ships_b;
		
		// Create a custom game.
		game(int width, int height, int ships_n);
		
		// Create a normal game (10x10 grid with 5 ships)
		game();
		
		// Figure out whether the given point on a board is over a ship. Returns the index of the ship underneath, or -1 if no ship is present.
		int is_over_ship(int x, int y, bool team);
		
		// Randomize the location and orientation of the ships within the grid. Returns false if a valid arrangement of the ships is not found.
		// Returning false does not necessarily mean a valid arrangement does not exist.
		bool randomize();
		
		// Fire a shot from this team to the given square on the opposing team's side. Returns whether it hit or not.
		int fire(bool team, int x, int y);
		
		// print out the game board for the given team.
		// A square is displayed with two characters. The left character will be the index of the ship under that square if a ship exists, an X if the enemy hit that square and it contained a ship, an O if the enemy hit that square and it did not contain a ship, or a ~ if it contains water and has not been hit.
		// The right square shows an X if it has been hit and contained an enemy ship, an O if it has been hit and did not contain an enemy ship, or a ~ if it has not been hit.
		void show(bool team);
		
		// returns whether the given team has won or not. Specifically, checks whether the opponent's ships are destroyed.
		bool check_win(bool team);
		
		~game();
		
	private:
		int is_over_ship(int x, int y, int n, bool team);
		
		// Returns whether the ships are in collision
		bool colliding(ship a, ship b);
	};

	class ship {
	public:
		// Position of lowest or left-most point on the ship.
		int x;
		int y;
		
		// Whether the ship has remaining undestroyed squares.
		bool is_alive;
		
		// HORIZONTAL or VERTICAL (true or false)
		bool orientation;
		
		// Length of the ship.
		int size;
		
		// Creates a valid default ship.
		ship();
		
		// Create a custom ship
		ship(int x, int y, bool orientation, int size);
	};
}

#include "battleship.cpp"
