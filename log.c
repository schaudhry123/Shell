// Shell
// CS 241 Fall 2015
#define _GNU_SOURCE 1
#include "log.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Log *Log_create(const char *filename) {
	Log *log = malloc(sizeof(Log));
	log->vtr = Vector_create();
	if (filename != NULL && access(filename, R_OK) == 0) {
		FILE *file = fopen(filename, "r+");
		if (file != NULL) {
			char *buffer = NULL;
			size_t size = 0;
			while (getline(&buffer, &size, file) != -1) {
				removeNewLines(buffer);
				Vector_append(log->vtr, buffer);
			}
			free(buffer);
		}
		fclose(file);
	}
	return log;
}

int Log_save(Log *log, const char *filename) {
	if (filename != NULL) {
		FILE *file = fopen(filename, "w+");
		if (file != NULL && access(filename, W_OK) == 0) {
			for (int i = 0; i < Vector_size(log->vtr); i++) {
				const char * line = Vector_get(log->vtr, i);
				fprintf(file, "%s\n", line);
			}
			fclose(file);
		}
		return 0;
	}
	return 1;
}

void Log_destroy(Log *log) {
	Vector_destroy(log->vtr);
	free(log);
	log = NULL;
}

void Log_add_command(Log *log, const char *command) {
	if (command != NULL && command[0] != '\0')
		Vector_append(log->vtr, command);
}

const char *Log_reverse_search(Log *log, const char *prefix) {
	if (log != NULL && prefix != NULL) {
		for (int i = Vector_size(log->vtr) - 1; i >= 0; i--) {
			const char *str = Vector_get(log->vtr, i);
			char *substr = strndup(str, strlen(prefix));
			if (substr != NULL && strcmp(substr, prefix) == 0) {
				free(substr);
				return str;
			}
			free(substr);
		}
	}
	return NULL;
}

const char *Log_get_command(Log *log, size_t index) {
	if ((int)index < Vector_size(log->vtr)) {
		return Vector_get(log->vtr, index);
	}
	return NULL;
}

char *Log_get_printable_history(Log *log) {
	char *str;
	int size = Vector_size(log->vtr);
	int len = 0;
	for (int i = 0; i < size; i++) {
		len += strlen(Vector_get(log->vtr, i)) + 10;
	}
	str = calloc(1, len);
	//fprintf(stderr, "Log_get_printable_history: Size is %d\n", size);
	for (int i = 0; i < size; i++) {
		const char * temp = Vector_get(log->vtr, i);
		char bruh[strlen(temp)+10];
		sprintf(bruh, "%d\t%s\n", i, Vector_get(log->vtr, i));
		strcat(str, bruh);
	}
	return str;
}