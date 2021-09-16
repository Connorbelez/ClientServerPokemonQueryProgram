
#define TRUE 1
#define FALSE 0
#define SERVER_PORT 5005
#define SERVER_IP "127.0.0.1"


//struct 
typedef struct Node {
    char pokemonresult[200];
    struct Node *next_node;
} NodeType;



//Function prototypes: 

void createNode(char *pokemonresult);
void *savePokeList(void *list);
void freeList(NodeType *headNode);
int testFile(char *);

