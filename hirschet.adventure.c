#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

const int num_rooms = 7;

struct Room {
	char* name;
	char** connections;
	int num_connections;
	char* type;
};

//Get the newest instance of room game files:
char* getNewestRoomsDir(){
	//Code retrieved from lecture notes
	int newestDirTime = -1; // Modified timestamp of newest subdir examined
        char targetDirPrefix[32] = "hirschet.rooms."; // Prefix we're looking for
        char* newestDirName; // Holds the name of the newest dir that contains prefix
	newestDirName = calloc(128, sizeof(char));

        DIR* dirToCheck; // Holds the directory we're starting in
        struct dirent *fileInDir; // Holds the current subdir of the starting dir
        struct stat dirAttributes; // Holds information we've gained about subdir

        dirToCheck = opendir("."); // Open up the directory this program was run in

        if (dirToCheck > 0) // Make sure the current directory could be opened
        {
                while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
                {
                        if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
                        {
                        //        printf("Found the prefex: %s\n", fileInDir->d_name);
                                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

                                if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
                                {
                                        newestDirTime = (int)dirAttributes.st_mtime;
                                        memset(newestDirName, '\0', sizeof(newestDirName));
                                        strcpy(newestDirName, fileInDir->d_name);
                                //        printf("Newer subdir: %s, new time: %d\n",
                                //        fileInDir->d_name, newestDirTime);
                                }
                        }
                }
        }
        closedir(dirToCheck); // Close the directory we opened
	return newestDirName;
}

void playTheGodForsakenGame(struct Room* rooms){
	struct Room* currRoom;
	int i, j;
	char* input;
	int badInput, pathLen;
	struct Room** path;

	//get the start room:
	for (i = 0; i < num_rooms; i++){
		if (strcmp(rooms[i].type, "START_ROOM") == 0) currRoom = &rooms[i];
	}
	//Begin the game:
	input = malloc(sizeof(char) * 256);
	path = malloc(sizeof(struct Room*) * 256);
	pathLen = 0;
	do {
		
		printf("CURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS:", currRoom->name);
		for (i = 0; i < currRoom->num_connections; i++){
			printf(" %s", currRoom->connections[i]);
			if (i < currRoom->num_connections - 1) printf(",");
		}
		printf(".\nWHERE TO? >");
		fgets(input, 255, stdin);
		input[strlen(input) - 1] = '\0';

		badInput = 1;
		for (i = 0; i < num_rooms; i++){
			if (strcmp(input, rooms[i].name) == 0){
				printf("\n");
				//update room
				badInput = 0;
				currRoom = &rooms[i];

				//update path
				path[pathLen] = currRoom;
				pathLen++;
				break;
			}	
		}
		if (strcmp(input, "time") == 0){
			printf("\nit's time for some RAM RANCH!\n\n");
		}
		else if (badInput == 1){
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}

	} while(strcmp(currRoom->type, "END_ROOM") != 0);
	free(input);

	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", pathLen);
	for (i = 0; i < pathLen; i++){
		printf("%s\n", path[i]->name);
	}

	free(path);
	
	return;
}

int main(){
	char* roomsDir;
	FILE* fp;
	struct Room* rooms;

	roomsDir = getNewestRoomsDir();	

	rooms = malloc(sizeof(struct Room) * num_rooms);	
	
	printf("Rooms directory is: %s\n", roomsDir);

	int i, j;
	DIR* dirToCheck;
	struct dirent* fileInDir;
	char* fileText;
	char* fullFilePath;

	//Construct the rooms array:
	fileText = malloc(sizeof(char) * 128);
	fullFilePath = malloc(sizeof(char) * 256);
	dirToCheck = opendir(roomsDir);
	readdir(dirToCheck);
	readdir(dirToCheck);
	if (dirToCheck > 0){
		for (i = 0; i < num_rooms; i++){
			
			rooms[i].name = malloc(sizeof(char) * 32);
			rooms[i].type = malloc(sizeof(char) * 32);
			rooms[i].num_connections = 0;
			rooms[i].connections = malloc(sizeof(char *) * 6);

			fileInDir = readdir(dirToCheck);

			strcpy(fullFilePath, roomsDir);
			strcat(fullFilePath, "/");
			strcat(fullFilePath, fileInDir->d_name);

			fp = fopen(fullFilePath, "r");

			//get name:	
			fgets(fileText, 127, fp);
			fileText[strlen(fileText) - 1] = '\0';
			strcpy(rooms[i].name, &fileText[11]);
		//	printf("ROOM NAME: '%s'\n", rooms[i].name);			

			//get connections (as names):
			fgets(fileText, 127, fp);
			while(fileText[0] == 'C'){
				rooms[i].connections[rooms[i].num_connections] = malloc(sizeof(char) * 32);
				fileText[strlen(fileText) - 1] = '\0';
				strcpy(rooms[i].connections[rooms[i].num_connections], &fileText[14]);
			//	printf("CONNECTION %d: '%s'\n", rooms[i].num_connections + 1, rooms[i].connections[rooms[i].num_connections]);
				rooms[i].num_connections++;

				fgets(fileText, 127, fp);	
			}

			//get type:
			fileText[strlen(fileText) - 1] = '\0';
			strcpy(rooms[i].type, &fileText[11]);
		//	printf("ROOM TYPE: '%s'\n\n", rooms[i].type);

			fclose(fp);
		}
	}

	//play the game:
	playTheGodForsakenGame(rooms);

	free(fullFilePath);
	free(fileText);
	free(rooms);
	free(roomsDir);

	return 0;
}
