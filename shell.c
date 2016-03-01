// Shell
// CS 241 Fall 2015
#define _GNU_SOURCE 1

#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

/**
 * Prints the helpful text when starting up the shell
 */
void printIntro() {
	printf("Welcome to my shell!\n");
	printf("Type 'help' for a helpful list of commands you can run!\n");
}

/*
 * Prints the command prompt
 */
void printCommandPrompt() {
	char path[PATH_MAX];
	if ( getcwd(path, sizeof(path)) != NULL)
		printf("(pid=%d)%s$ ", getpid(), path);
}

/*
 * Prints the command
 */
void printCommand(char *command) {
	printf("%s\n", command);
}

/*
 * Gets the params from the command
 */
char *getCommandParameters(char *command) {
	char *params = strstr(command, " ");
	//fprintf(stderr, "getCommandParameters: %s\n", params);
	if (params != NULL)
		params = strdup(params+1);
	return params;
}

/*
 * Gets just the command
 */
char *getCommand(char *command) {
	char *params = strstr(command, " ");
	char *cmnd = calloc(1, strlen(command) + 1);
	if (params != NULL) {
		params = params + 1;
		strncpy(cmnd, command, strlen(command) - strlen(params) - 1);
	}
	else
		strcpy(cmnd, command);
	// fprintf(stderr, "getCommand: cmnd is %s.\n", cmnd);
	return cmnd;
}

/*
 * Creates an array for use in execvp
 */
char ** getCommandArray(char *command) {
	int index = 0;
	char **arr = calloc(index+1, sizeof(char *));
	//arr[0] = getCommand(command);
	arr[index] = strtok(command, " ");
	while (arr[index] != NULL) {
		index++;
		arr = realloc(arr, (index+1) * sizeof(char *));
		arr[index] = strtok(NULL, " ");
	}

	/*
	fprintf(stderr, "Array should be size %d:\n", index);
	for (int i = 0; i < index; i++) {
		fprintf(stderr, " %s ", arr[i]);
	}
	fprintf(stderr, "\n");
	*/
	return arr;
}

/*
 * Frees an array of size length
 */
void freeArray(char **arr, int length) {
	for (int i = 0; i < length; i++) {
		free(arr[i]);
	}
	free(arr);
}

/*
 * Determine if command is a built in command (not cd, !history, !#, or !query)
 */
int builtInCommand(char *command) {
	char *cmnd = getCommand(command);
	int builtIn = 0;
	if (strcmp(cmnd, "cd") == 0)
		builtIn = 1;
	else if (command[0] == '!') {
		char c = command[1];
		if (strcmp(command, "!history") == 0)
			builtIn = 2;
		else if (isdigit(c))
			builtIn = 3;
		else if (isalpha(c))
			builtIn = 4;
	}
	else if (strcmp(command, "exit") == 0)
		builtIn = 5;
	else if (strcmp(command, "help") == 0)
		builtIn = 6;
	free(cmnd);
	return builtIn;
}

/*
 * Runs the cd command
 */
void run_cd(Log *log, char *command, int add) {
	//char *params = getCommandParameters(command);
	char *params = strstr(command, " ");
	// fprintf(stderr, "Command is %s and params is %s\n", command, params);
	if (params != NULL) {
		params = params + 1;
		// fprintf(stderr, "run_cd: params is %s\n", params);
		if (add != 0)
			Log_add_command(log, command);
		if (chdir(params) == -1)
			perror(params);
	}
}

/*
 * Runs the history command
 */
void run_history(Log *log, char *command) {
	char *history = Log_get_printable_history(log);
	printf("%s", history);
	free(history);
}

/*
 * Adds the command to log, forks and executes the command
 */
int runCommand(Log *log, char *command, int add) {
	if (strlen(command) > 0) {
		int background = 0;
		if (add != 0)
			Log_add_command(log, command);
		char *c = strstr(command, "&");
		if (c != NULL) {
			*c = '\0';
			background = 1;
		}
		if (strncmp(command, "cd", 2) == 0) {
			run_cd(log, command, 0);
			return 0;
		}
		if (strcmp(command, "exit") == 0)
			return 1;
		char **arr = getCommandArray(command);
		pid_t pid = fork();
		if (pid == -1) { // Failed fork
			perror("Fork failed");
			exit(1);
		}
		else if (pid == 0) { // Child process
			printf("Command executed by pid=%d\n", getpid());
			int result = execvp(*arr, arr);
			if (result == -1)
				printf("%s: not found\n", command);
			//freeArray
			exit(0);
		}
		else if (pid > 0) { // Parent process
			int status;
			if (background == 0)
				waitpid(pid, &status, 0);
			free(arr);
			// fprintf(stderr, "Finished running command\n");
		}
	}
	return 0;
}

