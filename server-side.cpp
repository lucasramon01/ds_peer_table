/*
* Peer Table System 
* Created by Bernardo Meneghini, Lucas Ramon, Thiago Alexandre and Victor Moraes.
* This program implements a peer table and exports the find and store functions.
*/

#include "supportl.h"

char *peer_table[32];

int  myKey;
char *myIp = (char *) malloc(100);
char *network = (char *) malloc(100);

int nextKey;
int previousKey;
char *next = (char *) malloc(100);
char *previous = (char *) malloc(100);

int  key;
char *result = (char *) malloc(100);
char *address = (char *) malloc(100);


void find(int argc) {
     printf("Find: Valor encontrado é %s\n", peer_table[argc]);
     result = peer_table[argc];	
}

void store(int argc, char *argv) {
     printf("Key: %i and Store: %s\n", argc, argv);
     peer_table[argc] = argv;
     printf("Store: Valor armazenado é %s ", peer_table[argc]);
     result = peer_table[argc];

    if ((key > myKey && key < nextKey) || ( nextKey < myKey && ( key > myKey || key < nextKey )) ){
     	nextKey = key;
     	strcpy(next, argv);
    }
    if ((key < myKey && key > previousKey) || ( previousKey > myKey && ( key < myKey || key > previousKey )) ){
     	previousKey = key;
     	strcpy(previous, argv);
    }
}

void search(int argc){
    char str[16];
    sprintf(str, "%i", argc);
    char comand[] = "./client -i ";
    strcat(comand, next);
    strcat(comand, " -find ");
    strcat(comand, str);
    char x = system(comand);
    if ( WIFEXITED(x) ) {
    	if (WEXITSTATUS(x) != 0){
    		strcpy(result, network);
    		sprintf(str, "%d", WEXITSTATUS(x));
    		strcat(result, str);
        	printf("The return value: %s\n", result);
    	}
    }
    else if (WIFSIGNALED(x)) {
        printf("The program exited because of signal (signal no:%i)\n", WTERMSIG(x));
    }
}

void getPreviousNode(int argc){
    char str[4];
    sprintf(str, "%i", argc);
    char comand[64] = "./client -i ";
    strcat(comand, previous);
    strcat(comand, " -p ");
    strcat(comand, str);
    int x = system(comand);
    if ( WIFEXITED(x) ) {
    	if (WEXITSTATUS(x) != 0){
    		//strcpy(previous, network);
    		sprintf(str, "%d", WEXITSTATUS(x));
    		strcpy(result, str);
        	printf("The return value: %s\n", result);        	
    	}
    }
    else if (WIFSIGNALED(x)) {
        printf("The program exited because of signal (signal no:%i)\n", WTERMSIG(x));
    }
}

void callStore(){
    char str[4];
    sprintf(str, "%d", myKey);

    char comand[64] = "./client -i ";
    strcat(comand, address);
    strcat(comand, " -store ");
	strcat(comand, str);
	strcat(comand, " ");
	strcat(comand, myIp);
	int x = system(comand);

	if ( WIFEXITED(x) ) {
    	if (WEXITSTATUS(x) != 0){
    		//strcpy(previous, network);
    		sprintf(str, "%d", WEXITSTATUS(x));
    		strcpy(result, str);
        	printf("The return value: %s\n", result);        	
    	}
    }
    else if (WIFSIGNALED(x)) {
        printf("The program exited because of signal (signal no:%i)\n", WTERMSIG(x));
    }
}

void callFind(int argc){
    char str[4];
    sprintf(str, "%d", argc);

    char comand[64] = "./client -i ";
    strcat(comand, address);
    strcat(comand, " -find ");
	strcat(comand, str);
	int x = system(comand);

	if ( WIFEXITED(x) ) {
    	if (WEXITSTATUS(x) != 0){
    		sprintf(str, "%d", WEXITSTATUS(x));
    		strcpy(result, str);
        	printf("The return value: %s\n", result);        	
    	}
    }
    else if (WIFSIGNALED(x)) {
        printf("The program exited because of signal (signal no:%i)\n", WTERMSIG(x));
    }
}

// Define a rede.
int defineNetwork(int argc, char *argv[]){
	char y[16];
	strcpy(y, argv[3]);
	int len = strlen(y);
	int i;
	for (i = len - 1; i > 0; i--) {
		if (y[i] == '.') {
        	i = 0;    
        } else {
			y[i] = y[i+1];  	
        }        
    }
    strcpy(network, y);

    return 0;
}

// Comando para saber seu próprio Ip
void defineMyIp(){
	char y[16];
    char comand[] = "ifconfig | grep inet' 'end.:' '";
    strcat(comand, network);
    strcat(comand, " | cut -f2 -d':'| cut -f2 -d' ' > output.txt");
    system(comand);

    FILE *arq;
    arq = fopen("output.txt", "rt");
    fgets(y, 100, arq);    
	strcpy(myIp, y);

    remove("output.txt");
}

