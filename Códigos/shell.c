#include "shell.h"

char **corte(char **argv, int inicio, int fim){
  	int i, tamanho;
  	tamanho = fim - inicio + 1;
  	char **vetor = (char**)malloc(tamanho*sizeof(char*));
  	for(i = 0; i<tamanho - 1;i++){
    	vetor[i] = argv[inicio+i];
    }
  	vetor[tamanho-1] = NULL;
	return vetor;
}

void handler(int sig){
    pid_t pid;
    pid = wait(NULL);
}

void fork_exec(char **argv, int background, char *name_arq_in, char *name_arq_out){
	pid_t pid;
	int status;
	FILE *arq, *arq2;

    if(background){
        signal(SIGCHLD,handler);
    }
	switch(pid = fork()){
		case -1:
			printf("Fork falhou!\n");
			exit(1);

		case 0:   // processo filho

			if(name_arq_out != NULL){
            	arq = freopen(name_arq_out, "w+", stdout);
              	if(arq == NULL){
                	perror("Error ");
                  	exit(1);
                }
            }
        	if(name_arq_in != NULL){
            	arq2 = freopen(name_arq_in, "r", stdin);
              	if(arq2 == NULL){
                	perror("Error ");
                  	exit(1);
                }
            }

            execvp(*argv, argv);
			perror("Error ");
			exit(1);

		default:    //processo pai
			if(!background){
        		while (wait(&status) != pid);
        	}else{
				printf("Programa executando em background\n");
			}
     }
}

void criaProcesso1(int pf[2], char **argv, char *name_arq_in){
    pid_t pid;
	int status;
	switch(pid = fork()){
		case -1:
			printf("Fork falhou!\n");
			exit(1);

		case 0:   // processo filho
			if(name_arq_in != NULL){
				FILE *arq = freopen(name_arq_in, "r", stdin);
				if(arq == NULL){
					perror("Error ");
					exit(1);
				}
			}
			close(pf[0]); // Fecha a leitura do pipe
			close(1); // Fecha a escrita padrão
			dup(pf[1]); // Substitui pela escrita no pipe

        	execvp(*argv, argv);
			perror("Error ");
			exit(1);

		default:    //processo pai
			close(pf[1]);
			waitpid(pid,&status,WUNTRACED);
     }
}

void criaProcesso2(int pf[2], char **argv, char *name_arq_out, int background){
	int status;
	pid_t pid;

    if(background){
        signal(SIGCHLD,handler);
    }

    switch(pid = fork()){
		case -1:
	        perror("Erro no processo 2");
	        exit(1);
    	case 0:
			if(name_arq_out != NULL){
				FILE *arq = freopen(name_arq_out, "w+", stdout);
				if(arq == NULL){
					perror("Error ");
					exit(1);
				}
			}
			close(pf[1]); // Fecha a escrita do pipe
			close(0); // Fecha a leitura padrão
	        dup(pf[0]); // Substitui pela leitura no pipe

			execvp(*argv, argv);
	        perror("execvp falhou");
	        exit(1);
		default:    //processo pai
			if(!background){
				close(pf[0]);
				waitpid(pid,&status,WUNTRACED);
			}else{
				close(pf[0]);
				printf("Programa executando em background.");
			}
    }
}

