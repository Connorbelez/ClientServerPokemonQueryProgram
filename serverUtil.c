/*
 ============================================================================
 Name        : clientUtil.c
 Author      : Connor Beleznay
 Version     : 3
 Copyright   : Copyright: Connor Beleznay, 2021
 Description : c file containing functions and/or globals used by PPS.c 
 ============================================================================
 */

/*************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "serverUtil.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>



/*********************************************************************/
// Function: *query_pokemon
// Description: declared as a function pointer with a void return type and
//              void type paramaters, as the function is intended to be run
//              as a thread. The function will parse a CSV file line by line
//              using fgets(), then storing the full line in currPokemon, which is then
//              copied into tokenString for parsing.  
//              the first 3 values in the line are parsed using commas as the seperators
//              the third value will be stored in the token and is compared to 
//              the string the user entered to search, if its identical then the 
//              found pokemon variable is set to true and currPokemon string is sent to the client.
//              the program then waits for client confirmatio and continues. When the end of the file is reached,
//              it sends back "stop" to the client before returning from the fucnction
//
//
//
// Input 
// *pokemontypesearch: pointer to the string which the query_pokemon function will be searching for. 
// *userSearch: string represting name of csv file to query
//  socket: int holding client socket number
//
// Output: None, but the currPokemon string is sent to the client if pokemonFound is True.
// 
//
// Return: void
// 
//

void *query_pokemon(char *pokemonTypeSearch, char *userSearch, int socket){

    char *typeSearch = ((char *)pokemonTypeSearch);
    int bytesRc;
    char buff[200];
//Handle case
    int i =0;
    while (typeSearch[i]) { 
        typeSearch[i] = (char)tolower(typeSearch[i]); 
        i++; 
    } 
    typeSearch[0] = (char)toupper(typeSearch[0]);

    char currPokemon[200],tokenString[200];  // curr pokemon is the current line that was read in
    int found_pokemon; //Boolean indicating if a pokemon with the desired type has been found,

    char *token; //char array to store string tokens
    // char *userSearch = ((char *)user_search);

    FILE *fid = fopen(userSearch, "r");
    while(!feof(fid)){
        found_pokemon = FALSE;

        fgets(currPokemon, 200,fid);
        stpcpy(tokenString, currPokemon); //Create a copy to preserve the origonal

        token = strtok(tokenString,",");
        for(int i=0;i<2;i++){     // only care about Type1 (3rd value) after 2 loops the token is holding the third CSV so we can stop there rather than looping through the rest. Efficiency goes BRRRR!
            token = strtok(NULL,",");
            if(token!=NULL){
                if(strcmp(token,typeSearch) == 0){
                    found_pokemon = TRUE;
                }
            }
        } 

        if(found_pokemon){
        //send pokemon string to client

            send(socket,currPokemon, sizeof(currPokemon), 0);
            bytesRc = recv(socket, buff, sizeof(buff), 0); 

        }

    }

    send(socket,"done", strlen("done"), 0);

    fclose(fid);

}