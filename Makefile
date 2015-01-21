CC=g++

CFLAGS=`sdl-config --libs --cflags`
INCLUDES=-I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
LDFLAGS=-L/opt/vc/lib -lbcm_host -lGLESv2 -lEGL -lSDL_image -ljson -lrt

ssm_client.bin: main.o opengl_es.o ssm_client.o json.o
	$(CC) -g -o ssm_client.bin main.o opengl_es.o ssm_client.o json.o $(INCLUDES) $(LDFLAGS) $(CFLAGS)

main.o : main.cpp opengl_es.h ssm_client.h
	$(CC) -c main.cpp $(INCLUDES) -lbcm_host -lSDL_image $(CFLAGS)
	
opengl_es.o : opengl_es.cpp opengl_es.h
	$(CC) -c opengl_es.cpp $(INCLUDES) -lSDL_image `sdl-config --libs --cflags`

ssm_client.o : ssm_client.cpp ssm_client.h json.h
	$(CC) -c ssm_client.cpp $(INCLUDES)

json.o : json.cpp json.h
	$(CC) -c json.cpp $(INCLUDES)

clean : 
	rm ssm_client.bin main.o opengl_es.o ssm_client.o json.o
