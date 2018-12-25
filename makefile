#This sample makefile has been setup for a project which contains the following files: 
#      aqualinkd.c
#      aqualink.h
#      aqualink_temps.c
#      aqualink_time.h  
#      json_messages.c  
#      logging.c  
#      aqualinkd.conf  
#      aqualink_menu.c  
#      aqualink_temps.h  
#      globals.c        
#      json_messages.h  
#      logging.h
#      aqualinkd.h     
#      aqualink_menu.h  
#      aqualink_time.c   
#      globals.h        

#Change output_file_name.a below to your desired executible filename

#Set all your object files (the object files of all the .c files in your project, e.g. main.o my_sub_functions.o )
OBJ = aqualinkd.o  aqualink_menu.o  aqualink_temps.o  aqualink_time.o  globals.o  json_messages.o  logging.o

#Set any dependant header files so that if they are edited they cause a complete re-compile (e.g. main.h some_subfunctions.h some_definitions_file.h ), or leave blank
DEPS = aqualinkd.h  aqualink_menu.h   aqualink_time.h  json_messages.h  logging.h aqualink.h   aqualink_temps.h  globals.h

#Any special libraries you are using in your project (e.g. -lbcm2835 -lrt `pkg-config --libs gtk+-3.0` ), or leave blank
LIBS = -lpthread -lz

#Set any compiler flags you want to use (e.g. -I/usr/include/somefolder `pkg-config --cflags gtk+-3.0` ), or leave blank
#CFLAGS = -lrt

#Set the compiler you are using ( gcc for C or g++ for C++ )
CC = gcc

#Set the filename extensiton of your C files (e.g. .c or .cpp )
EXTENSION = .c

#define a rule that applies to all files ending in the .o suffix, which says that the .o file depends upon the .c version of the file and all the .h files included in the DEPS macro.  Compile each object file
%.o: %$(EXTENSION) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#Combine them into the output file
#Set your desired exe output file name here
aqualinkd.a: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

#Cleanup
.PHONY: clean

clean:
	rm -f *.o *~ core *~ 
