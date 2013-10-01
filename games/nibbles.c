/* $Id: nibbles.c 421 2008-12-14 16:19:27Z kate $ */

/*
 * This is a faithful port of the Microsoft nibbles.bas program to ISO C90,
 * using curses for POSIX systems. Nibbles was provided as a demonstration with
 * QBasic from DOS 5 onwards.
 *
 * The structure of the program is unchanged. Refactoring is kept to a minimum
 * (to aid readability only); you should be able to make a side-by-side
 * comparison against the origional. Functions have been reordered so as to
 * avoid needing prototypes. In particular use has not been made of curses'
 * window and pad facilities, which might be more suited for dialog boxes.
 *
 * Colours are reworked according to the colour pairs mechanism provided by
 * curses, as opposed to the individual foreground and background used by
 * BASIC. The option for a monochrome or colour monitor is handled
 * automatically; this may be set through termcap by way of $TERM.
 *
 * Some details may not be exactly as they were in QBasic (prompting for input,
 * for example), but the gameplay ought to be identical.
 */

/* TODO: remove all X. UTF-8? */
/* TODO: sound? */
/* TODO; window resizing? */
/* TODO: I took the liberty of fixing the bug on level five. */
/* TODO: scores can be negative */
/* TODO: fairness: they both ought to die if no keys are pressed */
/* TODO: check logic when both snakes die */
/* TODO: add 'q' to quit and ' ' to pause */
/* TODO: monochrome colour pairs */

/*
 *
 *                         Q B a s i c   N i b b l e s
 *
 *                   Copyright (C) Microsoft Corporation 1990
 *
 * Nibbles is a game for one or two players.  Navigate your snakes
 * around the game board trying to eat up numbers while avoiding
 * running into walls or other snakes.  The more numbers you eat up,
 * the more points you gain and the longer your snake becomes.
 *
 * To run this game, press Shift+F5.
 *
 * To exit QBasic, press Alt, F, X.
 *
 * To get help on a BASIC keyword, move the cursor to the keyword and press
 * F1 or click the right mouse button.
 *
 */

#include <assert.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>


/*
 * Constants
 */

#define MAXSNAKELENGTH 1000

enum WhatToDo {
	STARTOVER,
	SAMELEVEL,
	NEXTLEVEL
};

/*
 * Colour pair IDs to be passed to Color()
 */
enum colorpair {
	COLOR_SNAKE1 = 1,
	COLOR_SNAKE2,
	COLOR_BOTH,
	COLOR_WALLS,
	COLOR_WALL1,
	COLOR_WALL2,
	COLOR_DIALOG,
	COLOR_STATUS,
	COLOR_NUMBER,
	COLOR_EMPTY
};


/*
 * User-defined TYPEs
 */

struct snakeBody {
	unsigned int row;
	unsigned int col;
};

/*
 * This type defines the player's snake
 */
struct snakeType {
	unsigned int head;
	unsigned int length;
	unsigned int row;
	unsigned int col;
	unsigned int direction;	/* TODO: enum? */
	unsigned int lives;
	unsigned int score;
	enum colorpair scolor;
	int alive;	/* TODO: bool? */
};

/*
 * This type is used to represent the playing screen in memory
 * It is used to simulate graphics in text mode, and has some interesting,
 * and slightly advanced methods to increasing the speed of operation.
 * Instead of the normal 80x25 text graphics using chr$(219) "X", we will be
 * using chr$(220)"X" and chr$(223) "X" and chr$(219) "X" to mimic an 80x50
 * pixel screen.
 * Check out sub-programs SET and POINTISTHERE to see how this is implemented
 * feel free to copy these (as well as arenaType and the DIM ARENA stmt and the
 * initialization code in the DrawScreen subprogram) and use them in your own
 * programs
 */
struct arenaType {
	unsigned int realRow;	/* Maps the 80x50 point into the real 80x25 */

	/* TODO: enum? */
	unsigned int acolor;	/* Stores the current color of the point */

	/* TODO: enum? */
	int sister;				/* Each char has 2 points in it.  .SISTER is -1 if sister point is above, +1 if below */
};

/*
 * Global Variables
 */
struct arenaType arena[50][80];
unsigned int curLevel;

