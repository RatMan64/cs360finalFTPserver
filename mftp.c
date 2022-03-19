//
// Created by Riley on 4/14/2021.
//
// Riley Barnes
// CS 360
//
//ftp server client






#include <sys/types.h>
#include "mftp.h"


int dataconnection(const char  *host , char port[]){
    int soceterr = 0;

    struct addrinfo hints, *actualdata;
    memset(&hints,0, sizeof(hints));
    int err;
    int socketfd;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    err = getaddrinfo(host, port, &hints, &actualdata);
    if(err<0){
        perror("[-] Data address Error....");
        soceterr = 1;
    }
    socketfd = socket(actualdata->ai_family, actualdata->ai_socktype,0);

    if(socketfd < 0){
        perror("[-] Data socket error.....");
        soceterr =1;

    }
    printf("[+] Server Data socket Created...\n");

    if(connect(socketfd, actualdata->ai_addr, actualdata->ai_addrlen)<0){
        perror("[-] Error in connecting....");
        soceterr = 1;

    }
    printf("[+] Data Socket Connected to server.....\n");
    if(soceterr == 0){
        return socketfd;
    }else{
        return -1;
    }

}




void readForServerresponse(int fd, char buffer[]){

    int bytes;
    int total = 0;
    while((bytes = read(fd, buffer, DATASIZE))>0){

        if(buffer[bytes-1] == '\n') break;
        total += bytes;
        buffer+=bytes;

    }
    if(bytes<0){
        perror("[-] Error connection closed..");
        exit(1);
    }
    buffer = buffer - total;


    int i = 0;
    while(1){
        if(buffer[i] == '\n'){
            buffer[i] = '\0';
            break;
        }
        i++;
    }
}









