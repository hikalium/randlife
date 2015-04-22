//
// randlife.c
// by hikalium
// 2012 - 2015
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// environment depended
#ifdef _WIN32
#include <conio.h>
//	getch();
//	kbhit();

int getchar_noWait(void)
{
	if(kbhit()){
		return getch();		
	}
	return -1;
}

void clearScreen(void)
{
	system("cls");		
}

#endif

#ifdef __linux
#include <time.h>
#endif

#ifdef __APPLE__
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int getchar_noWait(void)
{
	// retv: -1 (no any inputs) or character code.
	struct termios oldt, newt;
	int ch, oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();
	
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF){
		return ch;		
	}
	return -1;	
}

void clearScreen(void)
{
	system("clear");
}

#endif

/*---from CHNOSProject----*/
/*boolean*/
#define True	1
#define False	0

/*null*/
#define Null	0

typedef unsigned char uchar;
typedef unsigned int uint;
typedef struct DATA_LOCATION_2DU {
	uint x;
	uint y;
} DATA_Location2DU;

/*----randlife----*/
#define CONSOLE_OUT
#define VIEW_WAIT
#define VIEW_WAIT_USEC	50000
//bitsize must be less than (1 << (16 - 2)).
#define MATH_BINARY_MAP_2D_MAXSIZE	0x4000
#define RULE_BE_BORN	2
#define RULE_BE_ALIVE	1
#define RULE_DIE	0

#define TESTSIZEX	50
#define TESTSIZEY	50

typedef DATA_Location2DU DATA_Size2DU;

typedef struct MATH_BINARY_MAP_2D {
	DATA_Size2DU bitsize;
	uint bytesize;
	uint *map;	//uint=32bit
} MATH_BinaryMap2D;

typedef struct LIFEGAME {
	MATH_BinaryMap2D *mainmap, *submap;
	uint generation;
	uint livecells;
	uchar rule[9];
} AL_LifeGame;

AL_LifeGame *AL_LifeGame_Initialize(uint bitx, uint bity);
uint AL_LifeGame_SetRule(AL_LifeGame *lifegame, const uchar *rule);
uint AL_LifeGame_Next(AL_LifeGame *lifegame);
MATH_BinaryMap2D *MATH_BinaryMap2D_Initialize(uint bitx, uint bity);
uint MATH_BinaryMap2D_Free(MATH_BinaryMap2D *map);
uint MATH_BinaryMap2D_GetBit(MATH_BinaryMap2D *map, int bitx, int bity);
uint MATH_BinaryMap2D_SetBit(MATH_BinaryMap2D *map, int bitx, int bity, uint data);
uint MATH_BinaryMap2D_CopyMap(MATH_BinaryMap2D *destination, MATH_BinaryMap2D *source);

FILE *logfile;
FILE *maplogfile;