/*
 * Runs the previous command at #
 */
int run_indexedCommand(Log *log, char *command, int add) {
	const char *temp = Log_get_command(log, atoi(command));
	int exit = 0;
	if (temp != NULL) {
		char *search = calloc(1, strlen(temp) + 1);
		strcpy(search, temp);
		// printf("%s\n", search);
		exit = runCommand(log, search, add);
		free(search);
	}
	else
		printf("Invalid Index\n");
	return exit;
}

/*
 * Runs the latest command in log with prefix comand
 */
int run_queriedCommand(Log *log, char *command, int add) {
	const char *temp = Log_reverse_search(log, command);
	int exit = 0;
	if (temp != NULL) {
		char *query = strdup(temp);
		printf("%s\n", query);
		exit = runCommand(log, query, add);
		free(query);
	}
	else
		printf("No Match\n");
	return exit;
}

/**
 * Prints out list of commands that can be run
 */
void run_help() {
	printf("This shell can take run any commands that a normal terminal can. In addition, it includes some built-in commands, including\n");
	printf("\t!history - gives you a history of the previously typed in commands\n");
	printf("\t#<n> - runs the command at the line number n in your history file\n");
	printf("\t!<prefix> - runs the most recent command with the specified prefix\n");
	printf("\texit - exits the shell and saves your history\n");
}

/*
 * Runs a built in command based on int builtIn
 */
int runBuiltIn(Log *log, char *command, int builtIn, int add) {
	if (builtIn == 1)
		run_cd(log, command, add);
	else if (builtIn == 2)
		run_history(log, command);
	else if (builtIn == 3) {
		return run_indexedCommand(log, command + 1, add);
	}
	else if (builtIn == 4) {
		return run_queriedCommand(log, command + 1, add);
	}
	else if (builtIn == 5) {
		return 1;
	}
	else if (builtIn == 6) {
		run_help();
	}
	return 0;
}

int runBuiltOrCommand(Log *log, char *command) {
	int builtIn = builtInCommand(command);
	int exited = 0;
	if (builtIn > 0)
		exited = runBuiltIn(log, command, builtIn, 1);
	else
		exited = runCommand(log, command, 1);
	if (exited == 1)
		return 1;
	return 0;
}

/*
 * Reads commands in from stdin until "exit" is entered
 */
void readCommandsFromStdin(Log *log) {
	char *command = NULL;
	size_t size = 0;
	int exited = 0;
	printIntro();
	printCommandPrompt();
	while ( exited == 0 && getline(&command, &size, stdin) != -1 ) {
		removeNewLines(command);
		if (strlen(command) > 0)
			exited = runBuiltOrCommand(log, command);
		if (exited == 0)
			printCommandPrompt();
		// fprintf(stderr, "Input is %s\n", command);
	}
	Log_add_command(log, command);
	free(command);
}

/*
 * If filename exists, then load the commands from filename
 * Else, read in commands from stdin
 */
Log * loadCommandsFromFile(char *filename) {
	Log *log = NULL;
	if (filename != NULL) {
		Log *log = Log_create(filename);
		int size = Vector_size(log->vtr);
		int exited = 0;
		for (int i = 0; i < size; i++) {
			char *command = strdup(Log_get_command(log, i));
			printCommandPrompt();
			printCommand(command);
			if (exited == 1) {
				Log_add_command(log, command);
				break;
			}
		 	int builtIn = builtInCommand(command);
		 	if (builtIn == 5) { // Exit
		 		exited = 1;
		 		free(command);
		 		break;
		 	}
			if (builtIn > 0)
				exited = runBuiltIn(log, command, builtIn, 0);
			else
				exited = runCommand(log, command, 0);
			free(command);
		}
		if (exited == 0) {
			printCommandPrompt();
			Log_add_command(log, "exit");
			printCommand("exit");
		}
		Log_save(log, filename);
		Log_destroy(log);
		exit(0);
	}
	else
		log = Log_create("commands.txt");
	return log;
}

/*
 * If there are multiple arguments, check for -h options and -f option
 * If -h, printf and begin accepting inputs
 * If -f, run the commands from the file and exit
 */
Log * getInitialArguments(int argc, char *argv[]) {
	char *filename = NULL;
	if (argc > 1) {
		int c;
		while ((c = getopt(argc, argv, "hf:")) != -1) {
			switch (c) {
				case 'h':
					printf("Shell by srchaud2\n");
					break;
				case 'f':
					filename = optarg;
					break;
			}
		}
		// fprintf(stderr, "Filename is %s\n", filename);
	}
	return loadCommandsFromFile(filename);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, SIG_IGN);
	Log *log = getInitialArguments(argc, argv);
	readCommandsFromStdin(log);
	Log_save(log, "commands.txt");
	Log_destroy(log);
	return 0;
}