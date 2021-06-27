#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

// useful constant for the game logic
// statuses returned by a game state
#define FAILURE 0xFFFFFFFF
#define SUCCESS 0
#define RUNNING 2
// console window size
#define ROWS 35
#define COLS 120
// max length of buffers and expressions
#define BUFLEN 1000
#define STRING_SIZE 100
// max number of mistakes in a match
#define MISTAKES 7

// enum that maps the main menu buttons
typedef enum button
{
	NEW = 13,
	RESUME = 14,
	QUIT = 15
} button;

// current game data
typedef struct data
{
	// string that represent the word | expression
	// that need to be guessed
	char *string;
	// hidden string that is printed to the player
	char *hidden;
	// array of characters that were tried and represent
	// mistakes
	char *mistakes;
	// score
	int score;
	// stage of the drawn hangman
	int stage;
	// index of the expression in the database
	int index;
} data;

// database of words | expressions for the game
typedef struct vector_string
{
	// all the words } expresiions
	char **expressions;
	// all their size
	int *sizes;
	// number of words in the database (used when database is loaded)
	int count;
	// number of words in the database
	int exprs;
	// dimensions of words (used when database is loaded)
	int dim;
	int chars;
} vector_string;

// global varialbe used for storing data about the game
// can be the game resumed or not
bool resume;
// current game data
data *game;
// word | expression database
vector_string *puzzle;

// declaration of the functions used by the game
// for more details go to the definition of each

// declare used colors (ncurses)
void declare_colors();

// menu switching functions
void menu(WINDOW *win);
void run(WINDOW *win);
void start();
void instructions(WINDOW *win);

// menu printing functions
void print_new_game(WINDOW *win);
void print_hangman(WINDOW *win, int stage);
void message_win(WINDOW *win);
void message_lose(WINDOW *win);

// util functions
void alocator(vector_string *v);
void expression_store(FILE *source, vector_string *expr);
int  clean_for_print(char *s, int size);
void create_hidden_string(char *string, char *hidden);
int find_character(char *string, char *hidden, char *mistakes, int ch);


// the main function of the game
int main(int argc, char const *argv[])
{
	srand(time(NULL));

	// create game & puzzle data
	game = (data *) malloc(sizeof(data));
	puzzle = (vector_string *) malloc(sizeof(vector_string));

	puzzle->dim = 20;
	puzzle->chars = 0;
	puzzle->count = 0;
	puzzle->exprs = 0;
	alocator(puzzle);

	game->index = -1;
	game->score = 0;
	game->stage = 0;
	game->string   = (char *) calloc(STRING_SIZE, sizeof(char));
	game->hidden   = (char *) calloc(STRING_SIZE, sizeof(char));
	game->mistakes = (char *) calloc(MISTAKES, sizeof(char));

 	if (argc <= 1)
 	{
 		fprintf(stderr, "[Error] Not enough arguments\n\n\tUsage:\t$./hangman [<input-file-1> ...]\n\n");
 		return FAILURE;
 	}

	// create the word | expression database
 	for (int index = 1; index < argc; ++index)
 	{
		FILE *in = fopen(argv[index], "r");
		if (in == NULL)
		{
			fprintf(stderr, "[Error] File %s failed to open\n\n", argv[index]);
			return FAILURE;
		}
 		expression_store(in, puzzle);
        fclose(in);
 	}

	// prepare the terminal for the game
	char *terminal = (char *) malloc (30 * sizeof(char));
	sprintf(terminal, "printf '\e[8;%d;%dt'", ROWS, COLS);
 	system(terminal);
	free(terminal);

 	initscr();
	WINDOW* window = newwin(ROWS, COLS, 0, 0);

   	start_color();
   	declare_colors();

	cbreak();
   	noecho();

	keypad(window, TRUE);
	curs_set(FALSE);

	// start game logic
	// enter main menu
	resume = false;
  	menu(window);

	// game exit
  	werase(window);
  	delwin(window);
    endwin();

	// free the data from the heap
    free(game->mistakes);
    free(game->hidden);
    free(game->string);
	for (int index = 0; index < puzzle->count; ++index)
		free(puzzle->expressions[index]);
    free(puzzle->expressions);
    return SUCCESS;
}

