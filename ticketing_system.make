CC = cc

# --- Object Definitions ---

# Lista di tutti i file oggetto necessari per creare l'eseguibile del SERVER

OBJS_SERVER = build/server.o build/user_operations_handler.o build/ticket_operations_handler.o \
              build/cjson_ticket.o build/cjson_user.o build/cjson_utils.o build/cJSON.o \
              build/ticket_list.o build/ticket.o build/user.o build/user_list.o \
              build/communication.o

# Lista di tutti i file oggetto necessari per creare l'eseguibile del CLILENT
OBJS_CLIENT = build/client.o build/user_operations.o build/ticket_operations.o \
              build/communication.o build/input_validation.o



all: server client

server: $(OBJS_SERVER)
	$(CC) $^ -o bin/server

client: $(OBJS_CLIENT)
	$(CC) $^ -o bin/client

# Dipendenze

# SERVER
build/server.o: src/server/server.c include/server/server.h include/server/ticket_operations_handler.h include/server/user_operations_handler.h include/json/cjson_ticket.h include/json/cjson_user.h include/ticket/ticket_list.h include/user/user_list.h include/utils/communication.h | build
	$(CC) -c src/server/server.c -o build/server.o

build/user_operations_handler.o: src/server/user_operations_handler.c include/server/user_operations_handler.h include/utils/communication.h include/utils/config.h | build
	$(CC) -c src/server/user_operations_handler.c -o build/user_operations_handler.o

build/ticket_operations_handler.o: src/server/ticket_operations_handler.c include/server/ticket_operations_handler.h include/utils/communication.h include/utils/config.h | build
	$(CC) -c src/server/ticket_operations_handler.c -o build/ticket_operations_handler.o

# JSON
build/cjson_ticket.o: src/json/cjson_ticket.c include/json/cjson_ticket.h include/json/cjson_utils.h | build
	$(CC) -c src/json/cjson_ticket.c -o build/cjson_ticket.o

build/cjson_user.o: src/json/cjson_user.c include/json/cjson_user.h include/json/cjson_utils.h | build
	$(CC) -c src/json/cjson_user.c -o build/cjson_user.o

build/cjson_utils.o: src/json/cjson_utils.c include/json/cjson_utils.h include/json/cJSON.h | build
	$(CC) -c src/json/cjson_utils.c -o build/cjson_utils.o

build/cJSON.o: src/json/cJSON.c include/json/cJSON.h | build
	$(CC) -c src/json/cJSON.c -o build/cJSON.o

# TICKET
build/ticket_list.o: src/ticket/ticket_list.c include/ticket/node.h | build
	$(CC) -c src/ticket/ticket_list.c -o build/ticket_list.o

build/ticket.o: src/ticket/ticket.c include/ticket/ticket.h | build
	$(CC) -c src/ticket/ticket.c -o build/ticket.o

# USER
build/user_list.o: src/user/user_list.c include/user/user_list.h include/user/user.h | build
	$(CC) -c src/user/user_list.c -o build/user_list.o

build/user.o: src/user/user.c include/user/user.h | build
	$(CC) -c src/user/user.c -o build/user.o

# CLIENT
build/client.o: src/client/client.c include/client/client.h include/client/ticket_operations.h include/client/user_operations.h include/utils/communication.h include/client/input_validation.h | build
	$(CC) -c src/client/client.c -o build/client.o

build/user_operations.o: src/client/user_operations.c include/client/user_operations.h include/utils/communication.h include/client/input_validation.h include/utils/config.h | build
	$(CC) -c src/client/user_operations.c -o build/user_operations.o

build/ticket_operations.o: src/client/ticket_operations.c include/client/ticket_operations.h include/utils/communication.h include/client/input_validation.h include/utils/config.h | build
	$(CC) -c src/client/ticket_operations.c -o build/ticket_operations.o

build/input_validation.o: src/client/input_validation.c include/client/input_validation.h include/utils/config.h | build
	$(CC) -c src/client/input_validation.c -o build/input_validation.o

# UTILS
build/communication.o: src/utils/communication.c include/utils/communication.h | build
	$(CC) -c src/utils/communication.c -o build/communication.o


# Pulizia
clean:
	rm -rf build bin