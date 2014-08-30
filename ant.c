#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEBUG				0

#define EXE_STATUS_ERROR	-1

#define MAX_COMMAND_STR	16

/*
#define MAX_MAP_HEIGHT	50
#define MAX_MAP_WIDTH	170
*/
#define MAX_MAP_HEIGHT	20
#define MAX_MAP_WIDTH	20

#define MAX_ANT_NUM		10
#define ANT_INIT_X		10
#define ANT_INIT_Y		10
#define ANT2_INIT_X		0
#define ANT2_INIT_Y		0
#define ANT_DEFAULT_FORCE	10
#define MAX_PHEROMONE_FORCE	10
#define ANT_ASCII		'*'

enum DIRECTION {
	NORTH,
	EAST,
	SOUTH,
	WEST,
	MAX_DIRECTIONS
} enum_directions;

enum LOCAL_DIRECTION {
	FRONT,
	RIGHT,
	LEFT,
	BACK,
	MAX_LOCAL_DIRECTIONS
} enum_local_directions;

struct ant_status {
	unsigned int now_x;
	unsigned int now_y;
	unsigned char now_dir;
	char pheromone_force;
};

void field_init(unsigned int width, unsigned int height);
void field_do_1turn(unsigned int num_ants);
void field_disperse_pheromone();
void field_put(unsigned int x, unsigned int y, char value);
char field_get(unsigned int x, unsigned int y);
char field_get_neighbor_status(unsigned int x, unsigned int y, unsigned char dir, unsigned char loc_dir);
unsigned char field_loc_dir_to_northern_loc_dir(unsigned char dir, unsigned char loc_dir);
void field_show();
void field_move(unsigned int old_x, unsigned int old_y, unsigned int next_x, unsigned int next_y);

void ant_init();
void ant_init_ant(unsigned int ant_id, unsigned int x, unsigned int y, char force);
void ant_work_1turn(unsigned int ant_id);
void ant_get_next_dir(unsigned int ant_id);
void ant_walk(unsigned int ant_id);
unsigned char ant_is_other_pheromone(unsigned int ant_id, char field_status, unsigned int x, unsigned int y);
unsigned char ant_loc_dir_to_dir(unsigned int ant_id, unsigned char loc_dir);
void ant_show_pheromone_map();

unsigned int field_map_width;
unsigned int field_map_height;
char field_map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH] = {{ 0 }};
unsigned int field_ref_x;
unsigned int field_ref_y;

struct ant_status ant[MAX_ANT_NUM];
int ant_pheromone_map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];

int main(void)
{
	char command[MAX_COMMAND_STR];

	field_init(MAX_MAP_WIDTH, MAX_MAP_HEIGHT);
	ant_init();

	ant_init_ant(0, ANT_INIT_X, ANT_INIT_Y, ANT_DEFAULT_FORCE);
	ant_init_ant(1, ANT2_INIT_X, ANT2_INIT_Y, ANT_DEFAULT_FORCE);

	field_show();
	printf("\n> ");
	scanf("%s", command);

	while (1) {
		field_do_1turn(2);
		field_show();
#if DEBUG == 1
		printf("[DEBUG] ant pheromone map\n");
		ant_show_pheromone_map();
#endif

		printf("\n> ");
		scanf("%s", command);
	}

	return 0;
}

void field_init(unsigned int width, unsigned int height)
{
	field_map_width = width;
	field_map_height = height;
}

void field_do_1turn(unsigned int num_ants)
{
	unsigned int i;

	for (i = 0; i < num_ants; i++) {
		ant_work_1turn(i);
	}
	field_disperse_pheromone();
}

void field_disperse_pheromone()
{
	unsigned int i, j;

	for (i = 0; i < field_map_height; i++) {
		for (j = 0; j < field_map_width; j++) {
			if ((0 < field_map[i][j]) && (field_map[i][j] <= MAX_PHEROMONE_FORCE)) {
				field_map[i][j]--;
			}
		}
	}
}

void field_put(unsigned int x, unsigned int y, char value)
{
	field_map[y][x] = value;
}

char field_get(unsigned int x, unsigned int y)
{
	return field_map[y][x];
}