// initializing NCURSES colors
void declare_colors()
{
  	init_pair(1, COLOR_YELLOW, COLOR_BLACK);
   	init_pair(2, COLOR_GREEN, COLOR_BLACK);
   	init_pair(3, COLOR_BLUE, COLOR_BLACK);
   	init_pair(4, COLOR_RED, COLOR_BLACK);
	init_pair(5, COLOR_BLACK, COLOR_RED);
}

// main menu logic
void menu(WINDOW *win)
{
	char menu[3][12] = {"New Game", "Resume Game", "Quit"};

	while (true)
	{
		// clean-up console
		werase (win);
		wbkgd (win, COLOR_PAIR(1));

		// print main title
		wattron(win, COLOR_PAIR(1));
		box(win, 0, 0);
		mvwprintw(win,  3, 3, "Adrian Chiornita FIRST game (don't judge :D):");
		mvwprintw(win,  6, (COLS - 62) / 2, "##   ##    ###    ##   ##   #####   ##   ##    ###    ##   ##");
		mvwprintw(win,  7, (COLS - 62) / 2, "##   ##   ## ##   ###  ##  ##    #  ### ###   ## ##   ###  ##");
		mvwprintw(win,  8, (COLS - 62) / 2, "#######  #######  ## # ##  ##       ## # ##  #######  ## # ##");
		mvwprintw(win,  9, (COLS - 62) / 2, "##   ##  ##   ##  ##  ###  ##  ###  ##   ##  ##   ##  ##  ###");
		mvwprintw(win, 10, (COLS - 62) / 2, "##   ##  ##   ##  ##   ##   #####   ##   ##  ##   ##  ##   ##");
		mvwprintw(win, 18, (COLS - 62) / 2, "                   PRESS I FOR INSTRUCTIONS.                 ");

		// print menu buttons
		mvwprintw(win, QUIT, (COLS - strlen(menu[2])) / 2, "%s", menu[2]);
		wattroff(win, COLOR_PAIR(1));

		if (!resume)
			wattron(win, COLOR_PAIR(5));
		mvwprintw(win, RESUME, (COLS - strlen(menu[1])) / 2, "%s", menu[1]);
		if (!resume)
			wattroff(win, COLOR_PAIR(5));

		wattron (win, COLOR_PAIR(3));
		mvwprintw (win, NEW, (COLS - strlen(menu[0])) / 2, "%s", menu[0]);
		wattroff (win, COLOR_PAIR(3));

		// menu button logic
		int key = NEW;
		while (true)
		{
			// refresh
			wrefresh(win);
			// get input
			int ch = wgetch(win);

			// enter instructions menu
			if (toupper(ch) == 'I')
			{
				instructions(win);
				break;
			}

			// exit game
			if (toupper(ch) == 'Q')
				return;

			// switch menu button
			switch (ch)
			{
			case KEY_UP:
				wattron(win,  COLOR_PAIR(1));
				mvwprintw(win, key, (COLS - strlen(menu[key - NEW])) / 2, "%s", menu[key - NEW]);
				wattroff(win, COLOR_PAIR(1));

				key--;
				if (key < NEW)
					key = QUIT;
				if (!resume && key == RESUME)
					key--;
				break;
			case KEY_DOWN:
				wattron(win,  COLOR_PAIR(1));
				mvwprintw(win, key, (COLS - strlen(menu[key - NEW])) / 2, "%s", menu[key - NEW]);
				wattroff(win, COLOR_PAIR(1));

				key++;
				if (key > QUIT)
					key = NEW;
				if (!resume && key == RESUME)
					key++;
				break;
			default:
				break;
			}

			// if button pressed
			if (ch == 10)
			{
				// enter new game
				if (key == NEW)
				{
					game->score = 0;
					start();
					run(win);
				}
				// resume last game
				if (key == RESUME)
					run(win);

				// exit game
				if(key == QUIT)
					return;
				break;
			}

			// change collor of the current button
			wattron(win,  COLOR_PAIR(3));
			mvwprintw(win, key, (COLS - strlen(menu[key - NEW])) / 2, "%s", menu[key - NEW]);
			wattroff(win, COLOR_PAIR(3));
		}
	}
}

