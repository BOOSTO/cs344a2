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
#include<pthread.h>
#include<time.h>

const int num_rooms = 7;

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

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

void* printTime(void* arg){
	pthread_mutex_lock(&myMutex);

	FILE* fp;
	char* timeStr;	

	//Invent the very concept of time:
	time_t currentTime;
	struct tm *localTime;
	//current time:
	currentTime = time(NULL);
	localTime = localtime(&currentTime);
	//make time a formatted string:
	timeStr = malloc(sizeof(char) * 64);
	strftime(timeStr, 64, "%l:%M%P, %A, %B %e, %Y", localTime);

	//printf("\n\n	%s\n\n", timeStr);
	
	//Write time to file:
	fp = fopen("currentTime.txt", "w");
	fprintf(fp, "%s\n", timeStr);
	fclose(fp);
	
	//free dynamic memory:
	free(timeStr);	

	pthread_mutex_unlock(&myMutex);
	return;
}

void playTheGodForsakenGame(struct Room* rooms){
	struct Room* currRoom;
	int i, j;
	char* input;
	int badInput, pathLen;
	struct Room** path;
	pthread_t timeThread;
	FILE* ftime;
	char* timeText;
	
	//get the start room:
	for (i = 0; i < num_rooms; i++){
		if (strcmp(rooms[i].type, "START_ROOM") == 0) currRoom = &rooms[i];
	}
	//Begin the game:
	input = malloc(sizeof(char) * 256);
	path = malloc(sizeof(struct Room*) * 256);
	pathLen = 0;
	do{
		//Game Dialog:	
		printf("CURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS:", currRoom->name);
		for (i = 0; i < currRoom->num_connections; i++){
			printf(" %s", currRoom->connections[i]);
			if (i < currRoom->num_connections - 1) printf(",");
		}
		printf(".\nWHERE TO? >");
		//Get input:
		fgets(input, 255, stdin);
		input[strlen(input) - 1] = '\0';
		
		//Validate input:
		badInput = 1;
		//Check if it's a room name
		for (i = 0; i < num_rooms; i++){
			if (strcmp(input, rooms[i].name) == 0 && strcmp(input, currRoom->name) != 0){
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
		//check if it's time
		if (strcmp(input, "time") == 0){
			pthread_create(&timeThread, NULL, printTime, NULL);
			pthread_join(timeThread, NULL);
			
			//open the time file
			ftime = fopen("currentTime.txt", "r");
			
			//Read contents into string:
			timeText = malloc(sizeof(char) * 64);
			fgets(timeText, 64, ftime);
			fclose(ftime);

			//print contents to screen:
			printf("\n%s\n", timeText);
			//free memory:
			free(timeText);
		}
		//check if they fucked up
		else if (badInput == 1){
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}
	} while(strcmp(currRoom->type, "END_ROOM") != 0); //repeat until the end room is reached
	free(input);

	//Victory speech
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", pathLen);
	//Print the path
	for (i = 0; i < pathLen; i++){
		printf("%s\n", path[i]->name);
	}
	free(path); //free path since it's done being used
	
	return;
}

int main(){
	char* roomsDir;
	FILE* fp;
	struct Room* rooms;

	//get the latest rooms directory
	roomsDir = getNewestRoomsDir();	

	//init rooms array
	rooms = malloc(sizeof(struct Room) * num_rooms);	
	
	//printf("Rooms directory is: %s\n", roomsDir);

	int i, j;
	DIR* dirToCheck;
	struct dirent* fileInDir;
	char* fileText;
	char* fullFilePath;

	//Construct the rooms array:
	fileText = malloc(sizeof(char) * 128);
	fullFilePath = malloc(sizeof(char) * 256);
	
	//Access directory:
	dirToCheck = opendir(roomsDir);
	readdir(dirToCheck); //ignore .
	readdir(dirToCheck); //ignore ..
	if (dirToCheck > 0){
		for (i = 0; i < num_rooms; i++){
			//initialize memory:		
			rooms[i].name = malloc(sizeof(char) * 32);
			rooms[i].type = malloc(sizeof(char) * 32);
			rooms[i].num_connections = 0;
			rooms[i].connections = malloc(sizeof(char *) * 6);
			
			//get rooms file
			fileInDir = readdir(dirToCheck);
			
			//construct full filepath from current directory
			strcpy(fullFilePath, roomsDir);
			strcat(fullFilePath, "/");
			strcat(fullFilePath, fileInDir->d_name);
			
			//open it
			fp = fopen(fullFilePath, "r");

			//get name:	
			fgets(fileText, 127, fp);
			fileText[strlen(fileText) - 1] = '\0';
			strcpy(rooms[i].name, &fileText[11]);
		//	printf("ROOM NAME: '%s'\n", rooms[i].name);			

			//get connections (as names):
			fgets(fileText, 127, fp);
			//repeat this process as long as there are more connections
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
	closedir(dirToCheck);

	//play the game:
	playTheGodForsakenGame(rooms);

	
	//clean the mem:
	for (i = 0; i < num_rooms; i++){
		free(rooms[i].name);
		free(rooms[i].type);
		for (j = 0; j < rooms[i].num_connections; j++){
			free(rooms[i].connections[j]);
		}
		free(rooms[i].connections);
	}
	free(fullFilePath);
	free(fileText);
	free(rooms);
	free(roomsDir);
	pthread_mutex_destroy(&myMutex);

	return 0;
}
