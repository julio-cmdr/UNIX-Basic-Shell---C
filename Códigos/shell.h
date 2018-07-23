#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

char **corte(char **argv, int inicio, int fim);

void fork_exec(char **argv, int background, char *name_arq_in, char *name_arq_out);

void criaProcesso1(int pf[2], char **argv, char *name_arq_in);

void criaProcesso2(int pf[2], char **argv, char *name_arq_out, int background);

int execute(int argc, char **argv);
