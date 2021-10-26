
CC = gcc
CFLAGS = -std=c11
FLAGS = -g -O1

VPATH = simulator utils utils/layer_cores utils/img_process utils/post_process 
INC = -I./ -I ./include
LIBS = -lm -lpthread `pkg-config opencv --cflags --libs`

CXXFLAGS = ${CFLAGS} ${INC} ${LIBS} ${FLAGS}

EXEC = yolo
OBJS = yolo_simulate.o simulator.o cores.o weight.o cores_utils.o data_carrier.o data_loader.o \
	config.o img_process.o box.o decode.o post_utils.o 

${EXEC} : ${OBJS}
	${CC} -o $@ $^ ${CXXFLAGS}

clean:
	rm -rf ${OBJS}

uninstall:
	rm ${EXEC}

%.o : %.c
	$(CC) -o $@ -c $< $(CXXFLAGS)