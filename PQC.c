/*
 ============================================================================
 Name        : Pokemon Query Client (PQC)
 Author      : Connor Beleznay
 Version     : 3
 Copyright   : Copyright: Connor Beleznay, 2021
 Description : Client that will serve as a user interface for users to input a desired 
            search query to the PPS (server) communicate with said server, recieve
            query results from the server, store results in a linked list
            and save to a file with filename indicated by user. 
 ============================================================================
 */

/*************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "clientUtil.h"
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>


int main() {
    extern NodeType     *queryResults; //declared in clientUtil.c
    pthread_t           t1; //thread  used to save pokelist
    extern sem_t        mutex;
    struct sockaddr_in  clientAddress;
    int                 status, clientSocket, bytesRcv;
    char                buffer[200];   // stores sent and received data
    char                menu_choiceS[2];
    char                **fileIds; //list of files created
    int                 menu_choice1;
    int                 counter = 0; // Counter holds the number of queries
    int                 fileCounter = 0;


    if (sem_init(&mutex, 0, 1) < 0) {
        printf("Error: on semaphore init.\n");
        exit(1);
    }

    fileIds = malloc(fileCounter * sizeof(char *));


    //============================= Setup Connection ===============================================
    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0) {
        printf("*** CLIENT ERROR: Could open socket.\n");
        exit(-1);
    }

    // Setup address
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    clientAddress.sin_port = htons((unsigned short) SERVER_PORT);

    // Connect to server
    status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
    if (status < 0) {
        printf("*** CLIENT ERROR: Unable to establish connection to the PPS!.\n");
        exit(-1);
    }


    //======================================== Communicate with server ===============================================

    while (1) {
        // Get a command from the user
        printf("1: Type Search \n2: Save results\n3: Exit Program\n");
        scanf("%d", &menu_choice1);


        //check for invalid menu choice
        if (menu_choice1 > 3 || menu_choice1 < 1) {

            printf("INVALID MENU CHOICE \n");
            menu_choice1 = 3;
            menu_choiceS[0] = '3'; 

            send(clientSocket, menu_choiceS, sizeof(menu_choiceS), 0);
            break;
        }

        switch (menu_choice1) {

            //====================================== CASE 1 =====================================================
            case 1 :

                menu_choiceS[0] = '1';
                char *inStr;
                send(clientSocket, menu_choiceS, sizeof(menu_choiceS), 0); //send menu choice to server


                //wait for confirmaton
                bytesRcv = recv(clientSocket, buffer, sizeof(buffer),0);  //recieve menu choice confirmation from server

                buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string


                //search for pokemon
                printf("Enter type of pokemon to search: \n");
                scanf("%ms", &inStr);

                // Send type string to server
                send(clientSocket, inStr, strlen(inStr), 0);

                //Recieve incoming pokemon strings
                while (strcmp(buffer, "done") != 0) {
                    //recieve file

                    bytesRcv = recv(clientSocket, buffer, sizeof(buffer),0);  //recieve incoming pokemon string
                    buffer[bytesRcv] = 0;
                    //send confirmation

                    
                    if (strcmp(buffer, "done") !=0){
                        createNode(buffer); //buffer holds a pokemon string, add to a linked list
                    }
                    send(clientSocket, buffer, sizeof(buffer), 0); //send confirmation client is ready for another string

                }


                free(inStr);

                counter++;
                break;

                //====================================== CASE 2 =====================================================
            case 2 :

                menu_choiceS[0] = '2';

                char *destFile;
                printf("Enter a filename to save to (inc. file extension) \n");
                scanf("%ms", &destFile);



                if (testFile(destFile)) {
                    menu_choiceS[0] = '3';
                    send(clientSocket, menu_choiceS, sizeof(menu_choiceS), 0);
                    bytesRcv = recv(clientSocket, buffer, 80, 0); //send menu confirmation to client
                    printf("Closing socket\n");
                    close(clientSocket);
                    exit(0);
                }


                //Keep track of file names
                fileIds = realloc(fileIds, (fileCounter + 1) * sizeof(char *));
                fileIds[fileCounter] = malloc(sizeof(char *) + 1);
                strcpy(fileIds[fileCounter], destFile);
                fileCounter++;

                pthread_create(&t1, NULL, savePokeList, destFile);
                pthread_detach(t1);

                // printf("LINE 153!\n");
                break;

                //================================== CASE 3 ============================================
            case 3:

                menu_choiceS[0] = '3';
                send(clientSocket, menu_choiceS, sizeof(menu_choiceS), 0);
                bytesRcv = recv(clientSocket, buffer, 80, 0); //send menu confirmation to client

                if (fileCounter > 0) {
                    printf("\nTotal search queries: %d\nFiles Created:\n\n", counter);
                    for (int i = 0; i < fileCounter; i++) {
                        printf("%d. %s\n", i + 1, fileIds[i]);
                        free(fileIds[i]);
                    }
                    free(fileIds);
                } else if (counter > 0) {
                    printf("\nTotal search queries: %d\nNo Files Created\n", counter);
                }

                if (counter > 0) {
                    freeList(queryResults);
                }
                // free(destfile);
                
                send(clientSocket, menu_choiceS, sizeof(menu_choiceS), 0);
                printf("Closing socket\n");
                close(clientSocket);
                exit(0);
        }

    }

    close(clientSocket);
    printf("CLIENT: Shutting down.\n");
}