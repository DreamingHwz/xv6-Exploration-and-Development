#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MaxCommandLen 1000
#define MaxTokenLen 100
#define MaxFileNameLen 50
#define MaxPathLen 50
int DoCmd(char *[], int);

char * CommandLines[MaxCommandLen];
int LineNum = 0;
char * Tokens[MaxTokenLen];
int TokenNum = 0;
char * PATH[MaxPathLen];
int PathNum = 0;

void PrintPrompt(){
    printf("wish> ");
}

void Free(char * arr[], int len){
    for (int p = 0; p < len; p++) free(arr[p]);
}

int Error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    return 0;
}

void SubTokensCpy (char * dest[], char * src[], int start, int end){
    for (int j = start; j < end; j ++){
        dest[j - start] = malloc(strlen(src[j]));
        strcpy(dest[j - start], src[j]);
    }
}

void Exit(){
    Free(Tokens, TokenNum);
    Free(CommandLines, LineNum);
    Free(PATH, PathNum);
    exit(0);
}

void GetCommands(){
    int read;
    char * line = NULL;
    size_t len = 0;
    //getline() == -1 when touch EOF
    if (getline(&line, &len, stdin) != -1){
        if (CommandLines[0] != NULL)
            free(CommandLines[0]);

        CommandLines[0] = malloc(len);
        strcpy(CommandLines[0], line);
        LineNum = 1;

        free(line);
    }
    else exit(0);
}

void GetBatchCommands(char * Name){
    char FileName[MaxFileNameLen];
    strcpy(FileName, Name);
    FILE * fp = fopen(FileName, "r");
    if (fp == NULL) {
        Error();
        exit(1);
    }
    //getline() and store each line
    int read;
    char * line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) != -1){
        CommandLines[LineNum] = malloc(len);
        strcpy(CommandLines[LineNum], line);
        LineNum ++;
    }
    fflush(fp);
    fclose(fp);
}

int Have(char * str, char c){
    int ret = -1;
    for (int i = 0; i < strlen(str); i ++){
        if (str[i] == c){
            if (ret == -1) ret = i;
            else return -2;
        }
    }
    return ret;
}

void AddtoTokens(char * str){
    Tokens[TokenNum] = malloc(strlen(str));
    strcpy(Tokens[TokenNum], str);
    TokenNum ++;
}

int Cd(char * tokens[]){
    if (chdir(tokens[1]) == -1){
        printf("%s\n",strerror(errno));
    }
    return 0;
}

int AddtoPath(char ** res, char * Name){
    char path[40];
    for (int i = 0; i < PathNum; i ++){
        strcpy(path, PATH[i]);
        strcat(path, "/");
        strcat(path, Name);
        if (access(path, X_OK) == 0){
            *res = malloc(strlen(path));
            strcpy(*res, path);
            return 0;
        }
    }
    return -1;
}

int Path(char * tokens[], int len){
    Free(PATH, PathNum);
    for (int i = 1; i < len; i ++){
        PATH[i - 1] = malloc(strlen(tokens[i]));
        strcpy(PATH[i - 1], tokens[i]);
    }
    PathNum = len - 1;
    return 0;
}

int CountIf(char * tokens[], int len){
    int opr = 0, IF = 0, fi = 0, then = 0;
    for (int i = 0; i < len; i ++){
        if (strcmp(tokens[i], "if") == 0) IF++;
        else if (strcmp(tokens[i], "==") == 0) opr++;
        else if (strcmp(tokens[i], "!=") == 0) opr++;
        else if (strcmp(tokens[i], "fi") == 0) fi++;
        else if (strcmp(tokens[i], "then") == 0) then++;
    }
    if (IF==opr&&opr==then&&then==fi&&fi==IF) return 0;
    else return 1;
}

