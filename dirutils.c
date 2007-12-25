#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dirconsts.h"

/*
void print_dirent(const char *dirname, const struct dirent *ent) {
	printf("%s %s\n", dirname, ent->d_name);
}
*/

char *combine_paths(const char *p1, const char *p2) {
	int req_length = strlen(p1) + 1 + strlen(p2) + 1;
	char *final = malloc(req_length * sizeof(char));

	if (!final) 
		return NULL;

	sprintf(final, "%s%c%s", p1, DIR_SEPARATOR, p2);
	return final;
}

int walk_entries(const char *dirname,
		  bool (*go_deep)(const struct dirent *),
		  void (*visitor)(const char *, const struct dirent *, const struct stat *, const void *),
		  const void *data) {

	DIR *dir = opendir(dirname);
	struct dirent *current_entry;
	struct stat statbuf;

	if (dir == NULL)
		return -1;

	while ((current_entry = readdir(dir)) != NULL) {
		char *entrypath = combine_paths(dirname, current_entry->d_name);

		if (!entrypath)
			return -1;

		if (stat(entrypath, &statbuf) == -1)
			continue;
		
		visitor(dirname, current_entry, &statbuf, data);

		if (S_ISDIR(statbuf.st_mode) && go_deep(current_entry))
			walk_entries(entrypath, go_deep, visitor, data);
		
		free(entrypath);
	}

	return closedir(dir);
}

/*
int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("error, please specify as first parameter the directory name\n");
		return -1;
	}

	walk_entries(argv[1], WALK_ENTRIES_ALWAYS_GO_DEEP, print_dirent);

	return 0;
}
*/
