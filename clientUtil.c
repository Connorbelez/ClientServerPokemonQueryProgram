/*
 ============================================================================
 Name        : clientUtil.c
 Author      : Connor Beleznay
 Version     : 3
 Copyright   : Copyright: Connor Beleznay, 2021
 Description : c file containing functions and/or globals used by PQC.c 
 ============================================================================
 */

/*************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "clientUtil.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>


NodeType volatile *queryResults = NULL;
NodeType volatile *currNode = NULL;
NodeType volatile *nodeTail = NULL;

sem_t mutex;




/*************************************************************/
//functions


/**************************************************************
 * Function: testFile()
 * Description: Checks for valid file name by attempting to create a new file with the string passed it.
 *              if the filename is invalid it will prompt the user for a new filename or quit.
 * Parameters:
 * 		dfile: a string representing a filename
 * 
 * output: 
 *          if the filename is valid, there is no output other than the return. if it is invalid and a new filename
 *          is provided by the user, the new filename is returned via the paramter. 
 * 
 * return: 1 or 0. 1 indicates the user wishes to quit the program, 0 indicates a file was successfully opened. 
 * 
 */

int testFile(char *dfile){
    FILE *destFid;
    destFid = fopen(dfile,"w");

    while(!destFid){
        printf("Unable to create the new file: %s. Please enter the name of the file again. (inc. file extension)\n",dfile);
        printf("Enter a filename to save to or type 3 to quit to menu\n");
        free(dfile);
        char *dfile;
        scanf("%ms",&dfile);
        printf("%s\n",dfile);
        if(strcmp(dfile,"3")==0){
            return 1;
        }
        destFid = fopen(dfile,"w");
    }
    // free(dfile);
    fclose(destFid);
    return 0;

}


/*********************************************************************/
/*
 * Function: createNode()
 * Description: This function builds a new node with the data passed in and attaches it to the tail of the list. Since it is called by 
 *              
 * Parameters:
 * 		data: This is a string containing a csv line that will be added to the Data part of the linked list
 * Returns: modifies the LinkedList via paramter.
 */

void createNode(char *data){
    //CREATE NODE
    NodeType *curNode = ((NodeType *)currNode);

    curNode = (NodeType *)malloc(sizeof(NodeType));
    if(curNode == NULL){
        printf("Memory allocation Error\n");
        exit(0);
    }
  
    strcpy(curNode->pokemonresult,data);
    (curNode)->next_node = NULL;

    if(queryResults == NULL){
        (queryResults) = (curNode);
        (nodeTail) = (curNode);
    }else{
        // printf("LIST Adding next item!\n");
        (nodeTail)->next_node = (curNode);
        (nodeTail) = (curNode);
    }
    // free(data);

}




/**************************************************************
 * Function: freeList()
 * Description: iterates through linked list freeing each node then freeing the list
 * Parameters:
 * 		head_Node: Node of the linked list to begin iterating through
 * 
 */
void freeList(NodeType *head_Node){
    // NodeType *head_Node = ((NodeType *)head_Nodee);

    NodeType *nextNode;
    while(head_Node != NULL){
        nextNode = head_Node->next_node;
        free(head_Node);
        head_Node = nextNode;
    }
    free(head_Node);
}



/*********************************************************************/
/*
 * Function: savePokeList
 * Description: This function attempts to open a file with the provided string filename, 
 * if it doesn't open it prompts the user for another filename. If it does work it opens the file, stores
 * the file name in a dynamic list, then iterates through a global linked list writing each peice of data
 * in each node as a CSV line. It then releases the resources held. 
 *              
 * Parameters: 
 *      filename: pointer to string passed in representing the name of file to write to.
 * 	
 * OUTPUT: a CSV file contiaing the data stored in a linked list. 

 * Returns: void,
 */

void *savePokeList(void *str) {
    FILE *destFid;
    if (sem_wait(&mutex) < 0) { 
        printf("Error: on semaphore wait.\n"); exit(1);
    }

    destFid = fopen(str,"w");
    currNode = queryResults;
    while(currNode != NULL) {
        fprintf(destFid,"%s",currNode->pokemonresult);
        currNode = currNode->next_node;
    }

    fclose(destFid);

    if (sem_post(&mutex) < 0) { 
        printf("Error: on semaphore wait.\n"); 
        exit(1);
    }

    // fclose(destFid);
    // fclose(fid);
    free(str);
}


