/* CS 355 Systems Programming 
 * Final Project: Snake Game
 * Alexandra Iaquessa and Kara Martin
 *
 * use 'sudo apt-get install libncurses5 libncurses5-dev' if Google Cloud Shell can't find curses.h
 * compile with '-lcurses'
 * 
 * Intermediate Deliverable Date:   11/23/2020
 * Final Deliverable Date:          12/04/2020
 * 
 * Final Deliverable Functionality:
 * [✓] Snake Pit
 *      [✓]  The snake pit is the area where the snake can move.
 *      [✓]  The snake pit must utilize all available space of the current terminal window.
 *      [✓]  There must be a visible border delineating the snake pit.
 * [✓] Snake
 *      [✓]  Initial length 3 (can easily change, leave as 5 for debugging)
 *      [✓]  Random initial direction.
 *      [✓]  4 arrow keys change direction.
 *      [✓]  Speed proportional to length.
 * [✓] Trophies
 *      [✓] Represented from random digit from 1-9.
 *      [✓] Exactly 1 trophy in pit at any moment.
 *      [✓] Snake eats trophy -> length += trophy #
 *      [✓] Trophy expires after random interval from 1-9 seconds
 *      [✓] New trophy shown at random location on the screen after eaten/expires
 * [✓] Gameplay
 *      [✓] Snake dies on running into border
 *      [✓] Snake dies on running into itself
 *      [✓] User tries to reverse direction
 *      [✓] User wins when length >= border perimeter / 2
 */

#include <curses.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>

int alive = 1;
int reverseFlag = 0;                                        // Checks if user tried to reverse direction
int runningIntoSelfFlag = 0;                                // Checks if head hits body
int length;
int winThreshold;                                           // When length > Perimeter / 2, user wins. 
int xdir;                                                   // Up or Down
int ydir;                                                   // Left or Right (so starting direction is right)
int trophyValue;
int trophy[2] = {0};

void endGame(char *);
void newTrophy();

