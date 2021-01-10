/*
* Title: enc_server.c
* Author: Yoon-orn Chin
* Date: 11/30/2020
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

/********************
* Function: error
* Parameter: msg
* Output: Error message
* Source: Provided
********************/
void error(const char* msg) {
    perror(msg);
    exit(1);
}

/********************
* Function: setupAddressStruct
* Parameter: address
* Output: Struct filled with info
* Source: Provided
********************/
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

/********************
* Function: encryptFunct
* Parameter: encryptedMsg, key, message
* Output: encryptedMsg filled with random characters
* Why use this: I used this function to ease the load off my main and so I can easily check for bad characters as well as
*               combine the message with the key in order to receieve my encrypted message. 
********************/
char* encryptFunct(char encryptedMsg[], char key[], char message[]) {

    // Temp ints to hold int values of message and key characters
    int temp, temp1;

    // Final character after mod
    char finalChar;

    // Incrementer
    int counter = 0; 

    for (int i = 0; i < strlen(message); i++) {
        
        // Breaks if end of message
        if (message[i] == '\n') break;

        if ((message[i] < 65 || message[i] > 90) && message[i] != ' ') {
            fprintf(stderr, "ENC_SERVER: Bad character found\n");
            exit(1);
        }
        // If message contains a space
        if (message[i] == ' ' && key[i] != ' ') {
            temp = 26;
            temp1 = key[i] - 'A';
            finalChar = (temp + temp1) % 27;
        }

        // If key contains a space
        else if (message[i] != ' ' && key[i] == ' ') {
            temp = message[i] - 'A';
            temp1 = 26;
            finalChar = (temp + temp1) % 27;
        }

        // If both message and key contain a space
        else if (message[i] == ' ' && key[i] == ' ') {
            temp = 26;
            temp1 = 26;
            finalChar = (temp + temp1) % 27;
        }

        // Otherwise...
        else {
            temp = message[i] - 'A';
            temp1 = key[i] - 'A';
            finalChar = (temp + temp1) % 27;
        }
        
        // Final character to store into output
        char tempResult;

        // If equals 26 (space), add space to string.
        if (finalChar == 26) {
            tempResult = ' ';
            encryptedMsg[i] = tempResult;
        }

        // Otherwise...
        else {
            tempResult = finalChar + 'A';
            encryptedMsg[i] = tempResult;
        }

        // Increment
        counter++;
    }

    // Set last value to NULL
    encryptedMsg[counter] = '\n';
    encryptedMsg[counter + 1] = '\0';

    // Return encrypted message
    return encryptedMsg;

}