char field_get_neighbor_status(unsigned int x, unsigned int y, unsigned char dir, unsigned char loc_dir)
{
	unsigned char northern_loc_dir = field_loc_dir_to_northern_loc_dir(dir, loc_dir);
	char field_status = EXE_STATUS_ERROR;

	switch (northern_loc_dir) {
	case FRONT:
		if (y > 0) {
			field_ref_x = x;
			field_ref_y = y - 1;
			field_status = field_get(field_ref_x, field_ref_y);
		}
		break;

	case BACK:
		if (y < field_map_height - 1) {
			field_ref_x = x;
			field_ref_y = y + 1;
			field_status = field_get(field_ref_x, field_ref_y);
		}
		break;

	case LEFT:
		if (x > 0) {
			field_ref_x = x - 1;
			field_ref_y = y;
			field_status = field_get(field_ref_x, field_ref_y);
		}
		break;

	case RIGHT:
		if (x < field_map_width - 1) {
			field_ref_x = x + 1;
			field_ref_y = y;
			field_status = field_get(field_ref_x, field_ref_y);
		}
		break;
	}

	return field_status;
}

unsigned char field_loc_dir_to_northern_loc_dir(unsigned char dir, unsigned char loc_dir)
{
	unsigned char northern_loc_dir = FRONT;

	switch (dir) {
	case NORTH:
		northern_loc_dir = loc_dir;
		break;

	case EAST:
		switch (loc_dir) {
		case FRONT:
			northern_loc_dir = RIGHT;
			break;
		case RIGHT:
			northern_loc_dir = BACK;
			break;
		case BACK:
			northern_loc_dir = LEFT;
			break;
		case LEFT:
			northern_loc_dir = FRONT;
			break;
		}
		break;

	case SOUTH:
		switch (loc_dir) {
		case FRONT:
			northern_loc_dir = BACK;
			break;
		case RIGHT:
			northern_loc_dir = LEFT;
			break;
		case BACK:
			northern_loc_dir = FRONT;
			break;
		case LEFT:
			northern_loc_dir = RIGHT;
			break;
		}
		break;

	case WEST:
		switch (loc_dir) {
		case FRONT:
			northern_loc_dir = LEFT;
			break;
		case RIGHT:
			northern_loc_dir = FRONT;
			break;
		case BACK:
			northern_loc_dir = RIGHT;
			break;
		case LEFT:
			northern_loc_dir = BACK;
			break;
		}
		break;
	}

	return northern_loc_dir;
}

void field_show()
{
	unsigned i, j;

	for (i = 0; i < field_map_height; i++) {
		for (j = 0; j < field_map_width; j++) {
			switch (field_map[i][j]) {
			case '*':
				putchar(field_map[i][j]);
				break;
			case 0:
				putchar(' ');
				break;
			default:
				putchar((char)(field_map[i][j] + '0'));
			}
		}
		putchar('\n');
	}
}

void field_move(unsigned int old_x, unsigned int old_y, unsigned int next_x, unsigned int next_y)
{
	char tmp;

	tmp = field_map[old_y][old_x];
	field_map[next_y][next_x] = tmp;
	field_map[old_y][old_x] = 0;
}

void ant_init()
{
	unsigned int i, j;

	for (i = 0; i < MAX_MAP_HEIGHT; i++) {
		for (j = 0; j < MAX_MAP_WIDTH; j++) {
			ant_pheromone_map[i][j] = -1;
		}
	}
}

void ant_init_ant(unsigned int ant_id, unsigned int x, unsigned int y, char force)
{
	ant[ant_id].now_x = x;
	ant[ant_id].now_y = y;
	ant[ant_id].pheromone_force = force;
	field_put(x, y, ANT_ASCII);
	srand((unsigned int)time(0));
}

void ant_work_1turn(unsigned int ant_id)
{
	ant_get_next_dir(ant_id);
	ant_walk(ant_id);
}

