# compile and link client
gcc -o dropboxClient src/dropboxClient.c src/dropboxUtil.c -I"include/" -pthread

# compile and link server
gcc -o dropboxServer src/dropboxServer.c src/dropboxUtil.c -I"include/" -pthread