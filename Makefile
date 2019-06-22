all: hdsmon hdscmon hdstalk hdsjtalk

hdsjtalk: hdsjtalk.o
	gcc -o hdsjtalk hdsjtalk.o `pkg-config glib-2.0 --libs` --input-charset=UTF-8 --exec-charset=UTF-8 -lm

hdsjtalk.o: hdsjtalk.c
	gcc -c hdsjtalk.c `pkg-config glib-2.0 --cflags` --input-charset=UTF-8 --exec-charset=UTF-8

hdstalk: hdstalk.o
	gcc -o hdstalk hdstalk.o `pkg-config glib-2.0 --libs` -lm

hdstalk.o: hdstalk.c
	gcc -c hdstalk.c `pkg-config glib-2.0 --cflags`

hdscmon: hdscmon.o
	gcc -o hdscmon hdscmon.o  `pkg-config glib-2.0 --libs` -lcurses -lm

hdscmon.o: hdscmon.c
	gcc -c hdscmon.c  `pkg-config glib-2.0 --cflags`

hdsmon: hdsmon.o gtkut.o efs.o
	gcc -o hdsmon hdsmon.o gtkut.o efs.o `pkg-config gtk+-3.0 --libs` `pkg-config glib-2.0 --libs` `pkg-config cairo --libs` -lm

hdsmon.o: hdsmon.c
	gcc -c hdsmon.c `pkg-config gtk+-3.0 --cflags` `pkg-config glib-2.0 --cflags`

gtkut.o: gtkut.c
	gcc -c gtkut.c `pkg-config gtk+-3.0 --cflags` `pkg-config glib-2.0 --cflags`

efs.o: efs.c
	gcc -c efs.c `pkg-config gtk+-3.0 --cflags` `pkg-config glib-2.0 --cflags` `pkg-config cairo --cflags`

clean:
	rm -f *.o hdsmon hdscmon hdstalk *~
