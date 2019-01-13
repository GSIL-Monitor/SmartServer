#-include $(wildcard ./db/*.mk)
vpath %.cpp ./app/
vpath %.cpp ./inc/
vpath %.cpp ./net/
vpath %.cpp ./db/
vpath %.cpp ./net/gateway/
vpath %.cpp ./net/inc/
vpath %.cpp ./net/http/
vpath %.cpp ./db/db_gateway/
#vpath %.o ./obj/

CUR_PATH = ./
OBJ_PATH = $(CUR_PATH)obj

CC = g++ -g -c 
CFLAGS = `mysql_config --cflags --libs` -luuid -I./inc -L./lib -lssl -lcrypto -ldl -lpthread

objects = main.o http_client.o mystring.o
objects += http_get.o http_post.o db_user.o function.o log.o db_info.o
objects += gateway_client.o socket_server.o gateway_cmd.o db_gateway.o
objects += md5.o email.o email_ssl.o base64.o conf_info.o
#objects := $(wildcard ./obj/*.o)

target = ./bin/SmartServer

$(target):$(objects)
#	g++ -o $(target) *.o $(CFLAGS)
	g++ -o $(target) $(wildcard $(OBJ_PATH)/*.o) $(CFLAGS)
	
$(objects):%.o:%.cpp
	$(CC) $< -o $(OBJ_PATH)/$@ -I./inc -L./lib -lssl -lcrypto -ldl -lpthread
	
.PHONY : clean
clean :
	-rm $(target) $(wildcard $(OBJ_PATH)/*.o) $(wildcard *.o)
