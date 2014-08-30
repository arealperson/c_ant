ant: ant.c
	gcc -Wall -Wextra -O2 -g -o $@ $<

clean:
	@rm -f *~ ant