int execute(int argc, char **argv){
    int i,j;
	int status;

  	for(i=0;i<argc;i++){

		if(strcmp(argv[i], "=>") == 0){       // tratando o caso: comando => arquivo
			if((argc-i)>1 && i > 0){	// verifica se o tamanho do vetor é suficiente para conter um arquivo na frente e uma comando atrás

				if(i+2 == argc){				// se a linha já está no final
            		fork_exec(corte(argv,0,i),0,NULL,argv[i+1]); //passa arquivo para alterar a saída padrão
					return 1;
				}
				if(i+2 < argc){
					if(strcmp(argv[i+2], "&") == 0){
						fork_exec(corte(argv,0,i),1,NULL,argv[i+1]);
						return 1;
					}
				}
            }else{
            	printf("Error: No such file or directory\n");
          		return 0;
        	}
        }

      	if(strcmp(argv[i], "<=") == 0){       // tratando o caso: comando <= arquivo
			if((argc-i)>1 && i > 0){
				if(i+2 == argc){				// se não existem mais redirecionamentos
					fork_exec(corte(argv,0,i),0,argv[i+1],NULL); //passa arquivo para alterar a entrada padrão
					return 1;
				}
				if(i+2 < argc){
					if(strcmp(argv[i+2], "&") == 0){
						fork_exec(corte(argv,0,i),1,argv[i+1],NULL);
						return 1;
					}if(strcmp(argv[i+2], "=>") == 0){     // caso: comando <= arquivo => arquivo2
						fork_exec(corte(argv,0,i),1,argv[i+1],argv[i+3]);
						return 1;
					}if(strcmp(argv[i+2], "|") == 0){   // caso: comando <= arquivo | ...

						int pf[2];
					    if(pipe(pf) < 0){
					        perror("Nao criou o pipe pf");
					        return 0;
					    }

						criaProcesso1(pf,corte(argv,0,i),argv[i+1]);

						for(int j = i+3; j<argc;j++){
							if(strcmp(argv[j], "=>") == 0){       // tratando o caso: comando <= arquivo | comando => arquivo
								if((argc-j)>1){	// verifica se o tamanho do vetor é suficiente para conter um arquivo na frente e uma comando atrás

									if(j+2 == argc){				// se a linha já está no final
										criaProcesso2(pf,corte(argv,i+3,j),argv[j+1],0);
										return 1;
									}
									if(j+2 < argc){
										if(strcmp(argv[j+2], "&") == 0){   // tratando o caso: comando <= arquivo | comando => arquivo &
											criaProcesso2(pf,corte(argv,i+3,j),argv[j+1],1);
											return 1;
										}
									}
					            }else{
					            	printf("Error: No such file or directory\n");
					          		return 0;
					        	}
					        }
						}

						if(strcmp(argv[argc-1], "&") == 0)
							criaProcesso2(pf,corte(argv,i+3,argc-1),NULL,1);
						else
							criaProcesso2(pf,corte(argv,i+3,argc),NULL,0);

						return 1;
					}
				}

            }else{
            	printf("Error: No such file or directory\n");
          		return 0;
        	}
        }

		if(strcmp(argv[i], "|") == 0){       // tratando o caso: comando | ...
            if((argc-i)>1 && i > 0){	// verifica se o tamanho do vetor é suficiente para conter um comando na frente e outro atrás
                printf("opa\n");
                int pf[2]; // pipe
				// cria o pipe
				if(pipe(pf) < 0){
					perror("Nao criou o pipe pf");
					return 0;
				}

				criaProcesso1(pf,corte(argv,0,i),NULL);

				for(int j = i+1; j<argc;j++){
					if(strcmp(argv[j], "=>") == 0){       // tratando o caso: comando | comando => arquivo
						if((argc-j)>1){	// verifica se o tamanho do vetor é suficiente para conter um arquivo na frente e uma comando atrás

							if(j+2 == argc){				// se a linha já está no final
								criaProcesso2(pf,corte(argv,i+1,j),argv[j+1],0);
								return 1;
							}
							if(j+2 < argc){
								if(strcmp(argv[j+2], "&") == 0){   // tratando o caso: comando | comando => arquivo &
									criaProcesso2(pf,corte(argv,i+1,j),argv[j+1],1);
									return 1;
								}
							}
						}else{
							printf("Error: No such file or directory\n");
							return 0;
						}
					}
				}

				if(strcmp(argv[argc-1], "&") == 0)                // comando | comando &
					criaProcesso2(pf,corte(argv,i+1,argc-1),NULL,1);
				else											  // comando | comando
					criaProcesso2(pf,corte(argv,i+1,argc),NULL,0);

				return 1;

            }else{
            	printf("Error: No such file or directory\n");
          		return 0;
        	}
        }
    }

  	if(strcmp(argv[argc-1], "&") == 0)
    	fork_exec(corte(argv,0,argc-1),1,NULL,NULL); // ativa o sinal background
    else
		fork_exec(argv,0,NULL,NULL);

	return 1;
}
