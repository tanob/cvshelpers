#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dirutils.h"


typedef struct s_change_root s_change_root;

struct s_change_root {
	char *new_root;
	bool success;
};


int deltree(const char *dirname) {
	DIR *dir = opendir(dirname);
	struct dirent *current_entry;

	if (dir == NULL)
		return -1;

	while ((current_entry = readdir(dir)) != NULL) {
		if ((current_entry->d_name[0] == '.' && current_entry->d_name[1] == 0) ||
		    (current_entry->d_name[0] == current_entry->d_name[1] && 
		     current_entry->d_name[1] == '.' && current_entry->d_name[2] == 0))
			continue;

		char *entry_path = combine_paths(dirname, current_entry->d_name);

		if (!entry_path)
			return -1;

		remove(entry_path);

		free(entry_path);
	}

	closedir(dir);
	return rmdir(dirname);
}

void deltree_CVS_directory(const char *dirname, const struct dirent *ent, const struct stat *statbuf, const void *data) {
	if (!S_ISDIR(statbuf->st_mode) && !strcmp("CVS", ent->d_name)) {
		bool *success = (bool *) data;
		char *entry = combine_paths(dirname, ent->d_name);

		if (!entry)
			return;

		printf("Disconnecting %s\n", entry);
		bool success_deltree = !deltree(entry);
		*success = *success && success_deltree;

		free(entry);
	}
}

int cvs_disconnect(const char *dirname) {
	bool success = true;
	walk_entries(dirname, WALK_ENTRIES_ALWAYS_GO_DEEP, deltree_CVS_directory, (const void *)&success);
	return success ? 0 : -1;
}

void change_CVS_root(const char *dirname, const struct dirent *ent, const struct stat *statbuf, const void *data) {
	if (!S_ISDIR(statbuf->st_mode) && !strcmp(ent->d_name, "Root")) {
		int n = strlen(dirname);
		if (n >= 3 && !strcmp(&dirname[n-3], "CVS")) {
			s_change_root *pdata = (s_change_root *) data;
			char *root_filename = combine_paths(dirname, ent->d_name);
			FILE *root_file = fopen(root_filename, "w");

			if (!root_file) {
				pdata->success = false;
				goto error_fopen;
			}

			fprintf(root_file, "%s\n", pdata->new_root);
			pdata->success = true && pdata->success;

			printf("* %s\n", dirname);

			fclose(root_file);
error_fopen:		free(root_filename);
		}
	}
}

int cvs_change_root(const char *dirname, const char *new_root) {
	s_change_root data;
	data.new_root = (char *) new_root;
	data.success = true;

	walk_entries(dirname, WALK_ENTRIES_ALWAYS_GO_DEEP, change_CVS_root, (const void *)&data);
	return data.success ? 0 : -1;
}

void print_usage() {
	fprintf(stdout, "\nUsage:\n\
  cvshelpers --disconnect directory\n\
        This will recursively disconnect the CVS files inside the\n\
        'directory' parameter.\n\n\
	Example: cvshelpers --disconnect my_project_folder\n\n\
  cvshelpers --changeroot directory new_cvs_root\n\
        This will recursively change the CVS root of the files inside the\n\
        'directory' parameter.\n\n\
	Example: cvshelpers --changeroot my_project_folder \":extssh:user@host.org:/var/lib/cvs\"\n\n");
}

int main(int argc, char *argv[]) {
	if (argc < 3 || argc > 4) {
		print_usage();
		return -1;
	}
	
	int sc = -1;

	if (!strcmp(argv[1], "--disconnect") && argc == 3) {
		fprintf(stdout, "Disconnecting '%s' from CVS.\n", argv[2]);
		sc = cvs_disconnect(argv[2]);
	}
	else if (!strcmp(argv[1], "--changeroot") && argc == 4) {
		fprintf(stdout, "Changing CVS root for '%s' to '%s'.\n", argv[2], argv[3]);
		sc = cvs_change_root(argv[2], argv[3]);
	}
	else
		print_usage();

	return sc;
}

