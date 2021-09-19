#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#define BUF_SIZE 1024
const int NO_NUMS = 228;

typedef enum
{
    WORD = 0,
    TABS = 1
} WriteType;

int BizzBuzz       (int fd_in, int fd_out);
int WorkWithWord   (int fd_in, int fd_out, char* mini_buf);
int WriteWhileMode (int fd_in, int fd_out, off_t num_symb, char* mini_buf, WriteType mode);
int WriteNum       (int fd_in, int fd_out, char* mini_buf, off_t word_pos, int last_num, int sum_in_3);
WriteType IsTab    (const char symb);

#define SAVE_MODE(func, exit_act) if ((func) == -1)                                           \
                                  {                                                           \
                                      fprintf (stderr, "Error in " #func ", that called in"   \
                                      " %s on line %d:\n", __func__, __LINE__);               \
                                      perror ("");                                            \
                                      exit_act;                                               \
                                      return -1;                                              \
                                  }

#define NO_ACT {;}
#define CLOSE_FILES {                   \
                        close (fd_in);  \
                        close (fd_out); \
                    }

#define READ SAVE_MODE (check_end = read (fd_in, mini_buf, 1), CLOSE_FILES)

// bizzbuzz from file
int main (int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf (stderr, "Error: wrong number of input arguments.\n");
        return -1;
    }

    if (unlink (argv[2]) && errno != ENOENT) // delete file, doesn't exist - not error
    {
        perror ("Unlink error: ");
        return -1;
    }

    int fd_in = -1, fd_out = -1;
    SAVE_MODE (fd_in  = open (argv[1], O_RDONLY), NO_ACT);
    SAVE_MODE (fd_out = open (argv[2], O_WRONLY | O_CREAT, 0666), CLOSE_FILES);

    BizzBuzz (fd_in, fd_out);
    return 0;
}

int BizzBuzz (int fd_in, int fd_out)
{
    assert (fd_in  >= 0);
    assert (fd_out >= 0);

    /* char buf[BUF_SIZE] = "";
    int    num_symb = -1;
    size_t pos_symb = 0;
    SAVE_MODE (num_symb = read (fd_in, buf, BUF_SIZE), CLOSE_FILES); */

    char mini_buf  = '\0'; // FIXME!!!
    int check_end   = -1;

    SAVE_MODE (check_end = read (fd_in, &mini_buf, 1), CLOSE_FILES);

    while (check_end)
    {
        // one symbol was read already
        SAVE_MODE (check_end = WriteWhileMode (fd_in, fd_out, 1, &mini_buf, TABS), NO_ACT);
        if (!check_end)
            break;

        SAVE_MODE (check_end = WorkWithWord (fd_in, fd_out, &mini_buf), NO_ACT);
    }

    close (fd_in);
    close (fd_out);
    return 0;
}

int WorkWithWord (int fd_in, int fd_out, char* mini_buf)
{
    assert (mini_buf);
    assert (fd_in  >= 0);
    assert (fd_out >= 0);

    int check_end = -1; // ToDo: Is it correct?
    off_t word_pos = 1; // one symbol was read already

    int last_num = NO_NUMS;
    int sum_in_3 = 0;

    if (*mini_buf == '-' || *mini_buf == '+')
    {
        READ;
        word_pos++;
    }

    while (check_end)
    {
        if (IsTab (*mini_buf))
            break;

        if (!isdigit (*mini_buf)) // ToDo: add work with point
            return WriteWhileMode (fd_in, fd_out, word_pos, mini_buf, WORD);

        last_num = *mini_buf - '0';
        sum_in_3 = (sum_in_3 + last_num) % 3;

        READ;
        word_pos++;
    }

    SAVE_MODE (WriteNum (fd_in, fd_out, mini_buf, word_pos, last_num, sum_in_3), NO_ACT);
    return check_end;
}