// start a new game logic
void start()
{
	// if a new game is started remove a word
	if (game->index >= 0)
	{
		puzzle->chars -= puzzle->sizes[game->index];
		puzzle->sizes[game->index] = 0;
		puzzle->exprs--;
	}

	// get a random word | expression from the database
	int random_number = rand() % puzzle->count;
	while (!puzzle->sizes[random_number])
		random_number = rand() % puzzle->count;

	strcpy(game->string, puzzle->expressions[random_number]);

	// create the new hidden string
	create_hidden_string(game->string, game->hidden);
	game->mistakes = (char *) calloc(MISTAKES, sizeof(char));

	game->stage = 0;
	game->index = random_number;
}

// run a game match
void run (WINDOW *win)
{
	// clean console and print the game table
	werase(win);
	wbkgd(win, COLOR_PAIR (1));
	print_new_game(win);

	time_t rawtime;
  	struct tm * timeinfo;

	int len = strlen(game->string);
	bool victory = false;
	while (true)
	{
		// refresh statistics
		time(&rawtime);
 		timeinfo = localtime (&rawtime);

		wrefresh(win);

		// print statistics
		wattron(win,  COLOR_PAIR (1));
		box(win, 0 , 0);
		mvwprintw(win, 1, 1, "Score: %i", game->score);
		mvwprintw(win, 1, COLS - strlen(asctime (timeinfo)) - 5, "%s", asctime (timeinfo));
		mvwprintw(win, 1, 15, " Expressions: %i", puzzle->exprs);
		mvwprintw(win, 1, 41, "Characters: %i", puzzle->chars);
		mvwprintw(win, 8, COLS - strlen (game->hidden) - 5, "%s", game->hidden);
		mvwprintw(win, 4, COLS - 23, "%s", game->mistakes);
		wattron(win,  COLOR_PAIR (1));

		//print hangman stage
		print_hangman(win, game->stage);

		// print message if lose
		if (game->stage == 6)
		{
			message_lose(win);
			break;
		}
		// print message if win
		if(victory)
		{
			message_win(win);
			break;
		}

		// get a character
		int ch = wgetch(win);

		// if backspace return to main menu
		if (ch == KEY_BACKSPACE)
		{
			resume = true;
			werase(win);
			break;
		}

		// if is another printable character
		// then do hangman logic
		if (isprint(ch))
		{
			// find if character is in the hidden word
			int searcher = find_character(game->string, game->hidden, game->mistakes, ch);

			// character found
			if (searcher == 1)
			{
				game->score += ch * searcher;

				victory = true;
				for (int index = 0; index < len; ++index)
					if (game->hidden[index] != toupper (game->string[index]))
						victory = false;
			}
			// character not found
			else if(searcher == -1)
			{
				game->stage++;
				game->score -= ch * searcher;

				int len = strlen(game->mistakes);
				game->mistakes[len] = toupper(ch);
				game->mistakes[len + 1] = ',';
				game->mistakes[len + 2] = ' ';
				game->mistakes[len + 3] = '\0';
			}
		}
	}
}

