#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sem.h>

#define FIFONAME "fifo_clientTOserver"
#define KEYNAME "Ex31.c"
#define MEMSIZE 1024
#define SEMKEY "Ex32.c"
#define WAIT_MSG "Waiting for other player to make a turn\n"
#define WINMSG "You won!\n"
#define LOSEMSG "You lost!\n"

char* memVar;
int rowNum = -1;
int columnNum = -1;
int lastTurnMadeRow = -1;
int lastTurnMadeColumn = -1;
struct sembuf sb;
int semid;
int memKey;

union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
};
void lock() {
	/*Locks the semaphore*/
	sb.sem_op = -1;
	semop(semid, &sb, 1);
}
void unlock() {
	/*Unlocks the semaphore*/	
	sb.sem_op = 1;
	semop(semid, &sb, 1);
}
void removeFromBoard(int row, int column, char* board, int * numOfChars) {
	/*Receives: current board, current number of left chars, row and column.
	The function removes all 'X' which are placed above and to the right of the given
	location (row and column) and updates the board and number of chars accordingly.*/	
	int i, j;
	for (i = 0; i <= columnNum * row; i += columnNum) {
		for (j = i; j <= (i + column); ++j) {
			if (board[j] != '\0') {
				board[j] = '\0';
				(*numOfChars)--;
			}
		}
	}
}
void printCurrentBoardState(const char* board) {
	/*Receives current board and prints it out to the user.*/
	int i, j;
	for (i = 0; i < rowNum; ++i) {
		for (j = i * columnNum; j < (i * columnNum) + columnNum; ++j) {
			if (board[j] == '\0') {
				write(1, " ", 1);
			} else {
				write(1, "X", 1);
			}
		}
		write(1, "\n", 1);
	}
}
void updateBoard(char * board, char * turnMade, int * numOfChars) {
	/*Receives: current board, the new turn which was made by the player, current number of left chars.
	The function splits the given turn string into two chars (row and column) and uses them to update the server's board*/

	int row, column, i, switchToColumns;
	i = 1;
	row = 0;
	column = 0;
	switchToColumns = 0;
	while (turnMade[i] != '\0') {
		if (turnMade[i] == ',') {
			i++;
			switchToColumns = 1;
		}
		if (switchToColumns == 0) {
			row *= 10;
			row += turnMade[i] - '0';
		} else {
			column *= 10;
			column += turnMade[i] - '0';
		}
		i++;
	}
	removeFromBoard(row, column, board, numOfChars);
	printCurrentBoardState(board); /*Prints the board after removal*/
}
void playTurn(char* board, int myNum, int* numOfChars) {
	/*Generic turn function. Prints out a menu, receives inputs and removes from the board with given inputs.*/
	int curRow, curColumn, math;
	while (1) {
		write(1, "Please choose a row and column\n",
				strlen("Please choose a row and column\n"));
		scanf("%d%d", &curRow, &curColumn);
		math = curColumn + curRow * columnNum;
		if (curColumn > (columnNum - 1) || curRow > (rowNum - 1)) {
			write(1, "No such place\n", strlen("No such place\n"));
			continue;
		}
		if (board[math] == '\0') {
			write(1, "This place is empty\n", strlen("This place is empty\n"));
			continue;
		}
		break;
	}
	lastTurnMadeRow = curRow;
	lastTurnMadeColumn = curColumn;

	removeFromBoard(curRow, curColumn, board, numOfChars);
	printCurrentBoardState(board);

	write(1, WAIT_MSG, strlen(WAIT_MSG));
	char tmpBuffer[1024] = { '\0' };
	sprintf(tmpBuffer, "%d%d,%d", myNum, lastTurnMadeRow, lastTurnMadeColumn);

	lock();
	strcpy(memVar, tmpBuffer);
	unlock();
}
int checkFinish(int* numOfChars) {
	/*Check the amount of chars which are non null. If there are none- the game is over*/
	if ((*numOfChars) == 0) {
		return 1;
	}
	return 0;
}
void initSemaphore() {
	/*Initiate semaphore*/
	key_t semKey = ftok(SEMKEY, 'r');
	semid = semget(semKey, 1, 0666);
	/*semaphore now set*/
}
void readMsg(char* msgToCheck) {
	/*Reads the msg that has the number of rows and columns and updates local
	variables: rowNum,columnNum*/	
	int rows, columns, change;
	change = 0;
	rows = 0;
	columns = 0;
	int i;
	for (i = 0; i < strlen(msgToCheck); ++i) {
		if (msgToCheck[i] == ',') {
			change = 1;
			i++;
		}
		if (change == 0) {
			rows *= 10;
			rows += msgToCheck[i] - '0';
		} else {
			columns *= 10;
			columns += msgToCheck[i] - '0';
		}
	}
	rowNum = rows;
	columnNum = columns;
}
void updateRowsAndColumns(int* numOfChars) {
	/*Updates local information about the board*/
	char tmpMemHolder[1024];
	lock();
	strcpy(tmpMemHolder, memVar);
	unlock();
	readMsg(tmpMemHolder);
	*numOfChars = rowNum * columnNum;
}