void ant_get_next_dir(unsigned int ant_id)
{
	char field_status;
	char found_pheromone = 0;
	char found_loc_dir;

	field_status = field_get_neighbor_status(ant[ant_id].now_x, ant[ant_id].now_y, ant[ant_id].now_dir, RIGHT);
	if (ant_is_other_pheromone(ant_id, field_status, field_ref_x, field_ref_y) == 1) {
		found_pheromone = field_status;
		found_loc_dir = RIGHT;
	}
	field_status = field_get_neighbor_status(ant[ant_id].now_x, ant[ant_id].now_y, ant[ant_id].now_dir, FRONT);
	if ((ant_is_other_pheromone(ant_id, field_status, field_ref_x, field_ref_y) == 1) && (found_pheromone < field_status)) {
		found_pheromone = field_status;
		found_loc_dir = FRONT;
	}
	field_status = field_get_neighbor_status(ant[ant_id].now_x, ant[ant_id].now_y, ant[ant_id].now_dir, LEFT);
	if ((ant_is_other_pheromone(ant_id, field_status, field_ref_x, field_ref_y) == 1) && (found_pheromone < field_status)) {
		found_pheromone = field_status;
		found_loc_dir = LEFT;
	}

	if (found_pheromone <= 0) {
#if DEBUG == 1
		printf("[DEBUG] ant is random walk.\n");
#endif
		while (1) {
			found_loc_dir = (unsigned char)((rand() / ((double)RAND_MAX + 1.0)) * (MAX_LOCAL_DIRECTIONS - 1));
			field_status = field_get_neighbor_status(ant[ant_id].now_x, ant[ant_id].now_y, ant[ant_id].now_dir, found_loc_dir);
			if ((field_status != EXE_STATUS_ERROR) && (field_status != ANT_ASCII)) {
				break;
			}
		}
	}

	ant[ant_id].now_dir = ant_loc_dir_to_dir(ant_id, found_loc_dir);
}

void ant_walk(unsigned int ant_id)
{
	unsigned int old_x = ant[ant_id].now_x;
	unsigned int old_y = ant[ant_id].now_y;

	switch (ant[ant_id].now_dir) {
	case NORTH:
		ant[ant_id].now_y--;
		break;
	case SOUTH:
		ant[ant_id].now_y++;
		break;
	case WEST:
		ant[ant_id].now_x--;
		break;
	case EAST:
		ant[ant_id].now_x++;
		break;
	}
	field_move(old_x, old_y, ant[ant_id].now_x, ant[ant_id].now_y);
	field_put(old_x, old_y, ant[ant_id].pheromone_force);
	ant_pheromone_map[old_y][old_x] = ant_id;
}

unsigned char ant_is_other_pheromone(unsigned int ant_id, char field_status, unsigned int x, unsigned int y)
{
	if ((0 < field_status) && (field_status <= MAX_PHEROMONE_FORCE)) {
#if DEBUG == 1
		printf("[DEBUG] is_pheromone\n");
#endif
		if (ant_pheromone_map[y][x] != (int)ant_id) {
#if DEBUG == 1
			printf("[DEBUG] is_own_pheromone\n");
#endif
			return 1;
		}
	}
	return 0;
}

unsigned char ant_loc_dir_to_dir(unsigned int ant_id, unsigned char loc_dir)
{
	unsigned char dir = NORTH;

	switch (ant[ant_id].now_dir) {
	case NORTH:
		switch (loc_dir) {
		case FRONT:
			dir = NORTH;
			break;
		case RIGHT:
			dir = EAST;
			break;
		case BACK:
			dir = SOUTH;
			break;
		case LEFT:
			dir = WEST;
			break;
		}
		break;

	case EAST:
		switch (loc_dir) {
		case FRONT:
			dir = EAST;
			break;
		case RIGHT:
			dir = SOUTH;
			break;
		case BACK:
			dir = WEST;
			break;
		case LEFT:
			dir = NORTH;
			break;
		}
		break;

	case SOUTH:
		switch (loc_dir) {
		case FRONT:
			dir = SOUTH;
			break;
		case RIGHT:
			dir = WEST;
			break;
		case BACK:
			dir = NORTH;
			break;
		case LEFT:
			dir = EAST;
			break;
		}
		break;

	case WEST:
		switch (loc_dir) {
		case FRONT:
			dir = WEST;
			break;
		case RIGHT:
			dir = NORTH;
			break;
		case BACK:
			dir = EAST;
			break;
		case LEFT:
			dir = SOUTH;
			break;
		}
		break;
	}

	return dir;
}

void ant_show_pheromone_map()
{
	unsigned int i, j;

	for (i = 0; i < MAX_MAP_HEIGHT; i++) {
		for (j = 0; j < MAX_MAP_WIDTH; j++) {
			switch (ant_pheromone_map[i][j]) {
			case -1:
				printf("   ");
				break;
			default:
				printf("%02d ", ant_pheromone_map[i][j]);
				break;
			}
		}
		putchar('\n');
	}
}
