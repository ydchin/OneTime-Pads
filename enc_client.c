/*
* Title: enc_client.c
* Author: Yoon-orn Chin
* Date: 11/30/2020
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <netinet/in.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

/********************
* Function: error
* Parameter: msg
* Output: Error message
* Source: Provided
********************/
void error(const char* msg) {
    perror(msg);
    exit(0);
}

/********************
* Function: setupAddressStruct
* Parameter: address
* Output: Struct filled with info
* Source: Provided
********************/
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber,
    char* hostname) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        fprintf(stderr, "ENC_CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

/********************
* Function: getFileLength
* Parameter: fileName
* Output: Length of the given file
* Why use this: To retreieve the length of the file in order to send to the client. This is an easy method I found
*               online to save me a lot of time.
* Source: https://www.geeksforgeeks.org/c-program-to-count-the-number-of-characters-in-a-file/
********************/
int getFileLength(char* fileName) {

    FILE* fp = fopen(fileName, "r");

    if (fp == NULL) {
        printf("Could not open file %s", fileName);
        exit(1);
    }

    char c;
    int count = 0;

    for (c = getc(fp); c != EOF; c = getc(fp)) {
        count++;
    }

    fclose(fp);

    return count;

}

// Main
int main(int argc, char* argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    long int tempMsg, tempKey;
    char msgLen[100000];            // Store length of message
    char keyLen[100000];            // Store length of key
    char buffer[100000];            // Takes in info from server
    char message[100000];           // Store message
    char key[100000];               // Store key
    FILE* openMsg;
    FILE* openKey;

    // Check usage & args
    if (argc < 3) {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        fflush(stderr);
        exit(0);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("ENC_CLIENT: ERROR opening socket");
        exit(1);
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ENC_CLIENT: ERROR connecting");
        exit(1);
    }

    // Open message from message file
    openMsg = fopen(argv[1], "r");

    // If error...
    if (openKey == NULL) {
        error("ENC_CLIENT: Failed to open.");
        exit(1);
    }

    // Store message from message file
    fgets(message, sizeof(message), openMsg);

    // Open key from key file
    openKey = fopen(argv[2], "r");

    // If error...
    if (openKey == NULL) {
        error("ENC_CLIENT: Failed to open.");
        exit(1);
    }

    //Store key from key file
    fgets(key, sizeof(key), openKey);

    
    // Store message Length
    tempMsg = getFileLength(argv[1]);
    sprintf(msgLen, "%d", tempMsg);

    // Store key length
    tempKey = getFileLength(argv[2]);
    sprintf(keyLen, "%d", tempKey);
    
    // Check to see if key is greater than message
    if (atoi(msgLen) > atoi(keyLen)) {
        fprintf(stderr, "ENC_CLIENT: Key is too short!\n"); //Gives weird ": Success!" with error()
        fflush(stderr);
        exit(1);
    }
    


    // Send + writes message length to server
    charsWritten = send(socketFD, msgLen, strlen(msgLen), 0);

    // Checks for errors
    if (charsWritten < 0) {
        error("ENC_CLIENT: ERROR writing to socket");
        exit(1);
    }

    // Check for any excess data
    if (charsWritten < strlen(buffer)) {
        error("ENC_CLIENT: WARNING: Not all data written to socket!\n");
        exit(1);
    }

    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));

    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

    // Checks for a proper connection to ENC_SERVER
    if (strcmp(buffer, "##") != 0) {
        fprintf(stderr, "ENC_CLIENT: ERROR: Can not connect"); //Gives weird ": Success!" with error()
        fflush(stdout);
        exit(2);
    }
    
    // Checks for errors
    if (charsRead < 0) {
        error("ENC_CLIENT: ERROR reading from socket");
        exit(1);
    }
    


    // Send + writes key length to server 
    charsWritten = send(socketFD, keyLen, strlen(keyLen), 0);

    // Checks for errors
    if (charsWritten < 0) { 
        error("ENC_CLIENT: ERROR writing to socket");
        exit(1);
    }

    // Check for any excess data
    if (charsWritten < strlen(keyLen)) {  
        error("ENC_CLIENT: WARNING: Not all data written to socket!\n");
        exit(1);
    }

    // Clear buffer
    memset(buffer, '\0', sizeof(buffer)); 
    
    // Read data from the socket
    charsRead = recv(socketFD, buffer, sizeof(buffer), 0);    

    // Checks for errors
    if (charsRead < 0) {                                    
        error("ENC_CLIENT: ERROR reading from socket");
        exit(1);
    }



    // Send + writes message to server
    charsWritten = send(socketFD, message, strlen(message), 0);

    // Checks for errors
    if (charsWritten < 0) {                                                   
        error("ENC_CLIENT: ERROR writing to socket");
        exit(1);
    }

    // Check for any excess data
    if (charsWritten < strlen(message)) {  
        error("ENC_CLIENT: WARNING: Not all data written to socket!\n");
        exit(1);
    }

    // Clear buffer
    memset(buffer, '\0', sizeof(buffer)); 

    // Read data from the socket
    charsRead = recv(socketFD, buffer, sizeof(buffer), 0);   

    // Checks for errors
    if (charsRead < 0) {
        error("ENC_CLIENT: ERROR reading from socket");
        exit(1);
    }




    // Send + writes key to server
    charsWritten = send(socketFD, key, strlen(key), 0);

    // Checks for errors
    if (charsWritten < 0) {
        error("ENC_CLIENT: ERROR writing to socket");
        exit(1);
    }

    // Check for any excess data
    if (charsWritten < strlen(key)) {
        error("ENC_CLIENT: WARNING: Not all data written to socket!\n");
        exit(1);
    }

    // Clear buffer
    memset(buffer, '\0', sizeof(buffer));  
    
    // Read data from the socket
    charsRead = recv(socketFD, buffer, sizeof(buffer), 0); 

    // Checks for errors
    if (charsRead < 0) {
        error("ENC_CLIENT: ERROR reading from socket");
        exit(1);
    }




    // Send + writes encrypted data to server

    // Clear buffer
    memset(buffer, '\0', sizeof(buffer));

    // Read data from the socket
    charsRead = recv(socketFD, buffer, sizeof(buffer), 0); 

    // Checks for errors
    if (charsRead < 0) {
        error("ENC_CLIENT: ERROR reading from socket");
        exit(1);
    }

    // Print encrypted message to standard out/document
    fprintf(stdout, buffer);
    fflush(stdout);

    // Decrements length
    tempMsg = tempMsg - strlen(buffer);

    // Check for any remaining things in message before exiting
    // Loop until...
    while (1) {

        // Break if nothing in file
        if (strlen(buffer) == 0) break;

        // Breaks if there is nothing else in the message
        if (tempMsg == 0) break;

        // Otherwise...
        else {

            // Clear buffer
            memset(buffer, '\0', sizeof(buffer));

            // Read data from the socket
            charsRead = recv(socketFD, buffer, sizeof(buffer), 0);

            // Checks for errors
            if (charsRead < 0) {
                error("ENC_CLIENT: ERROR reading from socket");
                exit(1);
            }

            // Decrements length
            tempMsg = tempMsg - strlen(buffer);

            // Print encrypted message to standard out/document
            fprintf(stdout, buffer);
            fflush(stdout);
        }
    }

    // Memory Management
    memset(msgLen, '\0', sizeof(msgLen));
    memset(keyLen, '\0', sizeof(keyLen));
    memset(buffer, '\0', sizeof(buffer));
    memset(message, '\0', sizeof(message));
    memset(key, '\0', sizeof(key));

    // Close the socket
    close(socketFD);
    return 0;
}