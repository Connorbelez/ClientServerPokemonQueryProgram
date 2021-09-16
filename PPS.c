/*
 ============================================================================
 Name        : Pokemon Property Server (PPS)
 Author      : Connor Beleznay
 Version     : 3
 Copyright   : Copyright: Connor Beleznay, 2021 
 Description : Server that will open an existing csv from user input, accept 
 connections from a client and query the csv file for Type 1 field entered by the user 
 client side, sending each result back to client,  
 ============================================================================
 */

/*************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "serverUtil.h"
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>


int main() {
    int                 serverSocket;
    int                 clientSocket; //this is global so the query program knows where to send the results
    struct              sockaddr_in  serverAddress, clientAddr;
    int                 status, addrSize, bytesRcv;
    char                buffer[80];
    FILE                *fid, *destFid;
    int                 menu_choice;
    char                user_search[100]; //destfile is the name of the csv to write to, pokemonTypeSearch is the 
    char                pokemonTypeSearch[80];
    char                fileName[100];
    int                 clientConnected; //loop flag indicating if a client is connected

    
    //============================= Open File ===================================
    do {

        printf("Enter File Name: \n");
        scanf("%s", user_search);

        //open the file and error check
        fid = fopen(user_search, "r");
        if (!fid) { //Error checking, frees the user_search variable and redeclares it to avoid mem leak
            printf("Pokemon file is not found. Please enter the name of the file again or press ctrl+c to quit\n");
        }
    } while (!fid);

    // printf("File %s found...\n", user_search); //informs user file is found
    fclose(fid);



    //============================= Create the server socket ===================================

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        printf("*** SERVER ERROR: Could not open socket.\n");
        exit(-1);
    }


    //============================= Setup Connection =============================================== 
    memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons((unsigned short) SERVER_PORT);

    status = bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if (status < 0) {
        printf("*** SERVER ERROR: Could not bind socket.\n");
        exit(-1);
    }
    // Set up the line-up to handle up to 5 clients in line 
    status = listen(serverSocket, 5);
    if (status < 0) {
        printf("*** SERVER ERROR: Could not listen on socket.\n");
        exit(-1);
    }

    while (1) {
        addrSize = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
        if (clientSocket < 0) {
            printf("*** SERVER ERROR: Could not accept incoming client connection.\n");
            exit(-1);
        }
        printf("SERVER: Received client connection.\n");
        //============================= Connection Established ============================================= 


        clientConnected = TRUE; // flag used to indicate if there is a connection
        //============================= Talk To Clients ============================================ 
        do {
            // Get Menu choice from the client

            bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);

            //============================ Menu switch =====================================
            menu_choice = atoi(buffer); 
            switch (menu_choice) {

                //============================ CASE 1: Type Search =====================================

                case 1:
                    printf("Pokemon type search selected\n");
                    send(clientSocket, buffer, sizeof(buffer), 0); // send menu confirmation to client


                    //Get type of pokemon to search
                    bytesRcv = recv(clientSocket, pokemonTypeSearch, sizeof(pokemonTypeSearch), 0);
                    pokemonTypeSearch[bytesRcv] = 0; // put a 0 at the end so we can display the string
                    printf("SERVER: Received client request case 1: %s\n", pokemonTypeSearch);

                    query_pokemon(pokemonTypeSearch,user_search,clientSocket);


                    break;


                    //============================ CASE 2: SavePokeList =====================================
                case 2:
                    printf("SavePokeList selected\n");
                    break;

                    //============================ CASE 3: QUIT =====================================
                case 3:

                    printf("quit selected\n");

                    send(clientSocket, buffer, strlen(buffer), 0); //send menu confirmation to client

                    bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
                    printf("SERVER: Closing socket\n");
                    close(clientSocket);

                    clientConnected = FALSE; //flag to await a new connection
                    break;

            }

        } while(clientConnected);

    }


}