void onInit(int argc, char *argv[]){
	strcpy(next, "0");
	int i;
	for(i = 0; i < 32; i++ ){
		store(i, next);
	}
	// Salving the key to the node
	char s[16];
	strcpy(s, argv[1]);
	myKey = atoi(s);
	// Salving the next node
	strcpy(s, argv[2]);	
	strcpy(next, argv[3]);
	nextKey = atoi(s);
	store(nextKey, next);

	defineNetwork(argc, argv);
	defineMyIp();

	// If the node is not the first to enter in the network
	if (strcmp(argv[4], "y") != 0){
		getPreviousNode(nextKey);
		previousKey = atoi(result);

		strcpy(address, next);
		callFind(previousKey);
		strcpy(previous, result); 

		strcpy(address, previous);
		callStore();
		strcpy(address, next);
		callStore();
	}
}

int main(int argc, char *argv[])
{
    onInit(argc, argv);
    int listener, sockfd; // Socket file descriptors
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    // Opening socket to listen to connection:
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
        error("ERROR opening socket");

    // preparing socket:
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT_NUMBER);
    if (bind(listener, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(listener,5);

    while(1){

        char buffer[BUFFER_SIZE], filename[BUFFER_SIZE], operation[BUFFER_SIZE];
        struct Frame frame = {};

        // Accepting Connection. Receiving message request for frame size:
        sockfd = accept(listener, (struct sockaddr *) &cli_addr,  &clilen);
        if (sockfd < 0)
            error("ERROR on accept");
        char msg[25];
        receiveMessage(sockfd, msg);

        // Sending success message
        strcpy(buffer,"Connection established!");
        sendMessage(sockfd, buffer);


        bzero(buffer,BUFFER_SIZE);
        receiveMessage(sockfd, buffer);

        // Sending frame size response:
        bzero(buffer, BUFFER_SIZE);
        sprintf(buffer,"%d",MAX_BUFFER);
        sendMessage(sockfd, buffer);


	    //Receiving operation
	    bzero(operation,BUFFER_SIZE);
        bzero(operation,BUFFER_SIZE);
        receiveFrame(&frame, sockfd);
        strcpy(operation, frame.data);
        //strcat(operation, "2");
	
        if(strcmp("find",operation) == 0){

            printf("Find Operation\n");
            // send confirmation:
            strcpy(buffer,"OK");
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving the key:
            receiveFrame(&frame, sockfd);
            key = (int) *frame.data;
            printf("Receiving the key: %i \n", key);

            //find operation:
    	    find(key);

    	    // if not find it, ask the next
    	    if (*result == '0'){
                search(key);                
            }

    	    strcpy(buffer, result);
    	    createFrame(&frame, result);
    	    sendFrame(&frame, sockfd, frameSize(&frame));

        } else if (strcmp("store",operation) == 0){

            // send confirmation:
            strcpy(buffer,"OK");
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving the key:
            receiveFrame(&frame, sockfd);
            key = (int) *frame.data;            
            printf("Receiving the key: %i \n", key);

            // send confirmation:
            strcpy(buffer,"OK");
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving the address:
            receiveFrame(&frame, sockfd);
            strcpy(address, frame.data);
            printf("Receiving the key: %s \n", address);

            //store operation:
            store(key, address);
            strcpy(buffer,result);
            createFrame(&frame, result);
            sendFrame(&frame, sockfd, frameSize(&frame));          
    	

        } else if(strcmp("get node",operation) == 0){

            printf("Get Node\n");
            // send confirmation:
            strcpy(buffer,"OK");
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

            // receiving the key:
            receiveFrame(&frame, sockfd);
            key = (int) *frame.data;
            printf("Receiving the key: %i \n", key);

            //find operation:
    	    find(key);

    	    // if not find it, ask the next
    	    if (key == nextKey){
				char str[4];
    			sprintf(str, "%i", myKey);
    			strcpy(result, str);	
    	    	//strcpy(result,myIp);                                
            } else {
	            getPreviousNode(key);
            }

    	    strcpy(buffer, result);
    	    createFrame(&frame, result);
    	    sendFrame(&frame, sockfd, frameSize(&frame));

        } else if (strcmp("send",operation) == 0){

        	// send confirmation:
            strcpy(buffer,"OK");
            createFrame(&frame, buffer);
            sendFrame(&frame, sockfd, frameSize(&frame));

    		// Receiving filename:
    		bzero(filename,BUFFER_SIZE);
    		receiveFrame(&frame, sockfd);
    		strcpy(filename, frame.data);
    		strcat(filename, "2");
    		printf("File Name should be %s\n", filename);

    		// Sending OK message
    		strcpy(buffer,"Ready to Transfer Files...");
    		createFrame(&frame, buffer);
    		sendFrame(&frame, sockfd, frameSize(&frame));

    		// Initiating file transfer. Firstly, opening file to write:
    		FILE* file;
    		int fd;
    		file = fopen(filename,"wb");
    		fd = fileno(file);
    		if( !file | fd < 0)
    			error("Couldn't create file!");

    		// Actually receiving and writing file:
    		while(1) {
    		    bzero(buffer, BUFFER_SIZE);
    		    receiveFrame(&frame, sockfd);
    		    getData(&frame, buffer);
    		    size_t len = strlen(buffer);
    		    printf("Message of %d bytes received from client\n\n", (int) len);
    		    if ((int) len <= 0) break;
    		    else fwrite(buffer, sizeof(char), len, file);
    		}

    		printf("File received.\n");
    		fclose(file);
	   }
        close(sockfd);
        sleep(1);
    }
    return 0;
}
