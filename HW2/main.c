#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

char*  FileToBuf (const char* path);
char*** ParseString (char* buf, size_t* prog_num);
char**  ParseArgs   (char* str);
size_t CountProg (const char* buf);

int  IsTab (char symb);
void SkipTabs (char** text);
char* FindTab (char* text);

void CheckParse (char*** buf, size_t prog_num);
void ExecProgs (char*** exec_info, size_t prog_num);
void PtrsDestructor (char* buf, char*** exec_info, size_t prog_num);

// my realization "|" in console
int main (int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf (stderr, "Error: too few parameters.\n");
        return 0;
    }
    size_t prog_num = 0;

    char* buf = FileToBuf (argv[1]);
    char*** exec_info = ParseString (buf, &prog_num);
    // CheckParse (exec_info, prog_num);
    ExecProgs (exec_info, prog_num);

    PtrsDestructor (buf, exec_info, prog_num);
    return 0;
}

char* FileToBuf (const char* path)
{
    assert (path);

    struct stat st = {};
    if (stat (path, &st))
    {
        perror (path);
        return NULL;
    }
    
    int fd = -1;
    if ((fd = open (path, O_RDONLY)) == -1)
    {
        perror (path);
        return NULL;
    }

    char* buf = (char*) calloc (st.st_size + 1, sizeof (*buf));
    read (fd, buf, st.st_size);
    close (fd);
    return buf;
}

char*** ParseString (char* buf, size_t* prog_num)
{
    assert (buf);

    *prog_num = CountProg (buf);
    char*** progs = (char***) calloc (*prog_num, sizeof (*progs));
    char* i_prog = buf;

    for (size_t i_prog_num = 0; i_prog; i_prog_num++)
    {
        char* prog_end = strchr (i_prog, '|');
        if (prog_end)
            *prog_end = '\0';

        progs[i_prog_num] = ParseArgs (i_prog);
        i_prog = (prog_end) ? prog_end + 1 : NULL;
    }

    return progs;
}

size_t CountProg (const char* buf)
{
    assert (buf);
    size_t num = 0;

    while (buf)
    {
        num++;
        buf = strchr (buf, '|'); 
        if (buf)
            buf++;
    }

    return num;
}

char** ParseArgs (char* parse_str)
{
    assert (parse_str);
    char** arg_list = NULL;

    // mode 0 - count args
    // mode 1 - write args
    for (int i_mode = 0; i_mode < 2; i_mode++)
    {
        size_t args_num = 0;
        char* str = parse_str;
        
        while (*str)
        {
            SkipTabs (&str);
            if (!*str)
                break;

            if (*str == '\"')
            {
                if (i_mode)
                    arg_list[args_num] = str + 1;

                str = strchr (str, '\"');
                if (!str)
                    return NULL;
                
                if (i_mode)
                    *str = '\0';
                str++;
            }

            else
            {
                if (i_mode)
                    arg_list[args_num] = str;

                str = FindTab (str);
                if (*str)
                {
                    if (i_mode)
                        *str = '\0';
                    str++;
                }
            }
            args_num++;
        }

        if (!i_mode)
            arg_list = (char**) calloc (args_num + 1, sizeof (*arg_list));
        else
            arg_list[args_num] = NULL;
    }

    return arg_list;
}

void SkipTabs (char** text)
{
    assert (text);
    assert (*text);
    
    while (IsTab (**text))
        *text += 1;
    return;
}

char* FindTab (char* text)
{
    assert (text);
    while (*text && !IsTab(*text))
        text++;
    return text;
}

int IsTab (char symb)
{
    return symb == '\n' || symb == ' ' || symb == '\r' || symb == '\t';
}

void CheckParse (char*** buf, size_t prog_num)
{
    for (size_t i_prog = 0; i_prog < prog_num; i_prog++)
    {
        char** args = buf[i_prog];
        while (*args)
        {
            printf ("%s\n", *args);
            args++;
        }
        printf ("\n");
    }
}

void ExecProgs (char*** exec_info, size_t prog_num)
{
    assert (exec_info);

    typedef struct {int pair[2];} pipe_struct;
    const int IN  = 0;
    const int OUT = 1;

    pipe_struct* pipes = (pipe_struct*) calloc (prog_num, sizeof (*pipes));
    if (!pipes)
        return;

    for (size_t i_pipe = 0; i_pipe < prog_num; i_pipe++)
        pipe2 (pipes[i_pipe].pair, O_CLOEXEC);

    for (size_t i_pipe = 0; i_pipe < prog_num; i_pipe++)
    {
        if (!fork())
        {
            if (i_pipe != 0)
                dup2 (pipes[i_pipe - 1].pair[IN], STDIN_FILENO);
            if (i_pipe != prog_num - 1)
                dup2 (pipes[i_pipe].pair[OUT], STDOUT_FILENO);
            execvp (exec_info[i_pipe][0], exec_info[i_pipe]);

            /* if (i_pipe == 0 && strcmp (exec_info[0][0], "history") == 0 && exec_info[0][1] == NULL) // ToDo: why doesn't work?
            {
                pipe_struct history = {};
                pipe2 (history.pair, O_CLOEXEC);
                write (history.pair[OUT], "history\n" "exit\n", 13);
                close (history.pair[OUT]);
                dup2  (history.pair[IN], STDIN_FILENO);
                const char* call[2] = {"bash", NULL};
                execvp (call[0], (char* const*) call);
            } */

            // fprintf (stderr, "%zu |%s| %x\n", i_pipe, exec_info[0][0], exec_info[0][1]);
            perror ("Calling error");
            break;
        }
    }

    for (size_t i_pipe = 0; i_pipe < prog_num; i_pipe++)
    {
        close (pipes[i_pipe].pair[IN]);
        close (pipes[i_pipe].pair[OUT]);
    }

    free (pipes);
    
    int wait_all = 0;
    while (wait (&wait_all) != -1) {;}
    return;
}

void PtrsDestructor (char* buf, char*** exec_info, size_t prog_num)
{
    free (buf);
    for (size_t i_prog = 0; i_prog < prog_num; i_prog++)
        free (exec_info[i_prog]);
    free (exec_info);
    return;
}

/*     if (argc < 3)
    {
        fprintf (stderr, "Error: too few parameters.\n");
        return 0;
    }

    int new_stdout = -1;
    if ((new_stdout = open (argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
    {
        perror ("Can't open stdout");
        return 0;
    }

    if (!dup2 (new_stdout, STDOUT_FILENO))
    {
        perror ("");
        return 0;
    }

    close (new_stdout);
    execvp (argv[2], argv + 2); */