int main(int argc, char const * argv[]){ //todo have startup option


    if( argc == 1){ // if only one argument is entered then nothing was entered
        perror("[-] insufficient input....");
        exit(1);
    }else if(argc >= 2 && argc <4){
        struct addrinfo hints, *actualdata;
        memset(&hints,0, sizeof(hints));
        int err;
        int socketfd;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_INET;
        err = getaddrinfo(argv[1], CLIENTPORT, &hints, &actualdata);
        if(err<0){
            perror("[-] address Error....");
            exit(1);
        }
        socketfd = socket(actualdata->ai_family, actualdata->ai_socktype,0);
        if(socketfd < 0){
            perror("[-] socket error.....");
            exit(1);

        }
        printf("[+] Server socket Created...\n");

        if(connect(socketfd, actualdata->ai_addr, actualdata->ai_addrlen)<0){
            perror("[-] Error in connecting....");
            exit(1);

        }
        printf("[+] Connected to server.....\n");
        int exitloop = 0;

        while(exitloop != 1){ // do the user commands
            char userinput[1000];
            printf("[**]Enter Command: ");
            fgets(userinput, 1000, stdin); // grab all input from user

            char first_token[5]; // first token should be a command so should be small up to
            int i = 0;
            while (i <strlen(userinput)){ // get rid of every single \n so that they are read correctly
                if(userinput[i] == '\n'){
                    userinput[i] = '\0';
                    break;
                }
                i++;
            }


            strcpy(first_token, strtok(userinput, " ")); // copy first token
            if(strcmp(first_token, "exit") == 0){ // exit "done"
                if(strtok(NULL," ") != NULL){ // test for extra stuff if so then err
                    perror("[-] input for exit incorrect....\n");
                }else{
                    char quit[3] = "Q\n";
                    char cmdbuff[100];
                    if(write(socketfd, quit, strlen(quit)) < 0){ // write quit command
                        perror("[-] Error in sending request.....\n");
                    }else{

                        readForServerresponse(socketfd, cmdbuff); // see if server affirms

                        if(strcmp(cmdbuff,"A") !=0) {
                            exitloop = 0;
                            printf("[-] %s...", cmdbuff );
                        } else{
                            printf("[+] Sucessful exit...\n");
                            exitloop = 1;
                        }
                    }
                }
            }
            else if(strcmp(first_token,"ls") == 0){ // ls done
                if(strtok(NULL," ") != NULL){
                    perror("[-] incorect input for ls.....\n");
                }else{
                    int id2 = fork();
                    if(id2){
                        wait(0);

                    }else{

                        int fd[2];
                        char* left = "ls" ;
                        char* leftarg = "-l";
                        char* right = "more";
                        char* rightarg = "-20";

                        pipe(fd);
                        int id = fork();
                        //parent does right process and feeds to child child takes return from those processes and does the next process
                        if(id){ // second process
                            dup2(fd[0], 0);
                            close(fd[1]);
                            wait(0);
                            execlp(right,right,rightarg, (char*)NULL);
                            perror("lookslike your exec failed");
                            exit(1);
                        }else{ // first process

                            dup2(fd[1], 1);
                            close(fd[0]);
                            execlp(left,left, leftarg,(char*)NULL);
                            perror("lookslike your exec failed");
                            exit(1);
                        }


                    }
                }
            }
            else if(strcmp(first_token,"rls") == 0){ // rls done
                if(strtok(NULL," ") != NULL){
                    perror("[-] incorect input for rls.....\n");
                }else{
                    char lcommand[3] = "L\n";// lisst command
                    char dcommand[3] = "D\n"; // data connection command
                    char serverResponse[100];

                    write(socketfd,dcommand, strlen(dcommand));

                    readForServerresponse(socketfd, serverResponse);

                    if(serverResponse[0] != 'A'){
                        if(serverResponse[0] == 'E'){
                            printf("[-] %s", serverResponse);
                            continue;
                        }
                    }
                    int portfordata=0; // get port
                    int first = 1;
                    int i =1;
                    char temp[20];
                    while(i < strlen(serverResponse)){
                        if(first == 1){
                            portfordata =serverResponse[i] -'0';
                            first = 0;
                        }else{
                            portfordata = portfordata*10;
                            int tem =serverResponse[i] - '0';
                            portfordata+= tem;

                        }

                        i++;
                    }

                    sprintf(temp, "%d", portfordata);

                    temp[strlen(temp)] = '\0';

                    int datasocket;
                    datasocket = dataconnection(argv[1], temp );

                    if( datasocket < 0){
                        perror("[-] Data Socket creation erred....");

                    }else{

                        write(socketfd, lcommand, strlen(lcommand));

                        char response[100];
                        readForServerresponse(socketfd, response);

                        if(strcmp(response, "A") == 0){ // server has read everything into data
                            char* right = "more";
                            char* rightarg = "-20";
                            int idrls = fork();

                            if(idrls){
                                wait(0);
                                close(datasocket);
                                printf("[+] data socket closed....\n");
                            }else{

                                dup2(datasocket, 0);
                                execlp(right,right,rightarg,(char*) NULL);// perform more
                                exit(1);
                            }

                        } else if(response[0] == 'E'){// erred close data connection
                            printf("[-] %s....\n", response);
                            close(datasocket);
                            printf("[+] data socket closed....\n");
                        }
                    }

                }
            }
            else if(strcmp(first_token,"cd") == 0){ //cd done
                if(strtok(NULL," ") == NULL){
                    perror("[-] No path was given.....\n");
                }
                else if(strtok(NULL," ") != NULL){
                    perror("[-] incorrect input for cd.....\n");

                }
                else{
                    char path[100];
                    int first= 0;
                    int a = 0;
                    int b = 0;
                    while(1){ // grab path cause token checks screwed up this section
                        if(userinput[b] == '\0' && first != 1){
                            first = 1;
                            b++;
                        }

                        if(first == 1){
                            path[a] = userinput[b];
                            if(userinput[b] == '\0'){
                                break;
                            }
                            a++;
                        }


                        b++;
                    }
                    struct stat area, *s = &area;
                    if(stat(path, s) <0){
                        perror("[-] path given is not good....");
                    }else if(access(path,R_OK)<0){ // read acces
                        perror("[-] Error you cant read...");
                    }else if(access(path, X_OK)<0){
                        perror("[-] Error you cant execute.... ");

                    }else if((S_ISREG(s->st_mode))!=0) {
                        perror("[-] this is a file path not a dir path....");
                    }else if((S_ISDIR(s->st_mode)) != 0){
                        chdir(path);
                        printf("[+] directory changed...\n");
                    }
                }

            }
            else if(strcmp(first_token,"rcd") == 0){ // rcd done
                if(strtok(NULL," ") == NULL){
                    perror("[-] No path was given.....");
                }
                else if(strtok(NULL," ") != NULL){
                    perror("[-] incorrect input for rcd.....");

                } else{
                    char path[100];
                    int first= 0;
                    int a = 0;
                    int b = 0;
                    while(1){ // grab path cause token checks screwed up this section
                        if(userinput[b] == '\0' && first != 1){
                            first = 1;
                            b++;
                        }

                        if(first == 1){
                            path[a] = userinput[b];
                            if(userinput[b] == '\0'){
                                break;
                            }
                            a++;
                        }


                        b++;
                    }

                    char ccomand[150] = "C";
                    strcat(ccomand, path);
                    ccomand[strlen(ccomand)] = '\n';

                    write(socketfd, ccomand, strlen(ccomand));
                    char response[100];

                    readForServerresponse(socketfd, response);

                    if(response[0] == 'E'){
                        printf("[-] %s....\n", response );
                    }else if(response[0] == 'A'){
                        printf("[+] cd of server changed..\n");
                    }

                }



            }
            else if(strcmp(first_token,"get") == 0){ // get
                if(strtok(NULL," ") == NULL){
                    perror("[-] No path was given.....");
                }
                else if(strtok(NULL," ") != NULL){
                    perror("[-] incorrect input for rcd.....");

                }else{
                    char gcomand[150] = "G";
                    char filename[100];
                    char dcommand[3] = "D\n";
                    char serverResponse[100];

                    char path[100];
                    int firsts= 0;
                    int a = 0;
                    int b = 0;
                    while(1){ // grab path cause token checks screwed up this section
                        if(userinput[b] == '\0' && firsts != 1){
                            firsts = 1;
                            b++;
                        }

                        if(firsts == 1){
                            path[a] = userinput[b];
                            if(userinput[b] == '\0'){
                                break;
                            }
                            a++;
                        }


                        b++;
                    }

                    int c = 0;
                    int start = 0;
                    while (c < strlen(path)){ // grab filename
                        if(path[c] == '/'){
                            start = c+1;
                        }
                        c++;
                    }
                    int y = 0;
                    while(start < strlen(path)){ // set null ternimator
                        filename[y] = path[start];
                        y++;
                        start++;
                        if(start == strlen(path)){
                            filename[y] = '\0';
                        }
                    }

                    struct stat area, *s = &area;
                    if(stat(path,s) ==0 ){
                        printf("[-] Error File exists...\n");
                        continue;
                    }
                    write(socketfd,dcommand, strlen(dcommand));

                    readForServerresponse(socketfd, serverResponse);
                    if(serverResponse[0] != 'A'){
                        if(serverResponse[0] == 'E'){
                            printf("[-] %s", serverResponse);
                            continue;
                        }
                    }
                    int portfordata=0;
                    int first = 1;
                    int ie =1;
                    char temp[20];
                    while(ie < strlen(serverResponse)){
                        if(first == 1){
                            portfordata =serverResponse[ie] -'0';
                            first = 0;
                        }else{
                            portfordata = portfordata*10;
                            int tem =serverResponse[ie] - '0';
                            portfordata+= tem;

                        }


                        ie++;
                    }

                    sprintf(temp, "%d", portfordata);

                    temp[strlen(temp)] = '\0'; // make sure its null terminated=

                    int datasocket;
                    datasocket = dataconnection(argv[1], temp );
                    if(datasocket<0){
                        perror("[-] Data Socket creation erred....");

                    }else{ // do the get part


                        strcat(gcomand, path);
                        gcomand[strlen(gcomand)] ='\n';

                        write(socketfd, gcomand, strlen(gcomand));
                        char response[100];


                        readForServerresponse(socketfd, response);

                        if(response[0] == 'A'){



                        int newfilefd = open(filename, O_WRONLY|O_RDONLY|O_CREAT , 0700);
                        char filebuffer[DATASIZE];
                        int bytes = 0;

                        while((bytes = read(datasocket, filebuffer, DATASIZE)) >0){

                            if(bytes<DATASIZE){
                                write(newfilefd,filebuffer, bytes);
                                break;
                            }
                            write(newfilefd,filebuffer, bytes);
                            memset(filebuffer, 0, sizeof(filebuffer));
                        }
                        if(bytes<0){
                            perror("[-] Error socket closed");
                            exit(1);
                        }

                        close(newfilefd);
                        close(datasocket);

                        }else if(response[0] == 'E'){
                            printf("[-] %s...\n", response );
                            close(datasocket);

                        }


                    }




                }


            }
            else if(strcmp(first_token,"show") == 0){ // show exactly like get but not saving the file just outputing with more its contents
                if(strtok(NULL," ") == NULL){
                    perror("[-] No path was given.....");
                }
                else if(strtok(NULL," ") != NULL){
                    perror("[-] incorrect input for rcd.....");

                }else{
                    char gcomand[150] = "G";
                    char filename[100];
                    char dcommand[3] = "D\n";
                    char serverResponse[100];

                    char path[100];
                    int firsts= 0;
                    int a = 0;
                    int b = 0;
                    while(1){ // grab path cause token checks screwed up this section
                        if(userinput[b] == '\0' && firsts != 1){
                            firsts = 1;
                            b++;
                        }

                        if(firsts == 1){
                            path[a] = userinput[b];
                            if(userinput[b] == '\0'){
                                break;
                            }
                            a++;
                        }


                        b++;
                    }


                    write(socketfd,dcommand, strlen(dcommand));

                    readForServerresponse(socketfd, serverResponse);
                    if(serverResponse[0] != 'A'){
                        if(serverResponse[0] == 'E'){
                            printf("[-] %s", serverResponse);
                            continue;
                        }
                    }
                    int portfordata=0;
                    int first = 1;
                    int ie =1;
                    char temp[20];
                    while(ie < strlen(serverResponse)){
                        if(first == 1){
                            portfordata =serverResponse[ie] -'0';
                            first = 0;
                        }else{
                            portfordata = portfordata*10;
                            int tem =serverResponse[ie] - '0';
                            portfordata+= tem;

                        }


                        ie++;
                    }

                    sprintf(temp, "%d", portfordata);

                    temp[strlen(temp)] = '\0'; // make sure its null terminated=

                    int datasocket;
                    datasocket = dataconnection(argv[1], temp );
                    if(datasocket<0){
                        perror("[-] Data Socket creation erred....");

                    }else{ // do the get part


                        strcat(gcomand, path);
                        gcomand[strlen(gcomand)] ='\n';

                        write(socketfd, gcomand, strlen(gcomand));
                        char response[100];


                        readForServerresponse(socketfd, response);

                        if(response[0] == 'A'){


                            char* right = "more";
                            char* rightarg = "-20";
                            int idrls = fork();

                            if(idrls){
                                wait(0);
                                close(datasocket);
                                printf("[+] data socket closed....\n");
                            }else{

                                dup2(datasocket, 0);
                                execlp(right,right,rightarg,(char*) NULL);
                                exit(1);
                            }



                        }else if(response[0] == 'E'){
                            printf("[-] %s..\n", response);
                            close(datasocket);

                        }

                    }
                }

            }
            else if(strcmp(first_token,"put") == 0){ // put
                if(strtok(NULL," ") == NULL){
                    perror("[-] No path was given.....");
                }
                else if(strtok(NULL," ") != NULL){
                    perror("[-] incorrect input for rcd.....");

                }else{
                    char pcomand[150] = "P";
                    char dcommand[3] = "D\n";
                    char serverResponse[100];

                    char path[100];
                    int firsts= 0;
                    int a = 0;
                    int b = 0;
                    while(1){ // grab path cause token checks screwed up this section
                        if(userinput[b] == '\0' && firsts != 1){
                            firsts = 1;
                            b++;
                        }

                        if(firsts == 1){
                            path[a] = userinput[b];
                            if(userinput[b] == '\0'){
                                break;
                            }
                            a++;
                        }


                        b++;
                    }
                    struct stat area, *s = &area;
                    if(stat(path, s) <0){
                        printf("[-] path given is not good....\n");
                    }
                    else if(S_ISREG(s->st_mode)== 0){
                        printf("[-] not reg file...\n");
                    }
                    else if(access(path,R_OK)<0){
                        printf("[-] file has no reading allowed..");
                    }
                    else{

                        write(socketfd,dcommand, strlen(dcommand));

                        readForServerresponse(socketfd, serverResponse);
                        if(serverResponse[0] != 'A'){
                            if(serverResponse[0] == 'E'){
                                printf("[-] %s", serverResponse);
                                continue;
                            }
                        }
                        int portfordata=0;
                        int first = 1;
                        int ie =1;
                        char temp[20];
                        while(ie < strlen(serverResponse)){
                            if(first == 1){
                                portfordata =serverResponse[ie] -'0';
                                first = 0;
                            }else{
                                portfordata = portfordata*10;
                                int tem =serverResponse[ie] - '0';
                                portfordata+= tem;

                            }


                            ie++;
                        }

                        sprintf(temp, "%d", portfordata);

                        temp[strlen(temp)] = '\0'; // make sure its null terminated=

                        int datasocket;
                        datasocket = dataconnection(argv[1], temp );
                        if(datasocket<0){
                            perror("[-] Data Socket creation erred....");

                        }else{ // preform put
                            strcat(pcomand, path);
                            pcomand[strlen(pcomand)] ='\n';

                            struct stat area, *s = &area;
                            if(stat(path, s) <0){
                                printf("[-] path given is not good....\n");
                                close(datasocket);

                            }else if(S_ISREG(s->st_mode)== 0){
                                printf("[-] not reg file...\n");
                                close(datasocket);
                            }
                            else if(access(path,R_OK)<0){
                                printf("[-] file has no reading allowed..");
                                close(datasocket);
                            }else{

                                int filefd = open(path, O_RDONLY);
                                int bytes =0;
                                char filebuffer[DATASIZE];
                                while((bytes = read(filefd,filebuffer, sizeof(filebuffer)))>0){


                                    write(datasocket, filebuffer, bytes);
                                    memset(filebuffer,0, sizeof(filebuffer));
                                }
                                if(bytes<0){
                                    close(filefd);
                                    unlink(filefd);
                                    perror("[-] Error socket closed");
                                    exit(1);
                                }
                                printf("[+] out of file transfer\n");
                                close(filefd);
                                close(datasocket);
                                printf("[+] dataconnection is closed...\n");

                                write(socketfd, pcomand, strlen(pcomand));
                            }
                        }
                    }
                }
            }
            else perror("[-] input incorrect....\n");

        }
        close(socketfd);
        exit(1);
    }






}