int main(int argc, char *argv[])
{
	int x, y, i, k, count, c;
	AL_LifeGame *lifegame;
	uint livecells_old;
	uint nextflag;
	uint starttime;
	uchar rule[9] = {
		RULE_DIE,
		RULE_DIE,
		RULE_BE_ALIVE,
		RULE_BE_ALIVE | RULE_BE_BORN,
		RULE_DIE,
		RULE_DIE,
		RULE_DIE,
		RULE_DIE,
		RULE_DIE,
	};
	MATH_BinaryMap2D *outmap;
	char s[1024];

//Log file setting
	snprintf(s, sizeof(s), "log-%dx%d.txt", TESTSIZEX, TESTSIZEY);
	logfile = fopen(s, "w");
	if(logfile == Null){
		printf("main:Log file error.\nAbort.\n");
		exit(EXIT_FAILURE);
	}
	snprintf(s, sizeof(s), "log-%dx%d-map.txt", TESTSIZEX, TESTSIZEY);
	maplogfile = fopen(s, "w");
	if(maplogfile == Null){
		printf("main:Log file error.\nAbort.\n");
		exit(EXIT_FAILURE);
	}

	nextflag = False;

	lifegame = AL_LifeGame_Initialize(TESTSIZEX, TESTSIZEY);
	outmap = MATH_BinaryMap2D_Initialize(TESTSIZEX, TESTSIZEY);
#ifdef CONSOLE_OUT
	printf("%p\n", lifegame);
#endif
	AL_LifeGame_SetRule(lifegame, rule);

	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 10, 10, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 10, 11, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 10, 12, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 11, 10, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 12, 11, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 15, 7, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 16, 7, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 17, 7, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 18, 7, True);
	//MATH_BinaryMap2D_SetBit(lifegame->mainmap, 19, 7, True);


	fprintf(logfile, "<randlife.c>\n\nMapSize = %dx%d = %d cells\n", lifegame->mainmap->bitsize.x, lifegame->mainmap->bitsize.y, lifegame->mainmap->bitsize.x * lifegame->mainmap->bitsize.y);

	starttime = time(Null);
	for(k = 0; k < 1024; k++){
		count = 0;
		livecells_old = 0;

		srand(time(Null) + k);
		for(y = 0; y < (int)lifegame->mainmap->bitsize.y; y++){
			for(x = 0; x < (int)lifegame->mainmap->bitsize.x; x++){
				MATH_BinaryMap2D_SetBit(lifegame->mainmap, x, y, rand() & 1);
			}
		}
		MATH_BinaryMap2D_CopyMap(lifegame->submap, lifegame->mainmap);
		MATH_BinaryMap2D_CopyMap(outmap, lifegame->mainmap);
		lifegame->generation = 0;

#ifdef CONSOLE_OUT
		clearScreen();

		printf("%d:generation=%d:cells=Unknown / %d\n", k, lifegame->generation, lifegame->mainmap->bitsize.x * lifegame->mainmap->bitsize.y);
		printf("  ");
		for(x = 0; x < (int)lifegame->mainmap->bitsize.x; x++){
			printf("%2d", x);
		}
		printf("\n");
		for(y = 0; y < (int)lifegame->mainmap->bitsize.y; y++){
			printf("%2d", y);
			for(x = 0; x < (int)lifegame->mainmap->bitsize.x; x++){
				printf("%2c", MATH_BinaryMap2D_GetBit(lifegame->mainmap, x, y) ? 'O' : ' ');
			}
			printf("\n");
		}
		printf("----\n");
		printf("Press Ctrl + c to exit.\n");
#endif

		for(i = 0; i < 16384; i++){
#ifdef CONSOLE_OUT
			clearScreen();
			printf("\n");
#endif
			AL_LifeGame_Next(lifegame);
#ifdef CONSOLE_OUT
			printf("%d:generation=%d:cells=%d / %d\n", k, lifegame->generation, lifegame->livecells, lifegame->mainmap->bitsize.x * lifegame->mainmap->bitsize.y);
			printf("  ");
			for(x = 0; x < (int)lifegame->mainmap->bitsize.x; x++){
				printf("%2d", x);
			}
			printf("\n");
			for(y = 0; y < (int)lifegame->mainmap->bitsize.y; y++){
				printf("%2d", y);
				for(x = 0; x < (int)lifegame->mainmap->bitsize.x; x++){
					printf("%2c", MATH_BinaryMap2D_GetBit(lifegame->mainmap, x, y) ? 'O' : ' ');
				}
				printf("\n");
			}
			printf("----\n");
			printf("q: quit, n: nextWorld, other: pause.\n");
#endif
			if(livecells_old == lifegame->livecells){
				if(count == 5){
#ifdef CONSOLE_OUT
					printf("\nEnd of growing. Go to next world.");
#endif
					//fprintf(logfile,"%d:%d:End(livecells = %d)\n", k, i, lifegame->livecells);
					fprintf(logfile,"%d,%d,%d\n", k, i, lifegame->livecells);
					break;
				} else{
					count++;
				}
			} else{
				count = 0;
				livecells_old = lifegame->livecells;
			}
			c = getchar_noWait();
			if(c != -1){
				switch(c){
					case 'n':
#ifdef CONSOLE_OUT
						printf("\nEnd of growing. Go to next world.");
#endif
						fprintf(logfile,"%d:%d(Force):End(livecells = %d)\n", k, i, lifegame->livecells);
						nextflag = True;
						break;
					case 'q':
#ifdef CONSOLE_OUT
						printf("\nQuit.");
#endif
						fprintf(logfile,"%d:%d(Quit):End(livecells = %d)\n", k, i, lifegame->livecells);
						exit(EXIT_SUCCESS);
					default:
#ifdef CONSOLE_OUT
						printf("\nPause.");
#endif
						getchar();
						break;
				}
			}
			if(nextflag){
				nextflag = False;
				break;
			}
#ifdef VIEW_WAIT
			usleep(VIEW_WAIT_USEC);
#endif
		}
		if(i == 16384){
			//fprintf(logfile,"%d:%d(MAX):End(livecells = %d)\n", k, i, lifegame->livecells);
			fprintf(logfile,"%d,%d,%d\n", k, i, lifegame->livecells);
			fprintf(maplogfile,"%d,%d,%d\n", k, i, lifegame->livecells);
			fprintf(maplogfile,"Map at generation 0.\n");
			for(y = 0; y < (int)outmap->bitsize.y; y++){
				for(x = 0; x < (int)outmap->bitsize.x; x++){
					fprintf(maplogfile,"%c", MATH_BinaryMap2D_GetBit(outmap, x, y) ? '@' : ' ');
				}
				fprintf(maplogfile,"\n");
			}
			fprintf(maplogfile,"Map at generation %d.\n", lifegame->generation);
			for(y = 0; y < (int)lifegame->mainmap->bitsize.y; y++){
				for(x = 0; x < (int)lifegame->mainmap->bitsize.x; x++){
					fprintf(maplogfile,"%2c", MATH_BinaryMap2D_GetBit(lifegame->mainmap, x, y) ? 'O' : ' ');
				}
				fprintf(maplogfile,"\n");
			}
			fprintf(maplogfile,"----\n");

		}
	}
	fprintf(logfile,"Processed in %ld seconds.\n", time(Null) - starttime);
	fclose(maplogfile);
	fclose(logfile);
	return 0;
}

