#include <stdio.h>
#include<string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

/*
 * This function is used to read a whole line from the user. I've got it from :
 * https://stackoverflow.com/questions/314401/how-to-read-a-line-from-the-console-in-c
 */
char * getLine(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;
    if(line == NULL)
        return NULL;
    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;
        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }
        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}

/*
 * This function is called whenever the child is terminated and it logs that the child process has terminated and it logs the time I got it form:
 * https://www.zentut.com/c-tutorial/c-write-text-file/ (Write)
 * https://stackoverflow.com/questions/5141960/get-the-current-time-in-c/5142028 (Time)
*/
void logFunction(){
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    FILE * fp;
    fp = fopen ("log.txt","a");
    fprintf(fp,"Child process has terminated at time %s", asctime (timeinfo));
    fclose (fp);
}

//Main function
int main() {

    ///Signal function which calls the log function whenever a child is terminated
    signal(SIGCHLD,logFunction);

    ///Main loop
    while(1){

        /// This boolean (integer) indicates if the user enters '&' to remove it from arguments and marks this boolean which controls the parent process not to wait
        int andDetected = 0;

        /// This whole part is responsible for reading form the user and parsing the input to array of strings (Splitting)
        /// The way it works is that it uses the function strtok split them then it store these tokens in array called tokens
        /// If the token is & it is not stored !!
        /// Finally it stores these tokens after filtering in another array with known size called arguments
        char *inputLine = getLine();
        char * token = strtok(inputLine, " ");
        char * tokens [1000];
        int index = 1;
        int count = 1;
        tokens[0] = token;
        while( token != NULL ) {
            token = strtok(NULL, " ");
            if (token!= NULL && (!strcmp(token,"&") || !strcmp(token,"&\n"))) {
                andDetected = 1;
            }else{
                count++;
                tokens[index++] = token;
            }
        }
        char * arguments[count];
        for (int i = 0 ; i < count ; i++){
            if (i == count -2){
                char * lastToken = tokens[i];
                if (lastToken[strlen(lastToken)-1] == '\n')
                    lastToken[strlen(lastToken)-1]='\0';
                arguments[i] = lastToken;
                break;
            }
            arguments[i] = tokens[i];
        }
        arguments[count-1] = NULL;

        ///This code exits the shell if the user enters exit
        if (!strcmp(arguments[0],"exit"))
            break;

        ///Here the main process will fork a new process and execute its command.
        ///If the andDetected is 1 then it will not wait for the child
        int id = fork();
        if (id == 0) {
            int status = execvp(arguments[0], arguments);
            if (status == -1){
                printf("YOU HAVE ENTERED A WRONG COMMAND\n");
                exit(-1);
            }
            else
                exit(0);
        }else {
            if (!andDetected)
                waitpid(id,NULL,0);
        }
    }
    return 0;
}