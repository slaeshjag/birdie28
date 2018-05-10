//fuck da police
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <darnit/darnit.h>


char *util_binrel_path(const char *file) {
	char *path;
	char *pathpart;
	
	pathpart = strdup(d_fs_exec_path());

	asprintf(&path, "%s/%s", dirname(pathpart), file);
	free(pathpart);

	return path;
}
