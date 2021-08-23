all: schedule
schedule: schedule.c
	gcc -pthread -o schedule schedule.c -lm
	
clean:
	rm schedule
