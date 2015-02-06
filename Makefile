prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
#datarootdir=$(prefix)/share
localstatedir=$(prefix)/var


#..#


CC=g++

CFLAGS=`sdl-config --libs --cflags`
INCLUDES=-I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
LDFLAGS=-L/opt/vc/lib -lbcm_host -lGLESv2 -lEGL -lSDL_image -ljson -lrt

OBJECTFILES=main.o opengl_es.o ssm_client.o json.o ssm_server_scanner.o


DATAFOLDER=$(localstatedir)/snapshowmagic
IMAGESFOLDER=$(DATAFOLDER)/images
DOWNLOADSFOLDER=$(DATAFOLDER)/downloads
SERVERHISTORY=$(DATAFOLDER)/server_history


#..#


ssm_client: $(OBJECTFILES)
	$(CC) -g -o ssm_client $(OBJECTFILES) $(INCLUDES) $(LDFLAGS) $(CFLAGS)

main.o : main.cpp opengl_es.h ssm_client.h ssm_server_scanner.h
	$(CC) -c main.cpp $(INCLUDES) -lbcm_host -lSDL_image $(CFLAGS)

opengl_es.o : opengl_es.cpp opengl_es.h
	$(CC) -c opengl_es.cpp $(INCLUDES) -lSDL_image `sdl-config --libs --cflags`

ssm_client.o : ssm_client.cpp ssm_client.h json.h os_settings.h
	$(CC) -c ssm_client.cpp $(INCLUDES)

json.o : json.cpp json.h
	$(CC) -c json.cpp $(INCLUDES)

ssm_server_scanner.0 : ssm_server_scanner.cpp ssm_server_scanner.h json.h os_settings.h
	$(CC) -c ssm_server_scanner.cpp $(INCLUDES)

clean : 
	@rm ssm_client main.o opengl_es.o ssm_client.o json.o ssm_server_scanner.o

install : ssm_client
	@echo Installing the binary and make it executable...
	@cp --force ssm_client $(bindir)
	@chmod 755 $(bindir)/ssm_client

	@echo Creating data folders...
	@mkdir --parents "$(IMAGESFOLDER)"
	@mkdir --parents "$(DOWNLOADSFOLDER)"
	@chmod -R a+rwX "$(DOWNLOADSFOLDER)"

	@echo Installing essential data files...
	@# (The default black image could, arguably, belong in datarootdir because it is never modified by the program.)
	@cp ./images/black.png "$(IMAGESFOLDER)"
	@touch "$(SERVERHISTORY)"
	@chmod a+rw "$(SERVERHISTORY)"

	@echo Done!