// Main
int main(int argc, char* argv[]) {
    int connectionSocket, charsRead, msgLength, keyLength, childExitMethod;
    char buffer[100000];            //  Takes in info from client
    char messageBuffer[100000];     //  Holds message
    char keyBuffer[100000];         //  Holds key
    char encryptedMsg[100000];          //  Holds final encrypted message
    char clientMsg[1024];           //  Message sent back to client
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t pid;

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ENC_SERVER: ERROR opening socket");
        exit(1);
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0) {
        error("ENC_SERVER: ERROR on binding");
        exit(1);
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    // Accept a connection, blocking if one is not available until one connects
    while (1) {

        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
            (struct sockaddr*)&clientAddress,
            &sizeOfClientInfo);

        // If error on accepting connection
        if (connectionSocket < 0) {
            error("ENC_SERVER: ERROR on accept");
            exit(1);
        }

        // Fork
        pid = fork();

        switch (pid) {
            case -1:
                // Send error message if cannot fork
                error("Hull Breach! Couldn't fork!\n");
                childExitMethod = 1;
                exit(1);
            
            case 0:
                // Clear Buffer for message length
                memset(buffer, '\0', sizeof(buffer));
                
                // Read in from client's message
                charsRead = recv(connectionSocket, buffer, sizeof(buffer), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR reading from socket");
                    childExitMethod = 1;
                    exit(1);
                }

                // Store length of message into msgLength from buffer
                msgLength = atoi(buffer);

                // Send Message back to client. ## for ENC.
                strcat(clientMsg, "##");
                charsRead = send(connectionSocket, clientMsg, sizeof(clientMsg), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR writing to socket\n");
                    childExitMethod = 1;
                    exit(1);
                }



                // Clear Buffer for key length
                memset(clientMsg, '\0', sizeof(clientMsg));
                memset(buffer, '\0', sizeof(buffer));

                // Read in from client's message
                charsRead = recv(connectionSocket, buffer, sizeof(buffer), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR reading from socket");
                    childExitMethod = 1;
                    exit(1);
                }

                // Store length of key from buffer
                keyLength = atoi(buffer);

                // Send Message back to client
                strcat(clientMsg, "I am the server, and I got your key length.");
                charsRead = send(connectionSocket, clientMsg, sizeof(clientMsg), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR writing to socket");
                    childExitMethod = 1;
                    exit(1);
                }



                // Clear Buffer for message
                memset(clientMsg, '\0', sizeof(clientMsg));
                memset(buffer, '\0', sizeof(buffer));

                // Read in from client's message
                charsRead = recv(connectionSocket, buffer, sizeof(buffer), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR reading from socket");
                    childExitMethod = 1;
                    exit(1);
                }

                // Store message
                strcat(messageBuffer, buffer);



                // Decrement message length
                msgLength = msgLength - strlen(buffer);
                
                // Loop through message until...
                while (1) {

                    //Break if nothing in file
                    if (strlen(buffer) == 0) break;


                    // Breaks if there is nothing else in the message
                    if (msgLength == 0) break;

                    else {
                        // Clear Buffer for message
                        memset(buffer, '\0', sizeof(buffer));

                        // Read in from client's message
                        charsRead = recv(connectionSocket, buffer, sizeof(buffer), 0);
                        if (charsRead < 0) {
                            error("ENC_SERVER: ERROR writing to socket");
                            childExitMethod = 1;
                            exit(1);
                        }

                        // Decrement message length
                        msgLength = msgLength - strlen(buffer);

                        // Store message
                        strcat(messageBuffer, buffer);
                    }
                }                 

                // Send Message back to client
                strcat(clientMsg, "I am the server, and I got your message.");
                charsRead = send(connectionSocket, clientMsg, sizeof(clientMsg), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR writing to socket");
                    childExitMethod = 1;
                    exit(1);
                }
         


                // Clear Buffer for message
                memset(clientMsg, '\0', sizeof(clientMsg));
                memset(buffer, '\0', sizeof(buffer));

                // Read in from client's message
                charsRead = recv(connectionSocket, buffer, sizeof(buffer), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR writing to socket");
                    childExitMethod = 1;
                    exit(1);
                }

                // Store key
                strcat(keyBuffer, buffer);



                // Decrement key length
                keyLength = keyLength - strlen(buffer);    
                
                // Loop through key until...
                while (1) {

                    // Break if nothing in file
                    if (strlen(buffer) == 0) break;


                    // Breaks if there is nothing else in the key
                    if (keyLength == 0) break;

                    else {
                        // Clear Buffer for message
                        memset(buffer, '\0', sizeof(buffer));

                        // Read in from client's message
                        charsRead = recv(connectionSocket, buffer, sizeof(buffer), 0);
                        if (charsRead < 0) {
                            error("ENC_SERVER: ERROR writing to socket");
                            childExitMethod = 1;
                            exit(1);
                        }

                        // Decrement length
                        keyLength = keyLength - strlen(buffer);

                        // Store key
                        strcat(keyBuffer, buffer);
                    }
                }
                // Send Message back to client
                strcat(clientMsg, "I am the server, and I got your key.");
                charsRead = send(connectionSocket, clientMsg, sizeof(clientMsg), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR writing to socket");
                    childExitMethod = 1;
                    exit(1);
                }



                // Encrpyt Message + store into encryptedMsg
                strcpy(encryptedMsg, encryptFunct(encryptedMsg, keyBuffer, messageBuffer));

                // Send encryted message to client
                charsRead = send(connectionSocket, encryptedMsg, strlen(encryptedMsg), 0);
                if (charsRead < 0) {
                    error("ENC_SERVER: ERROR writing to socket");
                    childExitMethod = 1;
                    exit(1);
                }

                childExitMethod = 0;
                exit(0);

            
            default:
                // Wait for child process to stop
                waitpid(pid, &childExitMethod, WNOHANG);
        }

        // Close the connection socket for this client
        close(connectionSocket);
    }

    // Memory Management
    memset(buffer, '\0', sizeof(buffer));
    memset(messageBuffer, '\0', sizeof(messageBuffer));
    memset(keyBuffer, '\0', sizeof(keyBuffer));
    memset(clientMsg, '\0', sizeof(clientMsg));

    // Close the listening socket
    close(listenSocket);
    return 0;
}