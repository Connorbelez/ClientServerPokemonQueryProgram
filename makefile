all: PPS PQC serverUtil clientUtil
	gcc -o PPS PPS.o serverUtil.o
	gcc -o PQC PQC.o clientUtil.o -lpthread

PPS: PPS.c serverUtil.c serverUtil.h
	gcc -c PPS.c serverUtil.c

PQC: PQC.c clientUtil.c clientUtil.h
	gcc -c PQC.c clientUtil.c -lpthread

serverUtil: serverUtil.c serverUtil.h
	gcc -c serverUtil.c -lpthread

clientUtil: clientUtil.c clientUtil.h
	gcc -c clientUtil.c -lpthread




