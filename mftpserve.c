//
// Created by Riley on 4/14/2021.
//
// riley Barnes
// Cs 360


#include "mftp.h"
#include <strings.h>


void readForClientresponse(int fd, char buffer[]){
    int bytes;
    int total = 0;




    while((bytes = read(fd, buffer, PACKET_SIZE))>0){

        if(buffer[bytes-1] == '\n') break;
        total += bytes;
        buffer+=bytes;

    }
    if(bytes<0){
        perror("[-] Error connection closed..");
        exit(1);
    }
    buffer = buffer -total;
    int i = 0;
    while(1){

        if(buffer[i] == '\n'){

            buffer[i] = '\0';
            break;
        }
        i++;
    }
}





int main(){
    struct sockaddr_in servaddr;
    int socketfd;


    socketfd = socket(AF_INET, SOCK_STREAM,0);
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    memset(&servaddr, 0, sizeof(struct sockaddr_in));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MY_PORT_NUMBER);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);



    if(bind(socketfd, (struct sockaddr*) &servaddr, sizeof(struct sockaddr_in))){ // bind port
        perror("[-] Error in bind...");
        exit(1);
    }
    printf("[+] Server binded....\n");

    listen(socketfd, BACKLOG);
    int connectfd;

    int length = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddr;


    while(1){ // server is up now do something
        connectfd = accept(socketfd, (struct sockaddr *) &clientAddr, &length);
        if(connectfd < 0){
            perror("[-] Error in accept.....");
            continue;
        }
        printf("[+] Connection accepted......\n");
        int id = fork();
        if(id < 0){
            perror("[-] Error at fork.....");
            continue;
        }
        char hostName[NI_MAXHOST];
        int hostEntry;

        hostEntry = getnameinfo((struct sockaddr*) &clientAddr, sizeof(clientAddr), hostName, sizeof(hostName), NULL, 0, NI_NUMERICSERV);
        if(hostEntry <0){
            perror("[-] Error ");
            continue;
        }


        if(id){ // parent print who connected and time connected
            time_t current_time;
            char* c_time_string;
            current_time = time(NULL);


            c_time_string = ctime(&current_time);
            c_time_string[18] = '\n';

            printf("[*] %s connected %s.....\n",hostName, c_time_string);
            fflush(stdout);

        } else{ // child do everything the client asks of you
            int hasDataC = 0;
            int dataSocketfd;


            while(1) {

                char clientinput[10000];


                readForClientresponse(connectfd, clientinput);
                if(strcmp(clientinput,"Q") == 0){ // Q command issued exit child
                    printf("[*] Diconnecting from client %s........\n", hostName);
                    char cmdbuff[2] = "A\n";
                    write(connectfd,cmdbuff, sizeof(cmdbuff)); // send afirm
                    printf("[+] Client %s disconnected from server...\n", hostName);
                    close(connectfd); // close your done
                    break;
                }
                else if(strcmp(clientinput, "L") == 0){ // do ls on on server // todo not done
                    if(hasDataC != 1){ // if data connection has not been achieved then you need to err
                        char err[1000] = "E data connection was not established first\n";
                        write(connectfd, err, sizeof(err));
                        perror("[-] L was attempted without establishing a data connection....");
                    }else{
                        char cmdbuff[2] = "A\n";
                        write(connectfd,cmdbuff, sizeof(cmdbuff));
                        char* c = "ls" ;
                        char * arg = "-l";
                        int iddata = fork();
                        if(iddata){
                            wait(0);
                            close(dataSocketfd);
                            printf("[+] data socket closed...\n");
                        }else{
                            dup2(dataSocketfd, 1);
                            if(execlp(c,c, arg,(char*)NULL)<0) perror("[-] execlp for ls erred.....");
                            exit(1);
                        }

                    }

                }
                else if(strcmp(clientinput, "D") == 0){ // todo needs to get done before anything else
                    int errBe = 0;
                    struct sockaddr_in dataaddr, info; // create a datasocket
                    int datasocketfd;
                    int infolen;

                    infolen = sizeof(info);


                    datasocketfd = socket(AF_INET, SOCK_STREAM,0); // set socket
                    setsockopt(datasocketfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); //
                    memset(&dataaddr, 0, sizeof(struct sockaddr_in));
                    memset(&info, 0, sizeof(struct sockaddr_in));


                    dataaddr.sin_family = AF_INET;
                    dataaddr.sin_port = htons(0);
                    dataaddr.sin_addr.s_addr = htonl(INADDR_ANY);



                    if(bind(datasocketfd, (struct sockaddr*) &dataaddr, sizeof(struct sockaddr_in))){ // bind port // todo err to client
                        perror("[-] Error in bind...");
                        errBe =1;

                    }
                    printf("[+] data socket binded....\n");
                    int datatest;
                    datatest =getsockname(datasocketfd, (struct sockaddr*)&info, &infolen);
                    if( datatest <0){
                        perror("[-] Error in getting sockname ...");
                        errBe =1;
                    }
                    int portnum =ntohs(info.sin_port);

                    if(errBe ==1){

                        char err[100] = "E data connection could not be made\n";

                        write(connectfd, err, sizeof(err));
                        close(datasocketfd);
                        hasDataC = 0;
                    }else{

                        char port[20];
                        char a[20] = "A";
                        sprintf(port, "%d", portnum);
                        port[strlen(port)] = '\n';
                        strcat(a, port);
                        a[strlen(a)] = '\n';
                        int j = 0;
                        int foundnewline =0;
                        while(j< sizeof(a)){
                            if(a[j] == '\n' && foundnewline != 1){
                                foundnewline = 1;
                            }else if(foundnewline ==1){
                                a[j] = 0;
                            }
                            j++;
                        }

                        write(connectfd, a, strlen(a));
                        printf("[*] Data Port Sent to client.....\n");

                        listen(datasocketfd, 1);
                        int dataconnectfd;
                        int datalength = sizeof(struct sockaddr_in);
                        struct sockaddr_in dataclientAddr;

                        dataconnectfd = accept(datasocketfd, (struct sockaddr *) &dataclientAddr, &datalength);
                        if(dataconnectfd < 0){
                            perror("[-] Error in data accept.....");

                        }
                        printf("[+] data Connection accepted......\n");
                        dataSocketfd = dataconnectfd;
                        hasDataC = 1;

                    }

                }
                else if (clientinput[0] ==  'G' ){ // todo not done;

                    if(hasDataC == 0 ){
                        perror("[-] G was asked before a data connection was made...");
                        // send err
                    }else{

                        char path[100];
                        int index =1;
                        int indexb = 0;
                        while (1){


                            if( clientinput[index] == '\n'){
                                path[indexb] = '\0';
                                break;

                            }
                            path[indexb] = clientinput[index];
                            indexb++;
                            index++;
                        }
                        struct stat area, *s = &area;
                        if(stat(path, s) <0){
                            perror("[-] path given is not good....\n");
                            char err[100] = "Ebathpath\n";
                            write(connectfd, err,strlen(err));
                            close(dataSocketfd);

                        }else if(access(path,R_OK)<0){
                            char err[100] = "Ereadperm\n";
                            write(connectfd, err,strlen(err));
                            close(dataSocketfd);

                        }else if(S_ISREG(s->st_mode)== 0){
                            printf("[-] not reg file...\n");
                            char err[100] = "Enotreg\n";
                            write(connectfd, err,strlen(err));
                            close(dataSocketfd);
                        }
                        else{

                            int filefd = open(path, O_RDONLY);
                            int bytes =0;
                            char filebuffer[PACKET_SIZE];
                            while((bytes = read(filefd,filebuffer, sizeof(filebuffer)))>0){




                                write(dataSocketfd, filebuffer, bytes);
                                memset(filebuffer,0, sizeof(filebuffer));
                            }
                            if(bytes<0){
                                perror("[-] Error connection closed....");
                                exit(1);
                            }
                            printf("[+] out of file transfer\n");
                            close(filefd);
                            close(dataSocketfd);
                            printf("[+] dataconnection is closed...\n");
                            char acommand[10] = "A\n";
                            write(connectfd, acommand, strlen(acommand));
                        }
                    }

                }
                else if (clientinput[0] == 'C'){ // done
                    char path[100];
                    int a = 0;
                    int b = 1;
                    while(1){ // grab path cause token checks screwed up this section
                        path[a] = clientinput[b];
                        if(clientinput[b] == '\0'){
                            break;
                        }

                        a++;
                        b++;
                    }
                    struct stat area, *s = &area;
                    if(stat(path, s) <0){
                        char err1[100] = "Ebadpath\n";
                        write(connectfd, err1, strlen(err1));
                        perror("[-] path given is not good....\n");
                    }else if(access(path,R_OK)<0){ // read acces
                        char err2[100] = "Ereadpermission\n";
                        write(connectfd, err2, strlen(err2));
                        perror("[-] Error you cant read...");
                    }else if(access(path, X_OK)<0){
                        char err3[100] = "Eexecutepermission\n";
                        write(connectfd, err3, strlen(err3));
                        perror("[-] Error you cant execute.... ");

                    }else if((S_ISREG(s->st_mode))!=0) {
                        char err4[100] = "Enotdir\n";
                        write(connectfd, err4, strlen(err4));
                        perror("[-] this is a file path not a dir path....");

                    }else if((S_ISDIR(s->st_mode)) != 0){
                        chdir(path);
                        printf("[+] directory changed...\n");
                        char afirm[3] = "A\n";
                        write(connectfd, afirm, strlen(afirm));
                    }



                }
                else if ( clientinput[0] == 'P'){ //put pathname todo not done
                    if(hasDataC == 0 ){
                        perror("[-] P was asked before a data connection was made...");

                    }else{

                        char path[100];
                        int a = 1;
                        int b = 0;
                        while(1){
                            if(clientinput[a] =='\n'){
                                path[b] = '\0';
                                break;
                            }
                            path[b] = clientinput[a];
                            a++;
                            b++;
                        }
                        struct stat area, *s = &area;
                        if(stat(path,s) ==0 ){
                            printf("[-] file already exists\n");
                            close(dataSocketfd);
                            continue;
                        }


                        int newfilefd = open(path, O_WRONLY|O_RDONLY|O_CREAT , 0700);
                        char filebuffer[PACKET_SIZE];
                        int bytes = 0;

                        while((bytes = read(dataSocketfd, filebuffer, PACKET_SIZE)) >0){



                            if(bytes<PACKET_SIZE){
                                write(newfilefd,filebuffer, bytes);
                                break;
                            }
                            write(newfilefd,filebuffer, bytes);
                            memset(filebuffer, 0, sizeof(filebuffer));
                        }
                        if(bytes<0){
                            close(newfilefd);
                            unlink(newfilefd);
                            perror("[-] Error socket closed");
                            exit(1);
                        }
                        close(newfilefd);
                        close(dataSocketfd);





                    }


                }
            }



            exit(1);
        }



    }

}