AL_LifeGame *AL_LifeGame_Initialize(uint bitx, uint bity)
{
	AL_LifeGame *lifegame;
	uint x, y;

	lifegame = malloc(sizeof(AL_LifeGame));

	lifegame->mainmap = MATH_BinaryMap2D_Initialize(bitx, bity);
	lifegame->submap = MATH_BinaryMap2D_Initialize(bitx, bity);
	lifegame->generation = 0;
	lifegame->livecells = 0xffffffff;

	if(lifegame->mainmap == Null || lifegame->submap == Null){
#ifdef CONSOLE_OUT
		printf("AL_LifeGame_Initialize:Map initialize error.\nAbort.\n");
#endif
		exit(EXIT_FAILURE);
	}
	return lifegame;
}

uint AL_LifeGame_SetRule(AL_LifeGame *lifegame, const uchar *rule)
{
	uint i;

	if(lifegame == Null){
		return 1;
	}

	for(i = 0; i < 9; i++){
		lifegame->rule[i] = rule[i];
	}
	return 0;
}

uint AL_LifeGame_Next(AL_LifeGame *lifegame)
{
	uint lives, livecells;
	int x, y;

	if(lifegame == Null){
		return 1;
	}

	livecells = 0;
	for(y = 0; y < (int)lifegame->mainmap->bitsize.y; y++){
		for(x = 0; x < (int)lifegame->mainmap->bitsize.x; x++){
			lives = 0;
			lives += MATH_BinaryMap2D_GetBit(lifegame->mainmap, x - 1, y - 1);
			lives += MATH_BinaryMap2D_GetBit(lifegame->mainmap, x - 1, y + 0);
			lives += MATH_BinaryMap2D_GetBit(lifegame->mainmap, x - 1, y + 1);
			lives += MATH_BinaryMap2D_GetBit(lifegame->mainmap, x + 0, y - 1);
			lives += MATH_BinaryMap2D_GetBit(lifegame->mainmap, x + 0, y + 1);
			lives += MATH_BinaryMap2D_GetBit(lifegame->mainmap, x + 1, y - 1);
			lives += MATH_BinaryMap2D_GetBit(lifegame->mainmap, x + 1, y + 0);
			lives += MATH_BinaryMap2D_GetBit(lifegame->mainmap, x + 1, y + 1);
			if(lives >= 9){
#ifdef CONSOLE_OUT
				printf("AL_LifeGame_Next:lives error.\nAbort.\n");
#endif
				exit(EXIT_FAILURE);
			}
			livecells += lives;
			switch(lifegame->rule[lives]){
				case RULE_DIE:
					MATH_BinaryMap2D_SetBit(lifegame->submap, x, y, False);
					break;
				case RULE_BE_BORN:
				case (RULE_BE_BORN | RULE_BE_ALIVE):
					MATH_BinaryMap2D_SetBit(lifegame->submap, x, y, True);
					break;
			}
		}
	}
	MATH_BinaryMap2D_CopyMap(lifegame->mainmap, lifegame->submap);

	lifegame->generation++;
	lifegame->livecells = (livecells >> 3);
	return 0;
}