/*
 * Equivalent to the PLAY keyword.
 */
static void Play(const char *s) {
	/* TODO: output to PCM? */
}

/*
 * CHR$()
 */
static unsigned char CHR(unsigned char c) {
	/* TODO: map to something? */
	return c;
	switch (c) {
	case 219: return '#';
	case 220: return '_';
	case 221: return '[';
	case 222: return ']';
	case 223: return '"';
	default: return '?';
	}
}

/*
 * This is roughly equivalent to the COLOR keyword, except built around curses
 * colour pairs (each of which is given an enumeration name for readability)
 * rather than specifying the foreground and background independantly.
 */
static void Color(enum colorpair pair) {
	attron(COLOR_PAIR(pair) | A_BOLD);

	if (pair == COLOR_EMPTY) {
		attroff(A_BOLD);
	}

	/* special cases: no bold */
	if (has_colors() == FALSE && (pair == COLOR_SNAKE2 || pair == COLOR_WALLS || pair == COLOR_WALL2)) {
		attroff(A_BOLD);
	}
}

static void delay(unsigned int curSpeed) {
	usleep(1000 * curSpeed);
}

static int xgetnstr(char *str, size_t limit) {
	size_t z;
	int c;

	z = 0;
	while ((c = getch()) != '\n') {
		if (c == ERR) {
			return ERR;
		}

		if (c == KEY_BACKSPACE && z == 0) {
			continue;
		}

		if (c == KEY_BACKSPACE) {
			z--;
			move(getcury(stdscr), getcurx(stdscr) - 1);
			clrtoeol();
			continue;
		}

		if (z == limit - 1) {
			beep();
			continue;
		}

		str[z] = c;
		addch(c);
		z++;
	}

	str[z] = '\0';
	return !ERR;
}

/*
 * This is a rough analogue of the INPUT keyword.
 */
static void Input(const char *prompt, const char *fmt, ...) {
	char buf[10];
	int n;
	va_list ap;

	assert(prompt != NULL);
	assert(fmt != NULL);

	addstr(prompt);
	addstr("? ");
	clrtoeol();

	do {
		if (xgetnstr(buf, sizeof buf) == ERR) {
			continue;
		}

		va_start(ap, fmt);
		n = vsscanf(buf, fmt, ap);
		va_end(ap);
	} while (n < 0);
}

static void ClearKeyLocks(void) {
	/* TODO Turn off CapLock, NumLock and ScrollLock */
}

static void RestoreKeyLocks(void) {
	/* TODO Restore CapLock, NumLock and ScrollLock states */
}

static void SetColors(void) {
	unsigned int a;

	struct {
		enum colorpair pair;
		unsigned int fore;
		unsigned int back;
	} pairs[2][10] = {
		/* colour */
		{
			{ COLOR_SNAKE1, COLOR_YELLOW,  COLOR_BLUE    },
			{ COLOR_SNAKE2, COLOR_MAGENTA, COLOR_BLUE    },
			{ COLOR_BOTH,   COLOR_YELLOW,  COLOR_MAGENTA },
			{ COLOR_WALLS,  COLOR_RED,     COLOR_BLUE    },
			{ COLOR_WALL1,  COLOR_YELLOW,  COLOR_RED     },
			{ COLOR_WALL2,  COLOR_MAGENTA, COLOR_RED     },
			{ COLOR_DIALOG, COLOR_WHITE,   COLOR_BLUE    },
			{ COLOR_STATUS, COLOR_WHITE,   COLOR_BLUE    },
			{ COLOR_NUMBER, COLOR_WHITE,   COLOR_BLUE    },
			{ COLOR_EMPTY,  COLOR_BLUE,    COLOR_BLUE    }
		},

		/* mono */
		{
			{ COLOR_SNAKE1, COLOR_WHITE, COLOR_BLACK },
			{ COLOR_SNAKE2, COLOR_WHITE, COLOR_BLACK },
			{ COLOR_BOTH,   COLOR_WHITE, COLOR_WHITE },
			{ COLOR_WALLS,  COLOR_WHITE, COLOR_BLACK },
			{ COLOR_WALL1,  COLOR_WHITE, COLOR_WHITE },
			{ COLOR_WALL2,  COLOR_WHITE, COLOR_WHITE },
			{ COLOR_DIALOG, COLOR_WHITE, COLOR_BLACK },
			{ COLOR_STATUS, COLOR_WHITE, COLOR_BLACK },
			{ COLOR_NUMBER, COLOR_WHITE, COLOR_BLACK },
			{ COLOR_EMPTY,  COLOR_BLACK, COLOR_BLACK }
		}
	};

	assert(COLOR_PAIRS > 6);
/* TODO: ?
	assert(COLORS >= 15);
*/

	for (a = 0; a < sizeof *pairs / sizeof **pairs; a++) {
		int m = has_colors() == TRUE ? 0 : 1;

		init_pair(pairs[m][a].pair, pairs[m][a].fore, pairs[m][a].back);
	}
}

