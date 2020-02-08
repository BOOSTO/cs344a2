#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

const int num_room_names = 10;
const int num_rooms = 7;

struct Room {
	char* room_name;
	char* room_type;
	struct Room** connections;
	int num_connections;
};

//check rooms array to see if all rooms have < 3 connections
int is_graph_complete(struct Room rooms[num_rooms]){
	int i;
	for (i = 0; i < num_rooms; i++){
		if (rooms[i].num_connections < 3) return 0;
	}
	return 1;
}

//Check if a connection between two rooms is already present
int connection_exists(struct Room rooms[num_rooms], int index1, int index2){
	int i;
	for (i = 0; i < rooms[index1].num_connections; i++){
		if (strcmp(rooms[index1].connections[i]->room_name, rooms[index2].room_name) == 0) return 1;
	}
	return 0;
}

void add_connection(struct Room rooms[num_rooms]){
	int room1_index, room2_index;
	//get first index for connection, ensuring it has less than max connections already
	do {
		room1_index = rand() % num_rooms;
	} while (rooms[room1_index].num_connections >= 6);
	//get second index for connnection
	do {
		room2_index = rand() % num_rooms;
	} while (rooms[room2_index].num_connections >= 6 || room1_index == room2_index || connection_exists(rooms, room1_index, room2_index) == 1);

	//create connections
	//first room
	rooms[room1_index].connections[rooms[room1_index].num_connections] = &rooms[room2_index];
	rooms[room1_index].num_connections++;
	//second room
	rooms[room2_index].connections[rooms[room2_index].num_connections] = &rooms[room1_index];
        rooms[room2_index].num_connections++;
}

int main(){
	int i, j;
	int name_index, is_open, start_index, end_index;
	struct Room* rooms;
	char * room_names[num_room_names];
	room_names[0] = "Lain";
	room_names[1] = "Batou";
	room_names[2] = "Major";
	room_names[3] = "Tetsuo";
	room_names[4] = "Kaneda";
	room_names[5] = "Neo";
	room_names[6] = "PMaster";
	room_names[7] = "Hiro";
	room_names[8] = "Paladin";
	room_names[9] = "Eliasz";
	char * directory;
	char * filename;
	char directory_template[] = "./hirschet.rooms.";
	char * pidstr = malloc(sizeof(char) * 24);
	time_t t;
	FILE * fp;

	//make rooms mem:
	rooms = calloc(num_rooms, sizeof(struct Room));
	
	//make directory name:
	sprintf(pidstr, "%d", (int) getpid());
	directory = calloc(64, sizeof(char));
	strcat(directory, directory_template);
	strcat(directory, pidstr);
	free(pidstr);

	srand((unsigned) time(&t));
	
	//Create rooms
	for (i = 0; i < num_rooms; i++){
		//Assign Room Names
		//get random room name, but make sure it hasn't already been used
		do {
			is_open = 1;
			name_index = rand() % num_room_names;
			for (j = 0; j < i; j++){
				if (strcmp(room_names[name_index], rooms[j].room_name) == 0) is_open = 0;	
			}
		} while (is_open != 1);
		//set room to random name
		rooms[i].room_name = (char*) calloc(8, sizeof(char));
		strcpy(rooms[i].room_name, room_names[name_index]);
		//printf("room_%d is named %s\n", i, rooms[i].room_name);

		//Assign Default Room Type
		rooms[i].room_type = (char*) calloc(16, sizeof(char));
		strcpy(rooms[i].room_type, "MID_ROOM");

		//Alloc Connections memory
		rooms[i].connections = calloc(6, sizeof(struct Room*));
		rooms[i].num_connections = 0;
	}
	//Assign random Start and End rooms:
	//start room:
	start_index = rand() % num_rooms;
	strcpy(rooms[start_index].room_type, "START_ROOM");
	//make sure end rooms is different than start room:
	do{
		end_index = rand() % num_rooms;
	} while(start_index == end_index);
	strcpy(rooms[end_index].room_type, "END_ROOM");
	
	//Make connections:
	while(is_graph_complete(rooms) == 0){
		add_connection(rooms);
	}

	//test:
	/*for (i = 0; i < num_rooms; i++){
		printf("ROOM #%d NAME: %s TYPE: %s\n", i + 1, rooms[i].room_name, rooms[i].room_type);
		printf("--Connections:\n");
		for (j = 0; j < rooms[i].num_connections; j++){
			printf("	-%s\n", rooms[i].connections[j]->room_name);
		}
		printf("\n");
	}*/

	//Make game directory and room files:
	//game directory:
	if (mkdir(directory, 0777) == 0){
		//make roomfiles:
		filename = calloc(128, sizeof(char));
		for (i = 0; i < num_rooms; i++){
			//construct filename:
			strcpy(filename, directory);
			strcat(filename, "/");
			strcat(filename, rooms[i].room_name);
			//open filename for writing:
			fp = fopen(filename, "w+");
			//print name:
			fprintf(fp, "ROOM NAME: %s\n", rooms[i].room_name);
			//print connections:
			for (j = 0; j < rooms[i].num_connections; j++){
				fprintf(fp, "CONNECTION %d: %s\n", j + 1, rooms[i].connections[j]->room_name);
			}
			//print type:
			fprintf(fp, "ROOM TYPE: %s\n", rooms[i].room_type);
			fclose(fp);
		}
		free(filename);
	}
	//return with error code if the directory couldn't be made
	else {
		printf("there was an issue creating the directory");
		//clean mem
		for (i = 0; i < num_rooms; i++){
			free(rooms[i].room_name);
			free(rooms[i].connections);
			free(rooms[i].room_type);\
		}
		free(rooms);
		return 1;
	}
	//clean the memory
	free(directory);
	for (i = 0; i < num_rooms; i++){
		free(rooms[i].room_name);
		free(rooms[i].connections);
		free(rooms[i].room_type);
	}
	free(rooms);
	
	return 0;
}
