FILES := ftp.o

# 编译客户端代码
client: client.o $(FILES)
	gcc -o client client.o $(FILES) -lreadline

# 编译服务端代码
server: server.o $(FILES)
	gcc -o server server.o $(FILES)


ftp.o: ftp.h ftp.c
	gcc -c ftp.h ftp.c

client.o: client.h client.c ftp.h
	gcc -c client.h client.c ftp.h ftp.h

server.o: server.c server.h ftp.h
	gcc -c server.c server.h ftp.h

# 清除编译中间生产文件
clean: 
	rm *.o *.h.gch