/*
 * Centers text on given row
 */
static void Center(unsigned int row, const char *text) {
	assert(text != NULL);

	move(row - 1, 40 - strlen(text) / 2 - 1);
	addstr(text);
}

/*
 * Draws playing field
 */
static void DrawScreen(void) {
	unsigned int row, col;

	/* Print title */
	Center(1,  "Nibbles!");
	refresh();
	
	/* Initialize arena array */
	for (row = 1; row <= 50; row++) {
		for (col = 1; col <= 80; col++) {
			arena[row - 1][col - 1].realRow = (row + 1) / 2;
			arena[row - 1][col - 1].sister  = (row % 2) * 2 - 1;
		}
	}
}

/*
 * Gets player inputs
 */
static void GetInputs(unsigned int *NumPlayers, unsigned int *speed, char *diff) {
	assert(NumPlayers != NULL);
	assert(speed != NULL);
	assert(diff != NULL);

	attron(COLOR_PAIR(0));
	clear();

	do {
		move(5, 20);
		Input("How many players (1 or 2)", "%u", NumPlayers);
	} while (!(*NumPlayers == 1 || *NumPlayers == 2));

	mvaddstr(9,  22, "1   = Novice");
	mvaddstr(10, 22, "90  = Expert");
	mvaddstr(11, 22, "100 = Twiddle Fingers");
	do {
		move(8, 21);
		Input("Skill level (1 to 100)", "%u", speed);
	} while (!(*speed >= 1 && *speed <= 100));
  
	*speed = (100 - *speed) * 2 + 1;

	do {
		move(15, 15);
		Input("Increase game speed during play (Y or N)", "%c", diff);
		*diff = toupper((unsigned char) *diff);
	} while (!(*diff == 'Y' || *diff == 'N'));
}

/*
 * Sets row and column on playing field to given color to facilitate moving
 * of snakes around the field.
 */
static void Set(unsigned int row, unsigned int col, enum colorpair acolor) {
	unsigned int realRow;
	unsigned int sisterRow;
	unsigned int sisterColor;
	int topFlag;

	assert(row >= 0 && row <= 50);
	assert(row >= 0 && row <= 80);

	if (row == 0) {
		return;
	}

	arena[row - 1][col - 1].acolor = acolor;            /* assign color to arena */
	realRow = arena[row - 1][col - 1].realRow;          /* Get real row of pixel */
	topFlag = arena[row - 1][col - 1].sister + 1.0 / 2;   /* Deduce whether pixel is on topX, or bottomX */
	sisterRow = row + arena[row - 1][col - 1].sister;   /* Get arena row of sister */
	sisterColor = arena[sisterRow - 1][col - 1].acolor; /* Determine sister's color */

	move(realRow - 1, col - 1);

	/* If both points are same print a full-height block */
	if (acolor == sisterColor) {
		Color(acolor);
		addch(CHR(219));
		return;
	}

	assert(acolor != sisterColor);

	/*
	 * What follows is a little cumbersome; the program is designed in terms of
	 * separate foreground and background colours. However, curses thinks in
	 * assigned colour pairs, and so we need to map the separate colours onto
	 * an appropiate pair, taking care to draw the half-block in the correct
	 * half of the character cell.
	 *
	 * This might better be expressed as a lookup table.
	 */

	/* Colours which can never be adjacent to something */
	if (acolor == COLOR_EMPTY) {
		Color(sisterColor);
		addch(topFlag ? CHR(220) : CHR(223));
		return;
	} else if (sisterColor == COLOR_EMPTY) {
		Color(acolor);
		addch(topFlag ? CHR(223) : CHR(220));
		return;
	}

	/* Walls next to snakes */
	if (sisterColor == COLOR_WALLS) {
		Color(acolor == COLOR_SNAKE1 ? COLOR_WALL1 : COLOR_WALL2);
		addch(topFlag ? CHR(223) : CHR(220));
		return;
	}

	/*
	 * Snakes next to snakes. We have just one colour, COLOR_BOTH, since using
	 * yellow as a background colour usually comes out brown. So, in both cases
	 * the yellow is kept bold.
	 */
	Color(COLOR_BOTH);
	if (acolor == COLOR_SNAKE1) {
		addch(topFlag ? CHR(223) : CHR(220));
	} else {
		addch(topFlag ? CHR(220) : CHR(223));
	}
}

