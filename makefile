client: mftp.c
	gcc -o client mftp.c

server: mftpserve.c
	gcc -o server mftpserve.c

cleans:
	rm -f server
cleanc:
	rm -f client
