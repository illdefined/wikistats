#include <string.h>
#include <unistd.h>

int devour(int, char*[]);
int vomit(int, char*[]);

static const char info[] = "[c]wikistats version 0.4.0\n\nAvailable applets:\n  devour\n  merge\n  vomit\n";

#define out(str) write(1, str, sizeof (str) - 1);
#define err(str) write(2, str, sizeof (str) - 1);

int main(int argc, char *argv[]) {
	if (argc >= 2) {
		if (!strcmp(argv[1], "devour"))
			return devour(--argc, ++argv);
/*		else if (!strcmp(argv[1], "merge"))
			;*/
		else if (!strcmp(argv[1], "vomit"))
			return vomit(--argc, ++argv);
		else {
			err(info);
			return 1;
		}
	}
	
	out(info);
	return 0;
}