/*
 * Initializes playing field colors
 */
static void InitColors(void) {
	unsigned int row, col;

	for (row = 1; row <= 50; row++) {
		for (col = 1; col <= 80; col++) {
			arena[row - 1][col - 1].acolor = COLOR_EMPTY;
		}
	}

	/* initialize screen */
	Color(COLOR_EMPTY);
	clear();
	for (row = 1; row <= 25; row++) {
		mvhline(row - 1, 0, CHR(219), 80);
	}

	/* Set (turn on) pixels for screen border */
	for (col = 1; col <= 80; col++) {
		Set(3,  col, COLOR_WALLS);
		Set(50, col, COLOR_WALLS);
	}

	for (row = 4; row <= 49; row++) {
		Set(row, 1,  COLOR_WALLS);
		Set(row, 80, COLOR_WALLS);
	}
}

/*
 * Creates flashing border for intro screen
 */
static void SparklePause(void) {
	const char *s = "*    *    *    *    *    *    *    *    *    *    *    *    *    *    *    *    *    ";
	init_pair(1, COLOR_RED, COLOR_BLACK);
	attron(COLOR_PAIR(1));

	timeout(0);

	/* clear input buffer */
	while (getch() != ERR) {
		/* nothing */;
	}

	while (getch() == ERR) {
		unsigned int a, b;

		/* Print horizontal sparkles */
		for (a = 1; a <= 5; a++) {
			mvaddnstr(1, 1,  s + a,     80);
			mvaddnstr(22, 1, s + 6 - a, 80);

			/* Print vertical sparkles */
			for (b = 2; b <= 21; b++) {
				unsigned int c;

				c = (a + b) % 5;
				if (c == 1) {
					mvaddstr(b, 80, "*");
					mvaddstr(23 - b, 1, "*");
				} else {
					mvaddstr(b, 80, " ");
					mvaddstr(23 - b, 1, " ");
				}
			}

			delay(100);
			refresh();
		}
	}
	timeout(-1);

	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	attron(COLOR_PAIR(1));
}

/*
 *  Displays game introduction
 */
static void Intro(void) {
	clear();

	attron(COLOR_PAIR(0));
	attron(A_BOLD);
	Center(4, "C u r s e s   N i b b l e s");
	attroff(A_BOLD);

	Center(6, "Copyright (C) Microsoft Corporation 1990");
	Center(8, "Nibbles is a game for one or two players.  Navigate your snakes");
	Center(9, "around the game board trying to eat up numbers while avoiding");
	Center(10, "running into walls or other snakes.  The more numbers you eat up,");
	Center(11, "the more points you gain and the longer your snake becomes.");
	Center(13, " Game Controls ");
	Center(15, "  General             Player 1               Player 2    ");
	Center(16, "                        (Up)                   (Up)      ");
	Center(17, "P - Pause                ^                      W       ");
	Center(18, "                     (Left) <   > (Right)   (Left) A   D (Right)  ");	/* TODO: real arrows */
	Center(19, "                         v                      S       ");
	Center(20, "                       (Down)                 (Down)     ");
	Center(24, "Press any key to continue");

	Play("MBT160O1L8CDEDCDL4ECC");
	SparklePause();
}

/*
 * Erases snake to facilitate moving through playing field
 */