MATH_BinaryMap2D *MATH_BinaryMap2D_Initialize(uint bitx, uint bity)
{
	uint size;
	
	MATH_BinaryMap2D *map;
	size = ((bitx * bity) + (32 - 1)) >> (5 - 3);

	if(size > MATH_BINARY_MAP_2D_MAXSIZE){
		printf("MATH_BinaryMap2D_Initialize:Too large map size.\nAbort.\n");
		exit(EXIT_FAILURE);
	}

	map = malloc(sizeof(MATH_BinaryMap2D));
	map->bitsize.x = bitx;
	map->bitsize.y = bity;
	map->bytesize = size;
	map->map = malloc(size);
	memset(map->map, 0, size);

	return map;
}

uint MATH_BinaryMap2D_Free(MATH_BinaryMap2D *map)
{
	free(map->map);
	free(map);

	return 0;
}

uint MATH_BinaryMap2D_GetBit(MATH_BinaryMap2D *map, int bitx, int bity)
{
	uint bit, modbit;

	if(map == Null){
		return 0;
	}
	if(map->map == Null){
		return 0;
	}

	if(bitx >= (int)map->bitsize.x){
		if(bitx < (int)(map->bitsize.x << 1)){
			bitx -= map->bitsize.x;
		} else{
			bitx = bitx % map->bitsize.x;
		}
	} else if(bitx < 0){
		if(bitx >= -(int)map->bitsize.x){
			bitx += map->bitsize.x;
		} else{
			bitx = 4 + (bitx % map->bitsize.x);
		}
	}

	if(bity >= (int)map->bitsize.y){
		if(bity < (int)(map->bitsize.y << 1)){
			bity -= map->bitsize.y;
		} else{
			bity = bity % map->bitsize.y;
		}
	} else if(bity < 0){
		if(bity >= -(int)map->bitsize.y){
			bity += map->bitsize.y;
		} else{
			bity = 4 + (bity % map->bitsize.y);
		}
	}

	bit = (bity * map->bitsize.x) + bitx;
	modbit = bit & (32 - 1);

	return ((map->map[bit >> 5] & (1 << modbit)) >> modbit);
}

uint MATH_BinaryMap2D_SetBit(MATH_BinaryMap2D *map, int bitx, int bity, uint data)
{
	uint bit, modbit;

	if(map == Null){
		return 1;
	}
	if(map->map == Null){
		return 2;
	}

	if(bitx >= (int)map->bitsize.x){
		if(bitx < (int)(map->bitsize.x << 1)){
			bitx -= map->bitsize.x;
		} else{
			bitx = bitx % map->bitsize.x;
		}
	} else if(bitx < 0){
		if(bitx >= -(int)map->bitsize.x){
			bitx += map->bitsize.x;
		} else{
			bitx = 4 + (bitx % map->bitsize.x);
		}
	}

	if(bity >= (int)map->bitsize.y){
		if(bity < (int)(map->bitsize.y << 1)){
			bity -= map->bitsize.y;
		} else{
			bity = bity % map->bitsize.y;
		}
	} else if(bity < 0){
		if(bity >= -(int)map->bitsize.y){
			bity += map->bitsize.y;
		} else{
			bity = 4 + (bity % map->bitsize.y);
		}
	}

	bit = (bity * map->bitsize.x) + bitx;
	modbit = bit & ((1 << 5) - 1);

	if(data){
		map->map[bit >> 5] = map->map[bit >> 5] | (1 << modbit);
	} else{
		map->map[bit >> 5] = map->map[bit >> 5] & ~(1 << modbit);
	}
	return 0;
}

uint MATH_BinaryMap2D_CopyMap(MATH_BinaryMap2D *destination, MATH_BinaryMap2D *source)
{
	memcpy(destination->map, source->map, destination->bytesize);
	return 0;
}