int If(char * tokens[], int len){
    if (len < 6 || strcmp(tokens[len - 1], "fi") != 0 ) {
        return Error();
    }
    
    //deside whether it is legal
    if (CountIf(tokens, len) == 1) return Error();

    //nested at left or right
    int isleft = 1;
    if (strcmp(tokens[1], "if") == 0) isleft = 1;
    else if (strcmp(tokens[len-2], "fi") == 0) isleft = 0;

    //get the first command
    int isequal = 10086;
    int i = 1;
    int maxi = 0;
    while (i < len){
        if (strcmp(tokens[i], "!=") == 0){
            isequal = 0;
            if (isleft == 0) break;
            else if (i > maxi) maxi = i;
        }
        else if (strcmp(tokens[i], "==") == 0){
            isequal = 1;
            if (isleft == 0) break;
            else if (i > maxi) maxi = i;
        }
        i ++;
    }

    if (isleft == 1) i = maxi;
    int sublen = i - 1;

    //do the first command
    char * subtokens[sublen];
    SubTokensCpy(subtokens, tokens, 1, sublen + 1);
    int value = DoCmd(subtokens, sublen);
    //if (value == -1) return 0
    char strvalue[5];
    sprintf(strvalue, "%d", value);

    //check tokens
    if (len - 1 - i < 3){    //at least 3 tokens: 1 then fi
        Free(subtokens, sublen);
        return Error();
    }

    //check the constant
    i++;
    for (int j = 0; j != strlen(tokens[i]); j++){
        if (isdigit(tokens[i][j]) == 0) {
            Free(subtokens, sublen);
            return Error();
        }
    }

    //check the then
    i++;
    if (strcmp(tokens[i], "then") != 0){
        Free(subtokens, sublen);
        return Error();;
    }

    //do the second command
    i++;
    if (i == len - 1 && strcmp(tokens[i], "fi") == 0)
        return 0;
    if ((isequal == 1 && strcmp(strvalue, tokens[sublen + 2]) == 0)\
     || (isequal == 0 && strcmp(strvalue, tokens[sublen + 2]) != 0)){

        int sublen2 = len - i - 1;
        char * subtokens2[sublen2];
        SubTokensCpy(subtokens2, tokens, i, len - 1);
        int ret = DoCmd(subtokens2, sublen2);
        Free(subtokens, sublen);
        Free(subtokens2, sublen2);
        return ret;
    }
    else {
        Free(subtokens, sublen);
        return 0;
    }
}

int DoCmd(char * tokens[], int len){
    if (len == 0) return 0;

    if (strcmp(tokens[0], "exit") == 0){
        if (len > 1) return Error();
        else Exit();
    }
    else if (strcmp(tokens[0], "cd") == 0){
        if (len == 1 || len > 2) return Error();
        else return Cd(tokens);
    }
    else if (strcmp(tokens[0], "path") == 0){
        return Path(tokens, len);
    }
    else if (strcmp(tokens[0], "if") == 0){
        return If(tokens, len);
    }
    else{
        int ret;
        int rc = fork();
        if (rc < 0){
            Error();
            exit(1);
        }
        else if (rc == 0){
            char * path;
            if (AddtoPath(&path, tokens[0]) == -1){
                Error();
                Exit();
            }

            int i = 0;
            while(i < len){
                if (strcmp(tokens[i], ">") == 0)
                    break;
                i++;
            }
            char * cmd[i + 1];
            SubTokensCpy(cmd, tokens, 0, i);
            cmd[i] = NULL;
            if (i != len){
                if (i != len - 2 || strcmp(tokens[i + 1], ">") == 0) {
                    Free(cmd, i);
                    Error();
                    Exit();
                }
                int fd = open(tokens[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, fileno(stdout));
                close(fd);
            }

            execv(path, cmd);
            Free(cmd, i);
            free(path);
            printf("%s\n",strerror(errno));
            Exit();
        }
        else {
            int rc_wait = wait(&ret);
            if(WIFEXITED(ret) != 0){
                return WEXITSTATUS(ret);
            }
            else return -1;
        }
    }
}

void Parsing(){
    char * command;
    //for each line:
        //if token == "", skip and don't store
        //if token == "&" or "NULL", output as a full command line
        //else, store as tokens
        //(token == NULL means one command line ends)
    for (int i = 0; i < LineNum; i++){
        char space[] = " \t\n";
        command = strdup(CommandLines[i]);
        char * token;

        do {
            token = strsep(&command, space);
            if (token != NULL && strlen(token) == 0){}
            else if (token == NULL){            //token == "&" || 
                if (TokenNum != 0){
                    DoCmd(Tokens, TokenNum);
                    Free(Tokens, TokenNum);
                    TokenNum = 0;
                }
            }
            else if (strcmp(token, ">") != 0 && Have(token, '>') != -1){
                int i = Have(token, '>');
                if (i == -2) {
                    Free(Tokens, TokenNum);
                    Error();
                    break;
                }

                char * dup_token =  strdup(token);
                char delim[] = ">";
                char * t_token = strsep(&dup_token, delim);
                if (i == 0){
                    t_token = strsep(&dup_token, delim);
                    AddtoTokens(">");
                    AddtoTokens(t_token);
                }
                else if (i == strlen(token) - 1){
                    AddtoTokens(t_token);
                    AddtoTokens(">");
                }
                else {
                    AddtoTokens(t_token);
                    AddtoTokens(">");
                    t_token = strsep(&dup_token, delim);
                    AddtoTokens(t_token);
                } 
            }
            else {
                AddtoTokens(token);
            }
        } while(token != NULL);
    }
}




int main(int argc,char* argv[]){
    PATH[0] = malloc(strlen("/bin"));
    strcpy(PATH[0], "/bin");
    PathNum = 1;
    do {
        if (argc == 1){         //interactive mode
            PrintPrompt();
            GetCommands();
        }
        else if (argc == 2){    //batch mde
            GetBatchCommands(argv[1]);
        }
        else {
            Error();
            exit(1);           //too many files
        }
        Parsing();              //get tokens of each command and run

    } while(argc == 1);
    Free(CommandLines, LineNum);
    Free(PATH, LineNum);
}