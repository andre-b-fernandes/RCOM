#include "functions.h"
//mode -> receiver/transmiter
char SET[8] = "00000011";
char UA[8] = "00000111";


int llopen(int fileDescriptor, int mode){
	char test[8];
	if(mode == 0){ //transmitter
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
		int comp = strcmp(test,UA);
		if(comp == 0){
			printf("UA RECEIVED\n");
		}
		else{
			printf("%s\n", test);
			printf("LLOPEN TRANSMITER UA MISMATCHED\n");
		}
	}

	else if(mode == 1){//receiver
		int err = read(fileDescriptor, test, 8);
		if(err == -1){
			printf("ERROR ON READ(LLOPEN\n");
			return err;
		}
		else{
			printf("LLOPEN RECEIVER. READ ACCOMPLISHED!\n");
		}
		int diff = strcmp(test, SET);

		if(diff == 0){
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
			printf("Test: %s with %d bytes\n", test, err);
			printf("LLOPEN RECEIVER SET MISMATCH\n");
		}
	}


	else{
		printf("LLOPEN ARGUMENT ERRORS! MODE INCORRECT\n");
	 	return -1;
	}
	return 0;
}
