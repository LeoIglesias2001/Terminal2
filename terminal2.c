#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void ciclo();
char  *ler_linha(void);
char **dividir_linha(char *linha);
int iniciar(char **args);
int executar(char **args);

//Declaracao de funcoes para comandos builtin da shell:
int td(char **args);
int ajuda(char **args);
int sair(char **args);

int main (int argc, char **argv){
	ciclo();
	return EXIT_SUCCESS;
}

//loop onde ocorre a leitura de comandos
void ciclo(void){

    char *linha;
    char **args;
    int status;

    do {
        printf("> ");
        linha = ler_linha();
        args = dividir_linha(linha);
        status = executar(args);

        free(linha);
        free(args);
    } while (status);
}

#define TAM_STRING 1024

//funcao que realiza a leitura dos comandos
char *ler_linha(void){

    int tamString = TAM_STRING;
    int pos = 0;
    char *buffer = malloc(sizeof(char) * tamString);
    int c;

    if (!buffer) {
        fprintf(stderr, "erro de alocacao\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[pos] = '\0';
            return buffer;
        } else {
            buffer[pos] = c;
        }
        pos++;

        if (pos >= tamString) {
            tamString += TAM_STRING;
            buffer = realloc(buffer, tamString);
            if (!buffer) {
                fprintf(stderr, "erro de alocaçao\n");
                exit(EXIT_FAILURE);
            }
        }   
    }
}

#define TOK_TAM_STRING 64
#define LIM_TOK " \t\r\n\a"

//funcao que retorna um vetor de tokens a partir da string
char **dividir_linha(char *linha){

    int tamString = TOK_TAM_STRING, pos = 0;
    char **tokens = malloc(tamString * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "erro de alocacao\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(linha, LIM_TOK);
    while (token != NULL) {
        tokens[pos] = token;
        pos++;

        if (pos >= tamString) {
            tamString += TOK_TAM_STRING;
            tokens = realloc(tokens, tamString * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "erro de alocacao\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LIM_TOK);
    }
    tokens[pos] = NULL;
    return tokens;
}

//funcao que inicia a shell
int iniciar(char **args){

    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Processo filho
        if (execvp(args[0], args) == -1) {
            perror("Comando inexistente!");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        // Erro no fork()
        perror("Erro ao realizar o fork()");
    } 
    else {
        // Processo pai
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}



//Lista dos comandos builtin, seguidos de suas funcoes correspondentes. 
char *builtin_str[] = {
    "td",
    "ajuda",
    "sair"
};

int (*builtin_func[]) (char **) = {
    &td,
    &ajuda,
    &sair
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

//Implementacao das funcoes builtin.
int td(char **args){

    if (args[1] == NULL) {
        fprintf(stderr, "Argumento esperado para \"td\"\n");
    }
    else {
        if (chdir(args[1]) != 0) {
            perror("Erro");
        }
    }
    return 1;
}

int ajuda(char **args){

    int i;
    printf("Terminal feito com por Leonardo Iglesias, com o auxilio deste material: https://brennan.io/2015/01/16/write-a-shell-in-c/\n");
    printf("Digite nomes de programas e argumentos e aperte ENTER.\n");
    printf("Os comandos a seguir são builtin:\n");

    for (i = 0; i < num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use o comando man para informação em outros programas.\n");
    return 1;
}

int sair(char **args){
    return 0;
}

int executar(char **args){
    
    int i;

    if (args[0] == NULL) {
        // Enviaram um comando em branco
        return 1;
    }

    for (i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return iniciar(args);
    }