// print instruction menu
void instructions(WINDOW *win)
{
	werase(win);
	wbkgd(win, COLOR_PAIR(1));

	wattron(win, COLOR_PAIR(3));
	box(win, 0, 0);
	mvwprintw(win, 1, 1, "INSTRUCTIONS:");
	mvwprintw(win, 3, 2, "   _________________________________");
	mvwprintw(win, 4, 2, "        []  //                 |");
	mvwprintw(win, 5, 2, "        [] //                  |");
	mvwprintw(win, 6, 2, "        []//                   |");
	mvwprintw(win, 7, 2, "        []/                    |");
	mvwprintw(win, 8, 2, "        []");
	mvwprintw(win, 9, 2, "        []");
	mvwprintw(win, 10, 2, "        []");
	mvwprintw(win, 11, 2, "        []");
	mvwprintw(win, 12, 2, "        []");
	mvwprintw(win, 13, 2, "        []");
	mvwprintw(win, 14, 2, "        []");
	mvwprintw(win, 15, 2, "        []");
	mvwprintw(win, 16, 2, "        []");
	mvwprintw(win, 17, 2, "        []\\");
	mvwprintw(win, 18, 2, "       /[]\\\\");
	mvwprintw(win, 19, 2, "______//[]_\\\\_________");
	mvwprintw(win, 20, 1, "|______________________|");
	mvwprintw(win, 22, 2, "PRESS BACKSPACE TO RETURN");
	mvwprintw(win, 3, 40, "Hangman is a paper and pencil guessing game for two or more players.");
	mvwprintw(win, 4, 40, "One player thinks of a word, phrase or sentence and the other");
	mvwprintw(win, 5, 40, "tries to guess it by suggesting letters, within a certain number of guesses.");
	mvwprintw(win, 6, 40, "You are a programmer, so you must play with your best friend, a computer.");
	wattroff(win, COLOR_PAIR(3));

	print_hangman(win, 6);

	int ch = 0;
	while (ch != KEY_BACKSPACE)
		ch = wgetch(win);
}

// print message lose menu
void message_lose(WINDOW *win)
{
	werase(win);
	wbkgd(win, COLOR_PAIR(1));

	wattron(win, COLOR_PAIR(4));
	box(win, 0, 0);
	mvwprintw(win, 10, (COLS - 72) / 2, " #####     ###    ##   ##  #####     #####   ##   ##  ######  ######  ##");
	mvwprintw(win, 11, (COLS - 72) / 2, "##    #   ## ##   ### ###  ##       ##   ##  ##   ##  ##      ##  ##  ##");
	mvwprintw(win, 12, (COLS - 72) / 2, "##       #######  ## # ##  ####     ##   ##  ##   ##  ####    ######  ##");
	mvwprintw(win, 13, (COLS - 72) / 2, "##  ###  ##   ##  ##   ##  ##       ##   ##   ## ##   ##      ## ##     ");
	mvwprintw(win, 14, (COLS - 72) / 2, " #####   ##   ##  ##   ##  ######    #####     ###    ######  ##  ##  ##");

	mvwprintw(win, 17, (COLS - 73) / 2, "PRESS Q TO RETURN TO MENU.                   PRESS N TO START A NEW GAME.");
	wattroff(win, COLOR_PAIR(4));

	while (true)
	{
		// switch to a new menu
		int ch = wgetch(win);
		if (toupper(ch) == 'Q')
		{
			resume = false;
			werase(win);
			break;
		}
		if (toupper(ch) == 'N')
		{
			werase(win);
			game->score = 0;
			start();
			run(win);
			break;
		}
	}
}

// print message win menu
void message_win(WINDOW *win)
{
	werase(win);
	wbkgd(win, COLOR_PAIR(1));

	wattron(win, COLOR_PAIR(2));
	box(win, 0, 0);
	mvwprintw(win, 10, (COLS - 60) / 2, "##   ##   #####   ##   ##      ##  ##  ##  ##  ##   ##  ##");
	mvwprintw(win, 11, (COLS - 60) / 2, " ## ##   ##   ##  ##   ##      ##  ##  ##  ##  ###  ##  ##");
	mvwprintw(win, 12, (COLS - 60) / 2, "  ###    ##   ##  ##   ##      ##  ##  ##  ##  ## # ##  ##");
	mvwprintw(win, 13, (COLS - 60) / 2, "  ##     ##   ##  ##   ##       ## ## ##   ##  ##  ###    ");
	mvwprintw(win, 14, (COLS - 60) / 2, "  ##      #####   #######        ######    ##  ##   ##  ##");
	mvwprintw(win, 17, (COLS - 60) / 2, "PRESS Q TO RETURN TO MENU.   PRESS N TO CONTINUE THE GAME.");
	wattroff(win, COLOR_PAIR(2));

	while (true)
	{
		// switch to a new menu
		int ch = wgetch(win);
		if (toupper(ch) == 'Q')
		{
			resume = false;
			werase(win);
			break;
		}
		if (toupper(ch) == 'N')
		{
			werase(win);
			start();
			run(win);
			break;
		}
	}
}