int main() {
    initscr();                                              // initialize the curses library
    clear();                                                // clear the screen
    curs_set(0);                                            // hide the cursor
    noecho();                                               // do not echo the user input to the screen
    keypad(stdscr,TRUE);                                    // enables working with the arrow keys
    nodelay(stdscr, TRUE);                                  // Makes getch a non-blocking call
    srand(time(NULL));                                      // RNG for trophies   
    int head[2] = {LINES/2,COLS/2};                         // Snake setup
    int body[500][2] = {0};
    body[0][0] = LINES/2;
    body[0][1] = COLS/2;

    length = 3;                                             // Starting length of snake
    winThreshold = LINES + COLS - 2;                        // Perimeter = LINES*2 + COLS*2 - 4 as corners are double counted
    move(LINES/2, COLS/2);                                  // Start in middle 
    start_color();			                                // Color the board
    init_pair(1, COLOR_WHITE, COLOR_BLACK);                 // Border Color
    init_pair(2, COLOR_GREEN, COLOR_BLACK);                 // Snake Color
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);                // Trophy Color       

    newTrophy();
    alarm(trophyValue);
    
    int initialDirection = rand() % 4;                      // Generate a random number from 0-3 to determine starting direction
    switch(initialDirection) {
        case 0:                                             // Up
            xdir = -1;
            ydir = 0;
            body[1][0] = (LINES/2) + 1;
            body[1][1] = COLS/2;
            body[2][0] = (LINES/2) + 2;
            body[2][1] = COLS/2;
            break;
        case 1:                                             // Down
            xdir = 1;
            ydir = 0;
            body[1][0] = (LINES/2) - 1;
            body[1][1] = COLS/2;
            body[2][0] = (LINES/2) - 2;
            body[2][1] = COLS/2;
            break;
        case 2:                                             // Left
            xdir = 0;
            ydir = -1;
            body[1][0] = LINES/2;
            body[1][1] = (COLS/2) + 1;
            body[2][0] = LINES/2;
            body[2][1] = (COLS/2) + 2;
            break;
        case 3:                                             // Right, default
        default:
            xdir = 0;
            ydir = 1;
            body[1][0] = LINES/2;
            body[1][1] = (COLS/2) - 1;
            body[2][0] = LINES/2;
            body[2][1] = (COLS/2) - 2;
            break;
    }

    while(alive) {                                          // Main game loop
        erase();
        attron(COLOR_PAIR(1));                              // Print border
        border('|', '|', '-', '-', '*', '*', '*', '*'); 
        attroff(COLOR_PAIR(1));        
        int ch = getch();

        attron(COLOR_PAIR(3));                              // Print trophy
        mvprintw(trophy[0], trophy[1], "%d", trophyValue);
        attroff(COLOR_PAIR(3));       

        attron(COLOR_PAIR(2));
        for(int i = 1; i < length ; i++)                    // Printing the body 
            mvprintw(body[i][0], body[i][1], "o");

        mvprintw(head[0], head[1], "@");                    // Printing the head
        attroff(COLOR_PAIR(2));

        switch(ch) {                                        // Change x/y direction based on user input
            case KEY_UP:
                if( xdir == 1 ) 
                    reverseFlag = 1;
                else {
                    xdir = -1;
                    ydir = 0;                
                }
                break;
            case KEY_DOWN:    
                if( xdir == -1 )
                    reverseFlag = 1;
                else {  
                    xdir = 1;
                    ydir = 0;
                }
                break;
            case KEY_LEFT:     
                if( ydir == 1 )
                    reverseFlag = 1;
                else { 
                    xdir = 0;
                    ydir = -1;
                }
                break;
            case KEY_RIGHT:
                if( ydir == -1 )
                    reverseFlag = 1;
                else {
                    xdir = 0;
                    ydir = 1;
                }
                break;
            case 'x':
            case 'X': 
                endwin();
                return 0;
            default:              
                break;
        }

        head[0] += xdir;                                    // New snake head and body part positions
        head[1] += ydir;

        for(int i = length - 1; i > 0 ; i--)                // Snake body values
        {
            body[i][0] = body[i-1][0];
            body[i][1] = body[i-1][1];

            if( head[0] == body[i][0] && head[1] == body[i][1] )
                runningIntoSelfFlag = 1;
            
            if( head[0] == trophy[0] && head[1] == trophy[1] ) {
                for(int j = 0; j < trophyValue; j++) {
                    body[length + j][0] = body[length - 1][0];
                    body[length + j][1] = body[length - 1][1];
                }
                length += trophyValue;
                newTrophy();
            }
        }
        body[0][0] = head[0];
        body[0][1] = head[1];
        refresh(); 

        if( xdir == 0 ) {                                   // Speed proportional to length
            if( length <= 100 )
                usleep(150000 - (length * 1000));           // Moving left or right, speed should go from ~150k to ~50k
            else
                usleep(50000);
        }
        else if( ydir == 0 ) {
            if( length <= 100 )                             // There's usually anywhere from 3-10x more columns than lines, so account for that
                usleep(225000 - (length * 1000));           // Moving up or down, speed should go from ~225k to ~125k
            else
                usleep(125000);
        }

        if( head[0] == 0 || head[0] >= LINES - 1 || head[1] == 0 || head[1] >= COLS - 1 )  
            endGame("You hit a wall :(");                   // If head hits border, end the game.
        if( reverseFlag )                                   // If snake reverses direction, end the game. 
            endGame("You tried to reverse direction :(");
        if( runningIntoSelfFlag )                           // If snake runs into itself, end the game. 
            endGame("You ran into your body :(");
        if( length >= winThreshold )                        // If the length is longer than the threshold, end the game.
            endGame("You win!!! :)");
    }
    endwin();
    return 0;
}

void endGame(char * exitMessage) {                          // Print centered message and kill snake
    char * pressAnyKey = "Press any key to exit...";        // Don't repeat yourself
    mvprintw(LINES/2, (COLS - strlen(exitMessage))/2, exitMessage);
    mvprintw((LINES/2)+1, (COLS - strlen(pressAnyKey))/2, pressAnyKey);
    alive = 0;
    refresh();
    getchar();
    clear();
    return;
}

void newTrophy() {                                          // Generate next trophy and set alarm for next one
    signal(SIGALRM, newTrophy);
    trophyValue = ( rand() % 9 ) + 1;                       // Generate a random number from 1-9 to determine trophy value
    int ch;
    do {
        trophy[0] = ( rand() % (LINES - 2) ) + 1;           // Make sure the trophy is in-bounds
        trophy[1] = ( rand() % (COLS - 2) ) + 1;
        ch = mvinch(trophy[0], trophy[1]) & A_CHARTEXT;
    } while(ch == 111 || ch == 64);                         // Loop until the character at (t[0], t[1]) isn't 'o' or '@'
    alarm(trophyValue);
}