int WriteNum (int fd_in, int fd_out, char* mini_buf, off_t word_pos, int last_num, int sum_in_3)
{
    assert (mini_buf);
    assert (fd_in  >= 0);
    assert (fd_out >= 0);

    if (last_num == NO_NUMS || (sum_in_3 != 0 && last_num % 5 != 0))
        return WriteWhileMode (fd_in, fd_out, word_pos, mini_buf, WORD);

    // ToDo: Is it effective?
    char bizzbuzz_buf[16] = ""; // Only "bizz ", "buzz " or "bizzbuzz "

    if (sum_in_3 == 0)
        strcat (bizzbuzz_buf, "bizz");

    if (last_num == 0 || last_num == 5)
        strcat (bizzbuzz_buf, "buzz");

    SAVE_MODE (write (fd_out, bizzbuzz_buf, strlen (bizzbuzz_buf)), CLOSE_FILES);
    return 0;
}

WriteType IsTab (const char symb)
{
    return (symb == ' ' || symb == '\t' || symb == '\n' || symb == '\r') ? TABS : WORD;
}

int WriteWhileMode (int fd_in, int fd_out, off_t num_symb, char* mini_buf, WriteType mode)
{
    assert (fd_in  >= 0);
    assert (fd_out >= 0);
    assert (num_symb >= 0);

    int check_end = 1;

    SAVE_MODE (lseek (fd_in, -num_symb, SEEK_CUR), CLOSE_FILES);
    READ;

    while ((IsTab (*mini_buf) == mode) && check_end)
    {
        SAVE_MODE (write (fd_out, mini_buf, 1), CLOSE_FILES);
        READ;
    }

    return check_end;
}

/* 
// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

// Sem1

void PrintArg (char* arg)
{
    assert (arg);

    char* save_arg = arg;
    int last_num = NO_NUMS;
    int sum_in_3 = 0;

    SkipTabs (&arg);
    if (*arg == '-')
        arg++;

    while (isdigit (*arg))
    {
        last_num = *arg - '0';
        sum_in_3 = (sum_in_3 + last_num) % 3;
        arg++;
    }

    SkipTabs (&arg);
    if (*arg || last_num == NO_NUMS || (sum_in_3 != 0 && last_num % 5 != 0))
    {
        printf ("%s ", save_arg);
        return;
    }

    if (sum_in_3 == 0)
        printf ("bizz");

    if (last_num == 0 || last_num == 5)
        printf ("buzz");

    printf (" ");
    return; 
}

// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

// Sem2

int MyCp (const char* file_in, const char* file_out)
{
    assert (file_in);
    assert (file_out);

    int fd_in  = open (file_in, O_RDONLY);
    int fd_out = open (file_out, O_WRONLY | O_CREAT, 0666);
    char buf[BUF_SIZE] = "";

    #define return_err {                    \
                           close (fd_in);   \
                           close (fd_out);  \
                           return 1;        \
                       }

    if (fd_in == -1 || fd_out == -1)
        return_err;

    int read_num = -1;
    while ((read_num = read (fd_in, buf, BUF_SIZE)) > 0)
    {
        if (write (fd_out, buf, read_num) != read_num)
            return_err;
    }

    if (read_num == -1)
        return_err;

    close (fd_in); 
    close (fd_out);
    return 0;   
} 

// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

// Old functions

int SkipTabs (int fd_in, int fd_out, char* mini_buf)
{
    assert (mini_buf);
    assert (fd_in  >= 0);
    assert (fd_out >= 0);

    int return_value = -1;
    off_t num_tabs  = 0;

    while (IsTab (*mini_buf))
    {
        SAVE_MODE (return_value = read (fd_in, mini_buf, 1), CLOSE_FILES);
        if (return_value == 0)
            return 0;
        num_tabs++;
    }

    SAVE_MODE (WriteFile (fd_in, fd_out, num_tabs), NO_ACT);
    return num_tabs;
}

int WriteFdToFd (int fd_in, int fd_out, off_t num_symb)
{
    assert (fd_in  >= 0);
    assert (fd_out >= 0);
    assert (num_symb >= 0);

    SAVE_MODE (lseek (fd_in, -num_symb, SEEK_CUR), CLOSE_FILES);
    
    for (off_t i_symb = 0; i_symb < num_symb; i_symb++)
    {
        char mini_buf = '\0';
        SAVE_MODE (read  (fd_in,  &mini_buf, 1), CLOSE_FILES);
        SAVE_MODE (write (fd_out, &mini_buf, 1), CLOSE_FILES);
    }

    return 0;
}

*/