// print a new hangman game
void print_new_game(WINDOW *win)
{
	wattron(win, COLOR_PAIR(1));
	for (int index = 1; index < COLS; ++index)
			mvwprintw(win, 2, index, "_");
	mvwprintw(win,  3, 2, "   _________________________________");
	mvwprintw(win,  4, 2, "        []  //                 |");
	mvwprintw(win,  5, 2, "        [] //                  |");
	mvwprintw(win,  6, 2, "        []//                   |");
	mvwprintw(win,  7, 2, "        []/                    |");
	mvwprintw(win,  8, 2, "        []");
	mvwprintw(win,  9, 2, "        []");
	mvwprintw(win, 10, 2, "        []");
	mvwprintw(win, 11, 2, "        []");
	mvwprintw(win, 12, 2, "        []");
	mvwprintw(win, 13, 2, "        []");
	mvwprintw(win, 14, 2, "        []");
	mvwprintw(win, 15, 2, "        []");
	mvwprintw(win, 16, 2, "        []");
	mvwprintw(win, 17, 2, "        []\\");
	mvwprintw(win, 18, 2, "       /[]\\\\");
	mvwprintw(win, 19, 2, "______//[]_\\\\_________");
	mvwprintw(win, 20, 1, "|______________________|");
	mvwprintw(win, 22, 1, "______________________________________________");
	mvwprintw(win, 23, 1, "                                              |");
	mvwprintw(win, 24, 2, "Press BACKSPACE to return to the Main Menu.  |");
	mvwprintw(win, 25, 2, "Press a character to see if you guessed      |");
	mvwprintw(win, 26, 2, "a letter in the expression.                  |");
	mvwprintw(win, 27, 1, "                                              |");
	mvwprintw(win, 28, 1, "______________________________________________|");
	wattroff(win, COLOR_PAIR(1));

	wattron (win, COLOR_PAIR(4));
	mvwprintw (win, 3, COLS - 25, "| Mistakes Table:|");
	mvwprintw (win, 4,	COLS - 25, "|                |");
	mvwprintw (win, 5, COLS - 25, "|________________|");
	wattroff (win, COLOR_PAIR(4));
}

// print hangman stage
void print_hangman (WINDOW *win, int stage)
{
	wattron(win, COLOR_PAIR(4));
	switch(stage)
	{
	case 6:
		mvwprintw(win, 14, 34, "\\\\");
		mvwprintw(win, 15, 34, " \\\\");
		mvwprintw(win, 16, 34, "  \\\\");
	case 5:
		mvwprintw(win, 14, 29, "  // ");
		mvwprintw(win, 15, 29, " //");
		mvwprintw(win, 16, 29, "//");
	case 4:
		mvwprintw(win, 11, 35, "\\\\  ");
		mvwprintw(win, 12, 35, " \\\\ ");
		mvwprintw(win, 13, 35, "  \\\\");
	case 3:
		mvwprintw(win, 11, 28, "  //");
		mvwprintw(win, 12, 28, " //");
		mvwprintw(win, 13, 28, "//");
	case 2:
		mvwprintw(win, 11, 32, "[ ]");
		mvwprintw(win, 12, 32, "[ ]");
		mvwprintw(win, 13, 32, "[_]");
	case 1:
		mvwprintw(win,  7, 29, " ___|___");
		mvwprintw(win,  8, 29, "// X 0 \\\\");
		mvwprintw(win,  9, 29, " ]  ^  [");
		mvwprintw(win, 10, 29, " \\__~__/");;
		break;
	}
	wattroff(win, COLOR_PAIR (4));
}

// this is called when we initialize a vector_string type
// or when we want to allocate more memory into it
void alocator(vector_string *vector)
{
	// if it is first initialization
	if (!vector->expressions || !vector->sizes)
	{
		vector->expressions = (char**) malloc(sizeof(char *) * vector->dim);
		vector->sizes = (int *) malloc(sizeof(int) * vector->dim);
	}
	// if the vector_string needs more memory
	else
	{
		char **eaux = (char **) realloc(vector->expressions, sizeof(char *) * 2 * (vector->dim));
		int *saux = (int *) realloc(vector->sizes, sizeof(int) * 2 * (vector->dim));
		if (eaux && saux)
		{
			vector->expressions = eaux;
			vector->sizes = saux;
			vector->dim *= 2;
		}
		// Heap Memory is full
		else
		{
			fprintf(stderr, "[Error] Not enough memory.\n");
			exit(FAILURE);
		}
	}
}

