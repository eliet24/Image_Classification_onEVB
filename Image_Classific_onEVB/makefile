all: server client

server: server.o 
	gcc -ggdb -o server server.o

client: client.o 
	gcc -ggdb -o client client.o
	
server_check: server_check.o 
	gcc -ggdb -o server_check server_check.o
	
pipeline_check: pipeline_check.o 
	gcc -ggdb -o pipeline_check pipeline_check.o
	

