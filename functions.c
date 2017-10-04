#include "functions.h"
//mode -> receiver/transmiter
char SET[8] = "00000011";
char UA[8] = "00000111";
int llopen(int fileDescriptor, int mode){
	if(mode == 0){ //transmitter
		char test[8];
		int err = write(fileDescriptor, SET, 8);
		if(err == -1){
			printf("ERROR ON WRITE(LLOPEN\n");
			return err;
		}
		else{
			printf("SET WRITTEN\n");
		}
		err = read(fileDescriptor, test, 8);
		if(err == -1){
			printf("READ ON LLOPEN TRANSMITER\n");
		}
		if(strcmp(test, UA)){
			printf("UA RECEIVED\n");
		}
		else{
			printf("%s\n", test);
			printf("LLOPEN TRANSMITER UA MISMATCHED\n");
		}
	}

	else if(mode == 1){//receiver
		char test[8];
		int err = read(fileDescriptor, test, 8);
		if(err == -1){
			printf("ERROR ON READ(LLOPEN\n");
			return err;
		}
		int diff = strcmp(test, SET);
		printf("diff: %d\n", diff);
		else if(diff == 0){
			printf("LLOPEN RECEIVER SET READ\n");
			err = write(fileDescriptor, UA, 8);
			if(err == -1){
				printf("LLOPEN RECEIVER ERROR WRITE UA\n");
			}
			else{
				printf("LLOPEN RECEIVER UA WRITTEN\n");
			}
		}

		else{
			printf("%s\n", test);
			printf("LLOPEN RECEIVER SET MISMATCH\n");
		}
	}


	else{
		printf("LLOPEN ARGUMENT ERRORS!\n");
	 	return -1;
	}
	
}