void firstTurn(char * board, int * numOfChars, int * conNum) {
	/*The first turn tells each client his connection number (first client or second client) and plays
	a turn normally on the first client*/
	char tmpMemHolder[1024];
	char tmpBuffer[1024] = { '\0' };
	sprintf(tmpBuffer, "p%d", getpid());

	lock();
	strcpy(tmpMemHolder, memVar);
	unlock();
	if (strcmp(tmpMemHolder, tmpBuffer) == 0) {
		*conNum = 1;
		printCurrentBoardState(board);
		playTurn(board, *conNum, numOfChars);
		if (checkFinish(numOfChars) == 1) {
			write(1, LOSEMSG, strlen(LOSEMSG));
			free(board);
			free(numOfChars);
			shmdt(&memKey);
			exit(0);
		}
	} else {
		*conNum = 2;
		write(1, WAIT_MSG, strlen(WAIT_MSG));
	}
}
void wholeTurn(char* board, char* holdLastMsg, int* numOfChars,int* conNum) {
	/*Receives the current state of the board, the last msg ,current number of chars and connection number (client1 or client2).
	It then takes care of printing the new board, printing the menu and closing the client (if needed).*/
	updateBoard(board, holdLastMsg, numOfChars);
	if (checkFinish(numOfChars) == 1) {
		write(1, WINMSG, strlen(WINMSG));
		free(board);
		free(numOfChars);
		free(conNum);
		shmdt(&memKey);
		exit(0);
	}
	playTurn(board, *conNum, numOfChars);
	if (checkFinish(numOfChars) == 1) {
		write(1, LOSEMSG, strlen(LOSEMSG));
		free(board);
		free(numOfChars);
		free(conNum);
		shmdt(&memKey);
		exit(0);
	}
}
int main(void) {
	int fd = open(FIFONAME, O_WRONLY);
	int* numOfChars;
	pid_t pid = getpid();
	write(fd, &pid, sizeof(pid_t));
	close(fd);
	sb.sem_num = 0;
	sb.sem_flg = 0;
	key_t currentKey = ftok(KEYNAME, 'j');
	memKey = shmget(currentKey, MEMSIZE, 0666); //create shared memory
	memVar = (char*) shmat(memKey, NULL, 0);
	initSemaphore();

	int* conNum = malloc(sizeof(int));
	numOfChars = malloc(sizeof(int));

	updateRowsAndColumns(numOfChars);

	char* board = malloc(sizeof(char) * rowNum * columnNum);
	for (int i = 0; i < rowNum * columnNum; ++i) {
		board[i] = 'X';
	}
	firstTurn(board, numOfChars, conNum);
	char holdLastMsg[1024] = { '\0' };
	while (1) {
		lock();
		strcpy(holdLastMsg, memVar);
		unlock();
		if (holdLastMsg[0] == 'p') {
			sleep(1);
			continue;
		}
		if ((holdLastMsg[0] - '0') != *conNum) {
			wholeTurn(board,holdLastMsg,numOfChars,conNum);
		}
		sleep(1);
	}
	return 0;
}
