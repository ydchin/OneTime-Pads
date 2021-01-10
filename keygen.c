/*
* Title: keygen.c
* Author: Yoon-orn Chin
* Date: 11/26/2020
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Main
int main(int argc, char* argv[]) {

	//Random 
	srand(time(0));

	//26 chars in Alphabet + 1 char ' '/Space 
	char alphArray[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	//Array used to store how long the key is going to be
	int num = atoi(argv[1]);

	//Array used to store key
	char key[num];

	//Starting index for key
	int i;

	//Grab a random character and store it into key until length
	for (i = 0; i < num; i++) {
		int randNum = rand() % 27;
		key[i] = alphArray[randNum];
	}

	//Last character of array set to NULL to stop string
	key[num] = '\0';

	//Print out key
	printf("%s\n", key);

	return 0;
} 