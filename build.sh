gcc server_tcp.c  -o server -pthread
gcc client_tcp.c  -o client -pthread

# compile and link client
gcc -o dropboxClient src/dropboxClient.c src/dropboxUtil.c -I"include/" -pthread

# compile and link server
gcc -o dropboxServer src/dropboxServer.c src/dropboxUtil.c -I"include/" -pthread