static void EraseSnake(const struct snakeType *snake, struct snakeBody snakeBod[]) {
	unsigned int c;

	assert(snake != NULL);
	assert(snakeBod != NULL);

	for (c = 0; c <= 9; c++) {
		int b;

		for (b = snake->length - c - 1; b >= 0; b -= 10) {
			unsigned int tail;

			tail = (snake->head + MAXSNAKELENGTH - b) % MAXSNAKELENGTH;
			Set(snakeBod[tail - 1].row, snakeBod[tail - 1].col, COLOR_EMPTY);
		}

		refresh();
		delay(150);
	}
}

/*
 * Sets game level
 */
static void Level(enum WhatToDo WhatToDo, struct snakeType sammy[2]) {
	unsigned int i;

	switch (WhatToDo) {
	case STARTOVER:
		curLevel = 1;
		break;

	case NEXTLEVEL:
		curLevel++;
		break;

	default:
		break;
	}

	/* Initialize Snakes */
	sammy[0].head = 1;
	sammy[0].length = 2;
	sammy[0].alive = TRUE;
	sammy[1].head = 1;
	sammy[1].length = 2;
	sammy[1].alive = TRUE;

	InitColors();
	
	switch (curLevel) {
	case 1:
		sammy[0].row = 25;      sammy[1].row = 25;
		sammy[0].col = 50;      sammy[1].col = 30;
		sammy[0].direction = 4; sammy[1].direction = 3;
		break;

	case 2:
		for (i = 20; i <= 60; i++) {
			Set(25, i, COLOR_WALLS);
		}
		sammy[0].row = 7;       sammy[1].row = 43;
		sammy[0].col = 60;      sammy[1].col = 20;
		sammy[0].direction = 3; sammy[1].direction = 4;
		break;

	case 3:
		for (i = 10; i <= 40; i++) {
			Set(i, 20, COLOR_WALLS);
			Set(i, 60, COLOR_WALLS);
		}
		sammy[0].row = 25;      sammy[1].row = 25;
		sammy[0].col = 50;      sammy[1].col = 30;
		sammy[0].direction = 1; sammy[1].direction = 2;
		break;

	case 4:
		for (i = 4; i <= 30; i++) {
			Set(i, 20, COLOR_WALLS);
			Set(53 - i, 60, COLOR_WALLS);
		}
		for (i = 2; i <= 40; i++) {
			Set(38, i, COLOR_WALLS);
			Set(15, 81 - i, COLOR_WALLS);
		}
		sammy[0].row = 7;       sammy[1].row = 43;
		sammy[0].col = 60;      sammy[1].col = 20;
		sammy[0].direction = 3; sammy[1].direction = 4;
		break;

	case 5:
		for (i = 13; i <= 39; i++) {
			Set(i, 21, COLOR_WALLS);
			Set(i, 59, COLOR_WALLS);
		}
		for (i = 23; i <= 57; i++) {
			Set(11, i, COLOR_WALLS);
			Set(41, i, COLOR_WALLS);
		}
		sammy[0].row = 25;      sammy[1].row = 25;
		sammy[0].col = 50;      sammy[1].col = 30;
		sammy[0].direction = 1; sammy[1].direction = 2;
		break;

	case 6:
		for (i = 4; i <= 49; i++) {
			if (i > 30 || i < 23) {
				Set(i, 10, COLOR_WALLS);
				Set(i, 20, COLOR_WALLS);
				Set(i, 30, COLOR_WALLS);
				Set(i, 40, COLOR_WALLS);
				Set(i, 50, COLOR_WALLS);
				Set(i, 60, COLOR_WALLS);
				Set(i, 70, COLOR_WALLS);
			}
		}
		sammy[0].row = 7;       sammy[1].row = 43;
		sammy[0].col = 65;      sammy[1].col = 15;
		sammy[0].direction = 2; sammy[1].direction = 1;
		break;

	case 7:
		for (i = 4; i <= 49; i += 2) {
			Set(i, 40, COLOR_WALLS);
		}
		sammy[0].row = 7;       sammy[1].row = 43;
		sammy[0].col = 65;      sammy[1].col = 15;
		sammy[0].direction = 2; sammy[1].direction = 1;
		break;

	case 8:
		for (i = 4; i <= 40; i++) {
			Set(i, 10, COLOR_WALLS);
			Set(53 - i, 20, COLOR_WALLS);
			Set(i, 30, COLOR_WALLS);
			Set(53 - i, 40, COLOR_WALLS);
			Set(i, 50, COLOR_WALLS);
			Set(53 - i, 60, COLOR_WALLS);
			Set(i, 70, COLOR_WALLS);
		}
		sammy[0].row = 7;       sammy[1].row = 43;
		sammy[0].col = 65;      sammy[1].col = 15;
		sammy[0].direction = 2; sammy[1].direction = 1;
		break;

	case 9:
		for (i = 6; i <= 47; i++) {
			Set(i, i, COLOR_WALLS);
			Set(i, i + 28, COLOR_WALLS);
		}
		sammy[0].row = 40;      sammy[1].row = 15;
		sammy[0].col = 75;      sammy[1].col = 5;
		sammy[0].direction = 1; sammy[1].direction = 2;
		break;

	default:
		for (i = 4; i <= 49; i += 2) {
			Set(i, 10, COLOR_WALLS);
			Set(i + 1, 20, COLOR_WALLS);
			Set(i, 30, COLOR_WALLS);
			Set(i + 1, 40, COLOR_WALLS);
			Set(i, 50, COLOR_WALLS);
			Set(i + 1, 60, COLOR_WALLS);
			Set(i, 70, COLOR_WALLS);
		}
		sammy[0].row = 7;       sammy[1].row = 43;
		sammy[0].col = 65;      sammy[1].col = 15;
		sammy[0].direction = 2; sammy[1].direction = 1;
		break;
	}
}

