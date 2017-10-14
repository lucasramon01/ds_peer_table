/*
* Peer Table System 
* Created by Bernardo Meneghini, Lucas Ramon and Thiago Alexandre.
* This program implements a peer table and exports the find and store functions.
*/

#include "supportl.h"

char *peer_table[32];
char *result;
int  key;
char *address;

void find(int argc)
{
     result =  peer_table[argc];	
}

void store(int argc, char *argv)
{
     peer_table[argc] = argv;
     result = peer_table[argc];	
}

int main(int argc, char *argv[])
{
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
    	    strcpy(buffer,result);
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
