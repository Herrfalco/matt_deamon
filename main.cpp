#include <iostream>
#include <unistd.h>

int		main(void) {
	if (fork())
		exit(0);
	setsid();
	while (true);
	return (0);
}
