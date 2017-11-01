#include "functions.h"
//mode -> receiver/transmiter



static int state;
static bool stop = false;
static int count=0;
static int FD;

void alarmHandler(int sig){
 printf("Attempt number %d \n", count + 1);
 state = 5;
 count ++;
 unblockReadPortSettings(FD);
}

int send_SET(int fileDescriptor){
	unsigned char trame[TRAME_SIZE];
	trame[0] = FLAG;
	trame[1] = A;
	trame[2] = C_SET;
	trame[3] = trame[1] ^trame[2]; //XOR OF 1 AND 2
	trame[4] = FLAG;
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
	unsigned char trame[5];
	trame[0] = FLAG;
	trame[1] = A;
	trame[2] = C_UA;
	trame[3] = trame[1] ^ trame[2]; //XOR OF 1 AND 2
	trame[4] = FLAG;
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

int readByteWrite(int fd, unsigned char *c){
	int h = read(fd,c,1);
	if(h == -1){
		printf("LLOPEN(), TRANSMITTER MODE ERROR ON READ!\n");
	}
	printf("C: %x\n", *c);
  if(h == 0){
    defaultPortSettings(fd);
  }
	return h;
}

int readByteRead(int fd, unsigned char *c){
	int h = read(fd, c,1);
	if(h== -1){
		printf("LLOPEN(), RECEIVER MODE ERROR ON READ!\n");
	}
	printf("READ C: %x\n", *c);
  if(h == 0){
    defaultPortSettings(fd);
  }
	return h;
}

int unblockReadPortSettings(int fd){
  struct termios newtio;
  bzero(&newtio, sizeof(newtio));
  if (tcgetattr(fd,&newtio) == -1) {
    perror("tcgetattr");
    exit(-1);
  }
  newtio.c_cc[VMIN] = 0;

  if( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  return 0;
}

int defaultPortSettings(int fd){
  struct termios newtio;
  bzero(&newtio, sizeof(newtio));
  if (tcgetattr(fd,&newtio) == -1) {
    perror("tcgetattr");
    exit(-1);
  }
  newtio.c_cc[VMIN] = 1;

  if( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  return 0;
}

int stateMachineWrite(unsigned char c,int fileDescriptor){
  printf("State: %d\n", state );
	int test;
	switch(state){
		case 5:
		  test = send_SET(fileDescriptor);
			if(test == -1){
				printf("stateMachineWrite CASE 5 ERROR ON SEND_SET!\n");
				return test;
		  }
			state = 0;
			break;
		case 0:
			test = readByteWrite(fileDescriptor, &c);
			if(test == -1){
				printf("stateMachineWrite CASE 0 ERROR ON READ\n");
			}
      if(c == FLAG)
			{
				printf("stateMachineWrite CASE 0 FLAG READ!\n");
				state =1;
			}
      else{
        printf("READ2 C: %x\n", c);
      }
			break;
    case 1:
			test = readByteWrite(fileDescriptor, &c);
			if(test == -1){
				printf("stateMachineWrite CASE 1 ERROR ON READ\n");
				return test;
			}
      if(c!= FLAG)
			{
				printf("stateMachineWrite CASE 1 RETURN TO CASE 0\n");
				state =0;
			}
			if(c == A)
			{
				printf("stateMachineWrite CASE 1 A READ!\n");
				state = 2;
			}
       break;
    case 2:
			test = readByteWrite(fileDescriptor, &c);
			if(test == -1){
				printf("stateMachineWrite CASE 2 ERROR ON READ\n");
				return test;
			}
     	if(c!= FLAG){
				printf("stateMachineWrite CASE 2 RETURN TO CASE 0\n");
				state =0;
			}

			if(c == FLAG){
				printf("stateMachineWrite CASE 2 RETURN TO CASE 1\n");
				state =1;
			}

    	if(c == C_UA){
				printf("stateMachineWrite CASE 2 C_UA READ!\n");
        state =3;
			}
      break;
     case 3:
		 		test = readByteWrite(fileDescriptor, &c);
		 		if(test == -1){
			 		printf("stateMachineWrite CASE 3 ERROR ON READ\n");
			 		return test;
		 		}
        if(c!= FLAG)
				{
					printf("stateMachineWrite CASE 3 RETURN TO STATE 0\n");
					state =0;
				}
        if(c == FLAG)
				{
					printf("stateMachineWrite CASE 3 RETURN TO STATE 2\n");
					state =1;
				}
        if( c == (C_UA^A))
				{
					printf("stateMachineWrite CASE 3 C_UA^A READ!\n");
					state = 4;
				}
        break;
      case 4:
				test = readByteWrite(fileDescriptor, &c);
				if(test == -1){
					printf("stateMachineWrite CASE 4 ERROR ON READ\n");
					return test;
				}
        if(c != FLAG){
					printf("stateMachineWrite CASE 4 RETURN TO CASE 0\n");
					state =0;
				}
        if(c== FLAG){
           stop = true;
					 alarm(0);
         	printf("llopen(): received UA\n");
        }
        break;
       default:
         break;
	}

	return 0;
}



int stateMachineRead(unsigned char c,int fileDescriptor){
  printf("State: %d\n", state );
	int test;
	switch(state){
		case 5:
			;
			printf("StateMachineRead: SEND_UA\n");
			test = send_UA(fileDescriptor);
			if(test == -1){
				printf("StateMachineRead: SEND_UA ERRORS\n");
				return test;
		  }
			stop = true;
			break;
		case 0:
			test = readByteRead(fileDescriptor, &c);
			if(test == -1){
				printf("StateMachineRead CASE 0 ERROR ON READ\n");
			}
      if(c == FLAG)
			{
				printf("StateMachineRead CASE 0 FLAG READ!\n");
				state =1;
			}
			break;
    case 1:
			test = readByteRead(fileDescriptor, &c);
			if(test == -1){
				printf("StateMachineRead CASE 1 ERROR ON READ\n");
			}
      if(c != FLAG)
			{
				printf("StateMachineRead CASE 1, RETURNING TO CASE 0!\n");
				state = 0;
			}
			if(c == A)
			{
				printf("StateMachineRead CASE 1, A READ!\n");
				state = 2;
			}
      break;
    case 2:
			test = readByteRead(fileDescriptor, &c);
			if(test == -1){
				printf("StateMachineRead CASE 2 ERROR ON READ\n");
			}
     	if(c!= FLAG)
			{
				printf("StateMachineRead CASE 2, RETURNING TO CASE 0!\n");
				state =0;
			}

			if(c == FLAG){
				printf("StateMachineRead CASE 2, RETURNING TO CASE 1!\n");
				state = 1;
			}

    	if(c == C_SET){
					printf("StateMachineRead CASE 2, C_SET READ!\n");
				  state =3;
			}
      break;
     case 3:
		 	test = readByteRead(fileDescriptor, &c);
			if(test == -1){
			 printf("StateMachineRead CASE 3 ERROR ON READ\n");
		 	}
      if(c!= FLAG)
			{
				printf("StateMachineRead CASE 3, RETURNING TO CASE 0\n");
				state = 0;
			}
      if(c == FLAG)
			{
				printf("StateMachineRead CASE 3, RETURNING TO CASE 1!\n");
			 	state =1;
			}
      if( c == (C_SET^A))
			{
				printf("StateMachineRead CASE 3, C_SET^A read!\n");
				state = 4;
			}
      break;

    case 4:
			test = readByteRead(fileDescriptor, &c);
			if(test == -1){
		 		printf("StateMachineRead CASE 4 ERROR ON READ\n");
			}
      if(c != FLAG)
			{
				printf("StateMachineRead CASE 4, RETURNING TO CASE 0!\n");
				state =0;
			}
      if(c == FLAG){
         state = 5;
         printf("StateMachineRead CASE 4: received SET\n");
       }
        break;
       default:
         break;
	}

	return 0;
}



int llopen(int fileDescriptor, int mode){
  FD = fileDescriptor;
	signal(SIGALRM, alarmHandler);
	unsigned char c = 0;
	if(mode == 0){ //transmitter
		state = 5;
		printf("WRITE\n");
		do {
			stateMachineWrite(c, fileDescriptor);
			alarm(3);
		} while(!stop && count < 3);
	}

	else if(mode == 1){//receiver
		state = 0;
		printf("READ\n");
		do {
			stateMachineRead(c,fileDescriptor);
		} while(!stop);
	}

	else{
		printf("LLOPEN ARGUMENT ERRORS! MODE INCORRECT\n");
	 	return -1;
	}
  printf("END\n");
	sleep(5);

	return 0;
}
