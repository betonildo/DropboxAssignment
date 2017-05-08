# compile and link client
gcc -o dropboxClient src/dropboxClient.c src/dropboxUtil.c -I"include/" -pthread

# compile and link server
gcc -g -o dropboxServer src/dropboxServer.c src/dropboxUtil.c -I"include/" -pthread

#expand macros
gcc -E src/dropboxServer.c -pthread -I"include/" > server_out.c