build:
	gcc -o main main.c -lpthread -lm


listen: build
	sox synth_bpm100.wav -b 16 -c 1 -r 44100 -e signed-integer 1.raw pad 0 1
	cat 1.raw | ./main | aplay -t raw -c 1 -f s16 -r 44100

leaks: build
	valgrind --track-origins=yes --tool=memcheck ./main > /dev/null

prereqs:
	sudo apt-get install libsdl2-dev