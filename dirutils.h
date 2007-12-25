#ifndef __DIRUTILS_H
#define __DIRUTILS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h> /* for bool */

#include "dirconsts.h"

bool WALK_ENTRIES_ALWAYS_GO_DEEP(const struct dirent *ent) {
	/* dont go deep on . or .. */
	return !(ent->d_name[0] == '.' && 
			(ent->d_name[1] == '\0' || 
			 (ent->d_name[1] == '.' && 
			  ent->d_name[2] == '\0')));
}

char *combine_paths(const char *, const char *);

int walk_entries(const char *,
		  bool (*go_deep)(const struct dirent *),
		  void (*visitor)(const char *, const struct dirent *, const struct stat *, const void *),
		  const void *);

#endif /* __DIRUTILS_H */
