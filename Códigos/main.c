#include "shell.h"

void parse(char *linha, int *argc, char **argv){
    int i = 0;
    *argc=-1;
    while (linha[i] != '\0'){
        while(linha[i] == ' ' || linha[i] == '\t' || linha[i] == '\n'){    // pula eventuais espaços, tabulações e quebra de linhas iniciais
            linha[i] = '\0';
            i++;
        }

        if(linha[i] != '\0'){
            *argc = *argc+1;
            argv[*argc] = linha+i*sizeof(char);   // salva o início do parâmetro
        }

        while(linha[i] != '\0' && linha[i] != ' ' && linha[i] != '\t' && linha[i] != '\n'){
            i++;
        }
    }
    *argc = *argc+1;
    argv[*argc] = NULL;
}

int main(int argc, char **argv){
    pid_t pid;
    int status;
    FILE *arq;
    char linha[512];
    int argc2;
    char **argv2 = (char**)malloc(20*sizeof(char*));

    if(argc > 1){
        arq = freopen(argv[1],"r",stdin);
        if(arq == NULL){
            perror("Error: ");
            return(-1);
        }
    }

    int i;

    if(argc == 1)
        printf("Sim, mestre? ");

    while(fgets(linha,512,stdin)){
        if(argc > 1)
            if(feof(stdin))
                break;

        parse(linha, &argc2, argv2);

        if(argc2 == 0){
            printf("Linha em branco!\n");
        }else{
            if (strcmp(argv2[0], "fim") == 0){
                return 0;
            }

            execute(argc2, argv2);
        }
        if(argc == 1)
            printf("\nSim, mestre? ");
    }

    if(argc > 1)
        fclose(arq);
}
