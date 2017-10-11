#include "functions.h"
//mode -> receiver/transmiter


//Writing full 5 bytes
#define TRAME_SIZE 5

//READING BYTE BY BYTE

int send_SET(int fileDescriptor){
	char trame[5];
	trame[0] = 0x7E;
	trame[1] = 0x03;
	trame[2] = 0x03;
	trame[3] = trame[1] ^trame[2]; //XOR OF 1 AND 2
	trame[4] = 0x7E;
	int err = write(fileDescriptor, trame, TRAME_SIZE);
	if(err == -1){
		printf("ERROR ON WRITE(LLOPEN\n");
		return err;
	}
	else{
		printf("SET TRAME WRITTEN\n");
		return 0;
	}
}

int send_UA(int fileDescriptor){
	char trame[5];
	trame[0] = 0x7E;
	trame[1] = 0x03;
	trame[2] = 0x07;
	trame[3] = trame[1] ^ trame[2]; //XOR OF 1 AND 2
	trame[4] = 0x7E;
	int err = write(fileDescriptor, trame, TRAME_SIZE);
	if(err == -1){
		printf("ERROR ON WRITE(LLOPEN\n");
		return err;
	}
	else{
		printf("UA TRAME WRITTEN\n");
		return 0;
	}
}

int readByte(int fileDescriptor, char comparison){
	char r;
	while(1){
			int err = read(fileDescriptor, &r, 1);
			if(err == -1){
				printf("ERROR ON READ(LLOPEN)\n");
				return err;
			}
			else{
				printf("LLOPEN READ ACCOMPLISHED!\n");
			}

			if(r == comparison){
				printf("Matched Comparison!\n");
				break;
			}
			else{
				printf("r: 0X%x\n", r);
			}
		}

	return 0;
}


int llopen(int fileDescriptor, int mode){
	
	//int count = 0;
	if(mode == 0){ //transmitter
		printf("WRITE\n");
		int test = send_SET(fileDescriptor);
		if(test == -1){
			return test;
		}
		printf("READ INITIAL FLAG\n");
		test = readByte(fileDescriptor, 0x7E);
		if(test == -1){
			return test;			
			
		}
		printf("READ ADRESS FIELD\n");
		test = readByte(fileDescriptor, 0x03);
		if(test == -1){
			return test;				
		}

		printf("READ CONTROL CAMP SET\n");
		test = readByte(fileDescriptor, 0x07);
		if(test == -1){
			return test;				
		}


		printf("READ PROTECTION CAMP(BCC)\n");
		test = readByte(fileDescriptor, 0x04);
		if(test == -1){
			return test;				
		}

		printf("READ FINAL FLAG\n");
		test = readByte(fileDescriptor, 0x7E);
		if(test == -1){
			return test;				
		}
	}

	else if(mode == 1){//receiver
		//char count;
		printf("READ\n");
		int test;
		printf("READ INITIAL FLAG\n");
		test = readByte(fileDescriptor, 0x7E);
		if(test == -1){
			return test;			
			
		}
		printf("READ ADRESS FIELD\n");
		test = readByte(fileDescriptor, 0x03);
		if(test == -1){
			return test;				
		}

		printf("READ CONTROL CAMP SET\n");
		test = readByte(fileDescriptor, 0x03);
		if(test == -1){
			return test;				
		}


		printf("READ PROTECTION CAMP(BCC)\n");
		test = readByte(fileDescriptor, 0x00);
		if(test == -1){
			return test;				
		}

		printf("READ FINAL FLAG\n");
		test = readByte(fileDescriptor, 0x7E);
		if(test == -1){
			return test;				
		}

		test = send_UA(fileDescriptor);
		if(test == -1){
			return test;
		}
		
	}

	else{
		printf("LLOPEN ARGUMENT ERRORS! MODE INCORRECT\n");
	 	return -1;
	}
	
	sleep(5);
	
	return 0;
}
