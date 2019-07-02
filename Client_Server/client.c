/*//////////////////////////////////////////////////////////////////////////////////////////
 *	Name: Jed Yeung
 *	File: client.c
 *	Desc: Your client program will forward service requests to the server stating 
 *	(1) the kind of service is needed, and 
 *	(2) all required data necessary for the successful completion of the request.
 *	 Your client program must use the following syntax for any request to the server
 *	 where toUpper is the request to change characters to uppercase, and count to return 
 *	 how many times the supplied character is present in the file.
 *   	Your-Prompt> toUpper < file.txt >
 *    	Your-Prompt> count < char, file.txt >
 *   In the above two examples file.txt is to indicate that a text file is to be suppled as 
 *   an input parameter. The char string is to inform that a character is to be supplied.
 *   The information returned to the client, by your server, must be stored in text files 
 *   with the names fileUpper.txt and fileChar.txt. 
/*//////////////////////////////////////////////////////////////////////////////////////////

/*
 simple client to work with server.c program.
 * Host name and port used by server is to be
 * passed as arguments.
 *
 * To test: Open a terminal window.
 * At prompt ($ is my prompt symbol) you may
 * type the following as a test:
 *
 * $./client 127.0.0.1 54554
 * Please enter the message: Operating Systems is fun!  
 * I got your message
 * $ 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

/*****************************************************************
 *    Function: error
 *    Return: void
 *    Param: msg to perror
 *    Desc: perror with msg and exit
 ****************************************************************/
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/*****************************************************************
 *	Function: displayPrompt
 *	Return: void
 *	Param: 
 *	Desc: Prompt client for sevice selection
 ****************************************************************/
 
void displayPrompt(){
	const char prompt[] = "\n(1) changing all characters of a text file to uppercase.\n\te.g., toUpper <file.txt>\n(2) counting the number of a supplied character in a text file.\n\te.g., count <char, file.txt>\n";
	fprintf(stdout,"%s",prompt);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    //hostent stores server host details , name, type, length,list
    struct hostent *server;

    char buffer[256];
    char *servChoice = malloc(sizeof(char)*8);
    char filepath[] = "file.txt"; // here it is constant

    FILE *fp; // file pointer
    int fd; //file descriptor for read
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    //
    portno = atoi(argv[2]);

    // protocol 0 -causes socket() to use an unspecified default
    //  protocol appropriate for the requested socket type.
    //  open socket to default open port, get sockfd
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    //get server details from provided address
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //copy details from hostent to local serv-addr struct that
    //is required to make the connection
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    //assign provided port number to serv_addr
    serv_addr.sin_port = htons(portno);
    //connects socketfd to address at sockaddr
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    displayPrompt();
    printf("Please enter the message: ");
    //clear buffer
    bzero(buffer,256);
    //stdin to buffer
    fgets(buffer,255,stdin);
    //write to open socket connection
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");

    //collect first token of buffer (to get choice)
    //no need to collect file name since always file.txt here
    char c; //store char
    int ind = 0; // index
    while((c = buffer[ind])!= ' '){
        servChoice[ind++] = c;
    }
    bzero(buffer,256);

    // --------------------open file to be read from-----------------
    fd = open(filepath, O_RDONLY);
    if (fd < 0)
        error("ERROR File toUpper");
    // --------------read from file into buffer-----------------------
    n = read(fd, buffer, 255);
    if (n < 0) 
        error("ERROR reading from socket");
    close(fd);
    // is it necessary here to terminate buffer with null character?
    // buffer[sizeof(buffer)] = '\0';
    //-------write file contents to open socket connection-------------
    n = write(sockfd, buffer, sizeof(buffer));
    if (n < 0) 
        error("ERROR writing to socket");
    bzero(buffer,256);

   /* The information returned to the client, by your server, must be stored in text files 
 *   with the names fileUpper.txt and fileChar.txt*/
    
    // read for results of the completed service
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");

     if (strcmp(servChoice, "toUpper") == 0){
        fp = fopen("fileUpper.txt", "wb");
        if (fp == NULL)
            error("ERROR File toUpper");
        fprintf(fp, "%s\n", buffer);
     }
     if (strcmp(servChoice, "count") == 0){
        fp = fopen("fileChar.txt", "wb");
        if (fp == NULL)
            error("ERROR file filechar");
        fprintf(fp, "%s\n", buffer);
     }

     //write to file the contents in the buffer

    printf("%s\n",buffer);
    fclose(fp);
    close(sockfd);
    free(servChoice);
    return 0;
}