/*
 *  Pauses game play and waits for space bar to be pressed before continuing
 */
static void SpacePause(const char *fmt, ...) {
	unsigned int i, j;
	char buf[30];
	char str[34];
	va_list ap;

	assert(fmt != NULL);

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	snprintf(str, sizeof str, "X %-29.29s X", buf);

	Color(COLOR_DIALOG);
	Center(11, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	Center(12, str);
	Center(13, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	refresh();

	while (getch() == ERR) {
		delay(100);
		/* nothing */
	}

	/* Restore the screen background */
	for (i = 21; i <= 26; i++) {
		for (j = 24; j <= 56; j++) {
			Set(i, j, arena[i - 1][j - 1].acolor);
		}
	}
	refresh();
}

/*
 * Prints players scores and number of lives remaining
 */
static void PrintScore(unsigned int NumPlayers,
	unsigned int score1, unsigned int score2,
	unsigned int lives1, unsigned int lives2)
{
	char buf[30];

	Color(COLOR_STATUS);

	/* TODO: perhaps %'u */
	if (NumPlayers == 2) {
		snprintf(buf, sizeof buf, "%5u00  Lives: %u  <--JAKE", score2, lives2);
		mvaddstr(0, 0, buf);
	}

	snprintf(buf, sizeof buf, "SAMMY-->  Lives: %u     %5u00", lives1, score1);
	mvaddstr(0, 50, buf);
	refresh();
}

/*
 * Checks the global arena array to see if the boolean flag is set
 */
static int PointIsThere(unsigned int row, unsigned int col, enum colorpair acolor) {
	if (row == 0) {
		return FALSE;
	}

	/* TODO: fold */
	if (arena[row - 1][col - 1].acolor != acolor) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*
 * Main routine that controls game play
 */
static void PlayNibbles(unsigned int NumPlayers, unsigned int speed, char diff) {
	struct snakeBody sammyBody[2][MAXSNAKELENGTH];
	struct snakeType sammy[2];
	unsigned int startRow1, startCol1;
	unsigned int startRow2, startCol2;
	unsigned int curSpeed;
	bool gameOver;

	/* Initialize Snakes */
	sammy[0].lives = 5;
	sammy[0].score = 0;
	sammy[0].scolor = COLOR_SNAKE1,
	sammy[1].lives = 5;
	sammy[1].score = 0;
	sammy[1].scolor = COLOR_SNAKE2,

	Level(STARTOVER, sammy);
	startRow1 = sammy[0].row; startCol1 = sammy[0].col;
	startRow2 = sammy[1].row; startCol2 = sammy[1].col;

	curSpeed = speed;

	/* play Nibbles until finished */

	SpacePause("     Level %u,  Push Space", curLevel);
	gameOver = FALSE;
	do {
		unsigned int number;
		unsigned int a;
		bool nonum;
		bool playerDied;

		if (NumPlayers == 1) {
			sammy[1].row = 0;
		}

		number = 1;          /* Current number that snakes are trying to run into */
		nonum = TRUE;        /* nonum = TRUE if a number is not on the screen */

		playerDied = FALSE;
		PrintScore(NumPlayers, sammy[0].score, sammy[1].score, sammy[0].lives, sammy[1].lives);
		Play("T160O1>L20CDEDCDL10ECC");

		do {
			unsigned int numberRow;
			unsigned int numberCol;


			/* Print number if no number exists */
			if (nonum == TRUE) {
				unsigned int sisterRow;

				do {
					numberRow = ((double) rand() / RAND_MAX) * 47 + 3;
					numberCol = ((double) rand() / RAND_MAX) * 78 + 2;

					sisterRow = numberRow + arena[numberRow - 1][numberCol - 1].sister;
				} while (!(!PointIsThere(numberRow, numberCol, COLOR_EMPTY) && !PointIsThere(sisterRow, numberCol, COLOR_EMPTY)));
				numberRow = arena[numberRow - 1][numberCol - 1].realRow;
				nonum = FALSE;
				Color(COLOR_NUMBER);
				mvaddch(numberRow - 1, numberCol - 1, '0' + number % 10);
			}

			/* Delay game */
			delay(curSpeed);

			/* Get keyboard input & Change direction accordingly */
			switch (getch()) {
			case 'w': case 'W': if (sammy[1].direction != 2) sammy[1].direction = 1; break;
			case 's': case 'S': if (sammy[1].direction != 1) sammy[1].direction = 2; break;
			case 'a': case 'A': if (sammy[1].direction != 4) sammy[1].direction = 3; break;
			case 'd': case 'D': if (sammy[1].direction != 3) sammy[1].direction = 4; break;
			case KEY_UP: if (sammy[0].direction != 2) sammy[0].direction = 1; break;	/* TODO: arrows! */
			case KEY_DOWN: if (sammy[0].direction != 1) sammy[0].direction = 2; break;
			case KEY_LEFT: if (sammy[0].direction != 4) sammy[0].direction = 3; break;
			case KEY_RIGHT: if (sammy[0].direction != 3) sammy[0].direction = 4; break;
			case 'p': case 'P': SpacePause(" Game Paused ... Push Space  ");
			case ERR: default: break;
			}

			for (a = 0; a < NumPlayers; a++) {
				/* Move Snake */
				switch (sammy[a].direction) {
				case 1: sammy[a].row = sammy[a].row - 1; break;
				case 2: sammy[a].row = sammy[a].row + 1; break;
				case 3: sammy[a].col = sammy[a].col - 1; break;
				case 4: sammy[a].col = sammy[a].col + 1; break;
				}

				/* If snake hits number, respond accordingly */
				if (numberRow == ((sammy[a].row + 1) / 2) && numberCol == sammy[a].col) {
					Play("MBO0L16>CCCE");
					if (sammy[a].length < (MAXSNAKELENGTH - 30)) {
						sammy[a].length = sammy[a].length + number * 4;
					}
					sammy[a].score = sammy[a].score + number;
					PrintScore(NumPlayers, sammy[0].score, sammy[1].score, sammy[0].lives, sammy[1].lives);
					number++;
					if (number == 10) {
						/* TODO: erase both simultaneously? */
						EraseSnake(&sammy[0], sammyBody[0]);
						EraseSnake(&sammy[1], sammyBody[1]);
						mvaddstr(numberRow, numberCol, " ");
						Level(NEXTLEVEL, sammy);
						PrintScore(NumPlayers, sammy[0].score, sammy[1].score, sammy[0].lives, sammy[1].lives);
						SpacePause("     Level %u,  Push Space", curLevel);
						if (NumPlayers == 1) { sammy[1].row = 0; }
						number = 1;
						if (diff == 'P') { speed = speed - 10; curSpeed = speed; }
					}
					nonum = TRUE;
					if (curSpeed < 1) { curSpeed = 1; }
				}
			}

			for (a = 0; a < NumPlayers; a++) {
				/* If player runs into any point, or the head of the other snake, it dies. */
				if (PointIsThere(sammy[a].row, sammy[a].col, COLOR_EMPTY) || (sammy[0].row == sammy[1].row && sammy[0].col == sammy[1].col)) {
					Play("MBO0L32EFGEFDC");
					Color(COLOR_EMPTY);

					move(numberRow - 1, numberCol - 1);
					addch(' ');

					playerDied = TRUE;
					sammy[a].alive = FALSE;
					sammy[a].lives = sammy[a].lives - 1;

				/* Otherwise, move the snake, and erase the tail */
				} else {
					unsigned int tail;

					sammy[a].head = (sammy[a].head + 1) % MAXSNAKELENGTH;
					sammyBody[a][sammy[a].head - 1].row = sammy[a].row;
					sammyBody[a][sammy[a].head - 1].col = sammy[a].col;
#if 0
					tail = (sammy[a].head + MAXSNAKELENGTH - sammy[a].length) % MAXSNAKELENGTH;
					Set(sammyBody[a][tail - 1].row, sammyBody[a][tail - 1].col, COLOR_EMPTY);
#else
                                        tail = (sammy[a].head + MAXSNAKELENGTH - sammy[a].length + 1) % MAXSNAKELENGTH - 1;
                                        Set(sammyBody[a][tail - 1].row, sammyBody[a][tail - 1].col, COLOR_EMPTY);
#endif
					sammyBody[a][tail - 1].row = 0;
					Set(sammy[a].row, sammy[a].col, sammy[a].scolor);
				}
			}
			refresh();
		} while (!playerDied);

		/* reset speed to initial value */
		curSpeed = speed;

		for (a = 0; a < NumPlayers; a++) {
			/* If dead, then erase snake in really cool way */
			EraseSnake(&sammy[a], sammyBody[a]);

			if (sammy[a].alive == FALSE) {
				/* Update score */
				sammy[a].score = sammy[a].score - 10;
				PrintScore(NumPlayers, sammy[0].score, sammy[1].score, sammy[0].lives, sammy[1].lives);
				
				if (a == 0) {
					SpacePause(" Sammy Dies! Push Space! --->");
				} else {
					SpacePause(" <---- Jake Dies! Push Space ");
				}
			}
		}

		Level(SAMELEVEL, sammy);
		PrintScore(NumPlayers, sammy[0].score, sammy[1].score, sammy[0].lives, sammy[1].lives);
	 
		/* Play next round, until either of snake's lives have run out. */
	} while (!(sammy[0].lives == 0 || sammy[1].lives == 0));
}

/*
 * Determines if users want to play game again.
 */
static bool StillWantsToPlay(void) {
	char kbd;

	Color(COLOR_DIALOG);
	Center(10, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	Center(11, "X       G A M E   O V E R       X");
	Center(12, "X                               X");
	Center(13, "X      Play Again?   (Y/N)      X");
	Center(14, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

	do {
		kbd = toupper((unsigned char) getch());
	} while (!(kbd == 'Y' || kbd == 'N'));

	Color(COLOR_EMPTY);
	Center(10, "                                 ");
	Center(11, "                                 ");
	Center(12, "                                 ");
	Center(13, "                                 ");
	Center(14, "                                 ");

	if (kbd == 'Y') {
		return TRUE;
	} else {
		return FALSE;
	}
}

static void restore(void) {
	endwin();
}

int main(void) {
	unsigned int NumPlayers;
	unsigned int speed;
	char diff;

	srand(time(NULL));

	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	start_color();

	atexit(restore);

	/* TODO: signal handler for segfault to restore()? */

	curs_set(0);
	ClearKeyLocks();
	Intro();
	curs_set(1);
	GetInputs(&NumPlayers, &speed, &diff);
	curs_set(0);
	SetColors();
	DrawScreen();

	do {
		timeout(0);
		PlayNibbles(NumPlayers, speed, diff);
		timeout(-1);
	} while (StillWantsToPlay());

	RestoreKeyLocks();
	attron(COLOR_PAIR(0));
	clear();
	refresh();
	curs_set(1);

	return EXIT_SUCCESS;
}

