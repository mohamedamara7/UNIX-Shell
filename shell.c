#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<fcntl.h>
#define MAXLIST 100 //max number of commands
#define MAXCOM 100 //max number of letters in one command
#define clear() printf("\033[H\033[J")

void initShell()
{
    clear();
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****MY SHELL****");
    printf("\n\n\t-USE AT YOUR OWN RISK-");
    printf("\n\n\n\n*******************"
        "***********************");
    char * user=getenv("USER"); //stdlib
    printf("\n\n\nUSER is: @%s", user);
    printf("\n");
    sleep(2);//unistd
    clear();
}
void printDir()
{
    char cwd[1024];
    getcwd(cwd,sizeof(cwd));//unistd
    printf("\nDir: %s",cwd);
}
int takeInput(char * str)
{
    char * buf;
    buf=readline("\n>>> ");
    if(strlen(buf))
    {
        add_history(buf);
        strcpy(str,buf);
        return 0;
    }
    return 1;
}
int parsePipe(char *str,char**strpiped)
{
    int i;
    for(int i=0;i<2;i++)
    {
        strpiped[i]=strsep(&str,"|");
        if(strpiped[i]==NULL)
            break;
    }
    if(strpiped[1]==NULL)
        return 0;
    return 1;
}
void parseSpace(char* str,char**parsed)
{
    int i=0;
    for(int i=0;i<MAXCOM;i++)
    {
        parsed[i]=strsep(&str," ");
        if(parsed[i]==NULL)
            break;
        if(strlen(parsed[i])==0)
            --i;
    }
}
void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\nCopyright @ Suprotik Dey"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"
        "\n>improper space handling");
  
    return;
}
int ownCmdHandler(char **parsed)
{
    int NoOfOwnCmds = 4, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* user;
    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";
    for(i=0;i<NoOfOwnCmds;i++)
    {
        if(strcmp(parsed[0],ListOfOwnCmds[i])==0)
        {
            switchOwnArg=i+1;
            break;
        }
    }
    switch(switchOwnArg)
    {
        case 1:
            printf("\nGoodbye\n");
            exit(0);
        case 2:
            chdir(parsed[1]);
            return 1;
        case 3:
            openHelp();
            return 1;
        case 4:
            user=getenv("USER");
            printf("\nHello %s.\nMind that this is "
                "not a place to play around."
                "\nUse help to know more..\n",
                user);
            return 1;
        default:
        break;
    }
    return 0;
}
int processInput(char* str,char **parsed,char **parsedPipe)
{
    char *strpiped[2];
    int pipe=0;
    pipe=parsePipe(str,strpiped);
    if(pipe)
    {
        parseSpace(strpiped[0],parsed);
        parseSpace(strpiped[1],parsedPipe);
    }
    else
        parseSpace(str,parsed);
    if(ownCmdHandler(parsed))
        return 0;
    else
        return 1+pipe;
}
int inputRedirection=0,outputRedirection=0;
char *inputFile=NULL,*outputFile=NULL;
void checkRedirection(char **parsed,char **commandWithoutRedirection)
{
    int j=0;
    int i;
    for(i=0;i<MAXLIST;i++)
    {
        if(parsed[i]==NULL)
            break;
        if(strcmp(parsed[i],"<")==0)
        {
            inputRedirection=1;
            inputFile=parsed[++i];
        }
        else if(strcmp(parsed[i],">")==0)
        {
            outputRedirection=1;
            outputFile=parsed[++i];
        }
        else
            commandWithoutRedirection[j++]=parsed[i];
    }
    commandWithoutRedirection[j]=NULL;
}
void execArgs(char **parsed)
{
    pid_t pid=fork();
    if(pid<0)
    {
        printf("\nFailed forking child..");
        return;
    }
    else if(pid==0)
    {
        char *commandWithoutRedirection[MAXLIST];
        checkRedirection(parsed,commandWithoutRedirection);
        if(inputRedirection)
        {
            dup2(open(inputFile,O_RDONLY|O_CREAT,0777),STDIN_FILENO);
            inputRedirection=0;
            inputFile=NULL;
        }
        if(outputRedirection)
        {
            dup2(open(outputFile,O_WRONLY|O_CREAT,0777),STDOUT_FILENO);
            outputRedirection=0;
            outputFile=NULL;
        }
        if(execvp(commandWithoutRedirection[0],commandWithoutRedirection) < 0)
            printf("\nCould not execute command..");
        exit(0);
    }
    else
    {
        wait(NULL);
        return;
    }
}
void execArgsPiped(char **parsed,char **parsedPiped)
{
    int pipefd[2];
    pid_t p1,p2;
    if(pipe(pipefd)<0)
    {
        printf("\nPipe could not be initialized");
        return;
    }
    p1=fork();
    if(p1<0)
    {
        printf("\nCould not fork");
        return;
    }
    else if(p1==0)
    {
        close(pipefd[0]);
        dup2(pipefd[1],STDOUT_FILENO);
        close(pipefd[1]);
        if(execvp(parsed[0],parsed)<0)
        {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    }
    else
    {
        p2=fork();
        if(p2<0)
        {
            printf("\nCould not fork");
            return;
        }
        else if(p2==0)
        {
            close(pipefd[1]);
            dup2(pipefd[0],STDIN_FILENO);
            close(pipefd[0]);
            if(execvp(parsedPiped[0],parsedPiped)<0)
            {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        }
        else
        {
            close(pipefd[0]);
            close(pipefd[1]);
            wait(NULL);
            wait(NULL);
        }
    }
} 
int main()
{
    char inputString[MAXCOM],*parsedArgs[MAXLIST],*parsedArgsPiped[MAXLIST];
    int execFlag=0;
    initShell();
    while(1)
    {
        printDir();
        if(takeInput(inputString))
            continue;
        execFlag=processInput(inputString,parsedArgs,parsedArgsPiped);
        if(execFlag==1)
        {
            execArgs(parsedArgs);   
        }
        else if(execFlag==2)
        {
            execArgsPiped(parsedArgs,parsedArgsPiped);  
        }
    }
    return 0;
}