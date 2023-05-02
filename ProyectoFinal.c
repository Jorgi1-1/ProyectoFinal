#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BOARD_SIZE 10

pid_t player_pid[2];
pthread_t timer_thread;
int timer = 120;
pthread_mutex_t timer_mutex;

void clear_screen() 
{
    system("clear");
}

void init_board(char board[][BOARD_SIZE]) 
{
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++) 
    {
        for (j = 0; j < BOARD_SIZE; j++) 
        {
            board[i][j] = '-';
        }
    }
}

void print_board(char board[][BOARD_SIZE]) 
{
    int i, j;
    printf("  ");
    for (i = 0; i < BOARD_SIZE; i++) 
    {
        printf("%d ", i);
    }
    printf("\n");
    for (i = 0; i < BOARD_SIZE; i++) 
    {
        printf("%c ", 'A' + i);
        for (j = 0; j < BOARD_SIZE; j++) 
        {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

int is_valid(char board[][BOARD_SIZE], int row, int col) 
{
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) 
    {
        return 0;
    }
    if (board[row][col] != '-') 
    {
        return 0;
    }
    return 1;
}

int place_ship(char board[][BOARD_SIZE], int size) 
{
    int row, col, dir;
    char dir_char;

    printf("Enter row and column for the bow of the ship of size %d (e.g. A5): \n", size);
    scanf(" %c %d", &dir_char, &col);
    row = dir_char - 'A';

    printf("Enter direction (0 for horizontal, 1 for vertical): \n");
    scanf("%d", &dir);

    for (int i = 0; i < size; i++) 
    {
        int new_row = row + (dir == 1 ? i : 0);
        int new_col = col + (dir == 0 ? i : 0);
        if (!is_valid(board, new_row, new_col)) 
        {
            printf("Invalid input. Try again.\n");
            return 0;
        }
    }

    for (int i = 0; i < size; i++) 
    {
        int new_row = row + (dir == 1 ? i : 0);
        int new_col = col + (dir == 0 ? i : 0);
        board[new_row][new_col] = 'O';
    }
        
    return 1;
}

void *timer_func(void *arg) 
{
    while (timer > 0) 
    {
        sleep(1);
        pthread_mutex_lock(&timer_mutex);
        timer--;
        pthread_mutex_unlock(&timer_mutex);
    }
    printf("Time's up!\n");
    kill(player_pid[0], SIGINT);
    kill(player_pid[1], SIGINT);
    return 0;
}

int get_input(char input[]) 
{
    fgets(input, 3, stdin);
    if (input[0] >= 'A' && input[0] < 'A' + BOARD_SIZE && input[1] >= '0' && input[1] < '0' + BOARD_SIZE) 
    {
        return 1;
    } else 
    {
        return 0;
    }
}

int check_hit(char board[][BOARD_SIZE], char pBoard[] [BOARD_SIZE], char input[]) 
{
    int row = input[0] - 'A';
    int col = input[1] - '0';
    if (board[row][col] == 'O') 
    {
        board[row][col] = 'X';
        pBoard[row][col] = 'X';
        return 1;

    }else if (board[row][col] == 'X' || pBoard[row][col] == 'o')
    {
        printf("You already fired there.\n");
        return 0;
    }else 
    {
        board[row][col] = '-';
        pBoard[row][col] = 'o';
        return 0;
    }
}

int check_win(char board[][BOARD_SIZE])
{
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++) 
    {
        for (j = 0; j < BOARD_SIZE; j++) 
        {
            if (board[i][j] == 'O') 
            {
                return 0;
            }
        }
    }
    return 1;
}

void play_game(char board1[][BOARD_SIZE], char board2[][BOARD_SIZE], char boardp1[][BOARD_SIZE], char boardp2[][BOARD_SIZE]) 
{
    char input[3];
    int player = 1;
    int hit;
    int p1Counter = 0;
    int p2Counter = 0; 
    while (!check_win(board1) && !check_win(board2)) 
    {
        clear_screen();
        if (player == 1) 
        {
            printf("Player 1's turn:\n");
            printf("Player 2's board: \n");
            printf("Player 2's hits: %d\n", p2Counter);
            print_board(boardp2);
            printf("Enter a coordinate to attack (e.g. A1): ");

            while (!get_input(input)) 
            {
                printf("Invalid input. Enter a coordinate to attack (e.g. A1): ");
            }
            
            hit = check_hit(board1, boardp2, input);
            printf("%s\n", hit ? "Hit!" : "Miss!");
            if(hit)
            {
                p1Counter++;
            }
            sleep(1);
        } else 
        {
            printf("Player 2's turn:\n");
            printf("Player 1's board: \n");
            printf("Player 1's hits: %d\n", p1Counter);
            print_board(boardp1);
            printf("Enter a coordinate to attack (e.g. A1): ");

            while (!get_input(input)) 
            {
                printf("Invalid input. Enter a coordinate to attack (e.g. A1): ");
            }

            hit = check_hit(board2, boardp1, input);
            printf("%s\n", hit ? "Hit!" : "Miss!");
            if(hit)
            {
                p2Counter++;
            }
            sleep(1);
        }
        player = player == 1 ? 2 : 1;
    }

    clear_screen();
    if (check_win(board1)) 
    {
        printf("Player 1 wins!\n");
        print_board(board1);
    } else 
    {
        printf("Player 2 wins!\n");
        print_board(board2);
    }
}

int main() 
{
    int ship_lengths[] = {5, 4, 3, 3, 2};

    char board1[BOARD_SIZE][BOARD_SIZE];
    char board2[BOARD_SIZE][BOARD_SIZE];
    char boardP1[BOARD_SIZE][BOARD_SIZE];
    char boardP2[BOARD_SIZE][BOARD_SIZE];

    init_board(board1);
    init_board(boardP2);
    init_board(board2);
    init_board(boardP1);


    printf("Player 1, place your ships: \n");
    for ( int i = 0; i < 5; i++)
    {
        int ship = ship_lengths[i];
        place_ship(board1, ship);
    }
    clear_screen();
    
    printf("Player 2, place your ships: \n");
    for (int i = 0; i < 5; i++)
    {
        int ship = ship_lengths[i];
        place_ship(board2, ship);
    }
    
    pthread_mutex_init(&timer_mutex, NULL);
    pthread_create(&timer_thread, NULL, timer_func, NULL);
    play_game(board1, board2, boardP1, boardP2);

    pthread_cancel(timer_thread);
    pthread_mutex_destroy(&timer_mutex);
    printf("Game over.\n");
    return 0;
}
