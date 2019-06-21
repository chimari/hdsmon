hdstalk: hdstalk.o
	gcc -o hdstalk hdstalk.o `glib-config --libs`

hdstalk.o: hdstalk.c
	gcc -c hdstalk.c `glib-config --cflags`

hdscmon: hdscmon.o
	gcc -o hdscmon hdscmon.o  `glib-config --libs` -lcurses

hdscmon.o: hdscmon.c
	gcc -c hdscmon.c  `glib-config --cflags`

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