// store a line from the input file (word | expression) in
// vec -- the puzzle database
void expression_store(FILE *source, vector_string *vec)
{
	// allocate a auxiliary buffer for reading
	char *buffer = (char *) calloc(STRING_SIZE, sizeof (char));
	int read = 0;

	//read each line
	while(fgets(buffer, STRING_SIZE - 1, source))
	{
		read = strlen(buffer);
		if (!read) continue;

		// if the database is full call the allocator
		if (vec->count == vec->dim) alocator(vec);

		// clean the buffer for printing
		read = clean_for_print(buffer, read);

		// append the result into the puzzle database
		vec->count++;
		vec->sizes[vec->count - 1] = read;
		vec->chars += read;
		vec->expressions[vec->count - 1] = (char *) malloc(sizeof(char) * read);
		strcpy(vec->expressions[vec->count - 1], buffer);

		// error if the file couldn't be parsed
		if (ferror(source))
		{
			fprintf(stderr, "[Error] Could not parse the file\n");
			exit(FAILURE);
		}
	}
	// clean-up
	vec->exprs = vec->count;
	free(buffer);
}

// cleaning the passed string in order to be printed on console
int clean_for_print(char *string, int size)
{
	for (int index = 0; index < size; ++index)
	{
		// if we have multiple spaces in the string or charaters that are
		// not printable, we remove them in order for the game to be simple
		if (!isprint(string[index]) ||
			(index > 1 && isspace(string[index - 1]) && isspace(string[index])))
		{
			memmove(string + index, string + index + 1, size - index);
			--index;
			--size;
		}
		// transform all alphabetical characters into lowercase ones
		if (isalpha(string[index]))
			string[index]= tolower(string[index]);
	}
	return size;
}

// create a hidden string (string with _ where a letter should be guessed)
// from the word | expression given
void create_hidden_string(char *string, char *hidden)
{
	// init hidden
	strcpy (hidden, "");

	// make a copy of the unhidden string
	char *aux = (char*) malloc(sizeof(char) * STRING_SIZE);
	strcpy(aux, string);

	char sep[] = " ";
	char *pt = strtok(aux, sep);

	// iterate over each word in the hidden expression
	// transform words like banana -> b _ _ _ _ a
	while (pt)
	{
		int len = strlen (pt);
		if (len == 1) pt[0] = '_';
		else
			if (len == 2)
			{
				pt[0] = toupper(pt[0]);
				pt[1] = '_';
			}else
			{
				pt[0] = toupper(pt[0]);
				pt[len - 1] = toupper(pt[len -1]);

				for (int index = 1; index < len -1; ++index)
					pt[index] = '_';
			}

		strcat(hidden, pt);
		strcat(hidden, " ");

		pt = strtok(NULL, sep);
	}

	// iterate over each word in the hidden expression
	// transform words like b _ _ _ _ a -> b a _ a _ a
	int len = strlen (hidden);
	for (int index = 0; index < len; ++index)
		if (hidden[index] != '_' && hidden[index] != ' ')
		{
			for (int j = 0; j < len; ++j)
				if (tolower(hidden[index]) == string[j])
					hidden[j] = hidden[index];

		}
	hidden[len] = '\0';
}

// find if a character is in the hidden string and is not discovered yet
int find_character(char *string, char *hidden, char *mistakes, int ch)
{
	ch = tolower(ch);
	if (strchr(string, ch))
	{
		if (!strchr(hidden, toupper(ch)))
		{
			int len = strlen(string);
			for (int index = 0; index < len; ++index)
				if (string[index] == ch)
					hidden[index] = toupper(ch);
			return 1;
		}
		else
			return 0;
	}
	else
	{
		return (!strchr(mistakes, toupper(ch))) ? -1 : 0;
	}
}