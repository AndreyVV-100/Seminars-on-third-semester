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

typedef struct
{
    char buf[BUF_SIZE];
    int fd;
    off_t pos_in_buf,
          buf_end;
} File;

#define FILE_INTIALIZATION {{}, -1, 0, 0}

int BizzBuzz       (File* file_in, File* file_out);
int WorkWithWord   (File* file_in, File* file_out, char* symb_now);
int WriteWhileMode (File* file_in, File* file_out, off_t num_symb, char* symb_now, WriteType mode);
int WriteNum       (File* file_in, File* file_out, char* symb_now, off_t word_pos, int last_num, int sum_in_3);
WriteType IsTab    (const char symb);

int BufferedRead  (File* file, char* symb);
int BufferedWrite (File* file, const char* str, size_t len);
int BufferedLseek (File* file, off_t offset);
int BufferedFlush (File* file);

#define SAVE_MODE(func, exit_act) if ((func) == -1)                                           \
                                  {                                                           \
                                      fprintf (stderr, "Error in " #func ", that called in"   \
                                      " %s on line %d:\n", __func__, __LINE__);               \
                                      perror ("");                                            \
                                      exit_act;                                               \
                                      return -1;                                              \
                                  }

#define NO_ACT {;}
#define CLOSE_FILES {                         \
                        close (file_in->fd);  \
                        close (file_out->fd); \
                        file_in->fd = -1;     \
                        file_in->fd = -1;     \
                    }

#define READ SAVE_MODE (check_end = BufferedRead (file_in, symb_now), CLOSE_FILES)

#define ASSERT_FILES assert (file_in);           \
                     assert (file_out);          \
                     assert (file_in->fd  >= 0); \
                     assert (file_out->fd >= 0)

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

    File file_in  = FILE_INTIALIZATION, 
         file_out = FILE_INTIALIZATION;

    // int fd_in = -1, fd_out = -1;
    SAVE_MODE (file_in.fd  = open (argv[1], O_RDONLY), NO_ACT);
    SAVE_MODE (file_out.fd = open (argv[2], O_WRONLY | O_CREAT, 0666), close (file_in.fd);
                                                                       close (file_out.fd););

    SAVE_MODE (BizzBuzz (&file_in, &file_out), NO_ACT);
    BufferedFlush (&file_out);

    close (file_in.fd);
    close (file_out.fd);
    return 0;
}

int BizzBuzz (File* file_in, File* file_out)
{
    ASSERT_FILES;

    char symb_now  = '\0';
    int check_end   = -1;

    SAVE_MODE (check_end = BufferedRead (file_in, &symb_now), CLOSE_FILES);

    while (check_end)
    {
        // one symbol was read already
        SAVE_MODE (check_end = WriteWhileMode (file_in, file_out, 1, &symb_now, TABS), NO_ACT);
        if (!check_end)
            break;

        SAVE_MODE (check_end = WorkWithWord (file_in, file_out, &symb_now), NO_ACT);
    }

    return 0;
}

int WorkWithWord (File* file_in, File* file_out, char* symb_now)
{
    assert (symb_now);
    ASSERT_FILES;

    int check_end = -1; // ToDo: Is it correct?
    off_t word_pos = 1; // one symbol was read already

    int last_num  = NO_NUMS, 
        sum_in_3  = 0, 
        was_point = 0;

    if (*symb_now == '-' || *symb_now == '+')
    {
        READ;
        word_pos++;
    }

    while (check_end)
    {
        #define PRINT_WORD return WriteWhileMode (file_in, file_out, word_pos, symb_now, WORD)
        if (IsTab (*symb_now))
            break;

        if (*symb_now == '.' || *symb_now == ',')
        {
            if (was_point)
                PRINT_WORD;
            
            was_point = 1;
            READ;
            word_pos++;
            continue;
        }
        else if (!isdigit (*symb_now))
            PRINT_WORD;

        if (!was_point)
        {
            last_num = *symb_now - '0';
            sum_in_3 = (sum_in_3 + last_num) % 3;
        }
        else if (*symb_now != '0')
            PRINT_WORD;

        READ;
        word_pos++;
    }

    SAVE_MODE (WriteNum (file_in, file_out, symb_now, word_pos, last_num, sum_in_3), NO_ACT);
    return check_end;
    #undef PRINT_WORD
}

int WriteNum (File* file_in, File* file_out, char* symb_now, off_t word_pos, int last_num, int sum_in_3)
{
    assert (symb_now);
    ASSERT_FILES;
    
    if (last_num == NO_NUMS || (sum_in_3 != 0 && last_num % 5 != 0))
        return WriteWhileMode (file_in, file_out, word_pos, symb_now, WORD);

    // ToDo: Is it effective?
    char bizzbuzz_buf[16] = ""; // Only "bizz ", "buzz " or "bizzbuzz "

    if (sum_in_3 == 0)
        strcat (bizzbuzz_buf, "bizz");

    if (last_num == 0 || last_num == 5)
        strcat (bizzbuzz_buf, "buzz");

    SAVE_MODE (BufferedWrite (file_out, bizzbuzz_buf, strlen (bizzbuzz_buf)), CLOSE_FILES);
    return 0;
}

WriteType IsTab (const char symb)
{
    return (symb == ' ' || symb == '\t' || symb == '\n' || symb == '\r') ? TABS : WORD;
}

int WriteWhileMode (File* file_in, File* file_out, off_t num_symb, char* symb_now, WriteType mode)
{
    ASSERT_FILES;
    assert (num_symb >= 0);

    int check_end = 1;

    SAVE_MODE (BufferedLseek (file_in, -num_symb), CLOSE_FILES);
    READ;

    while ((IsTab (*symb_now) == mode) && check_end)
    {
        SAVE_MODE (BufferedWrite (file_out, symb_now, 1), CLOSE_FILES);
        READ;
    }

    return check_end;
}

int BufferedRead (File* file, char* symb)
{
    assert (file);
    assert (file->fd >= 0);
    assert (file->buf_end >= file->pos_in_buf);
    assert (file->buf_end <= BUF_SIZE);
    assert (symb);

    if (file->buf_end == file->pos_in_buf)
    {
        SAVE_MODE (file->buf_end = read (file->fd, file->buf, BUF_SIZE), NO_ACT);
        file->pos_in_buf = 0;
    }

    if (file->buf_end == 0)
        return 0;

    *symb = file->buf[file->pos_in_buf];
    file->pos_in_buf++;
    return 1;
}

int BufferedWrite (File* file, const char* str, size_t len)
{
    assert (file);
    assert (file->fd >= 0);
    assert (file->pos_in_buf <= BUF_SIZE);
    assert (str);

    if (len >= 2ul * BUF_SIZE - file->pos_in_buf) // need >= 2 memcpy
    {
        SAVE_MODE (write (file->fd, file->buf, file->pos_in_buf), NO_ACT);
        SAVE_MODE (write (file->fd, str, len), NO_ACT);
        file->pos_in_buf = 0;
    }

    else if (len >= (size_t) BUF_SIZE - file->pos_in_buf)
    {
        int write_part = BUF_SIZE - file->pos_in_buf;
        memcpy (file->buf + file->pos_in_buf, str, write_part);
        SAVE_MODE (write (file->fd, file->buf, BUF_SIZE), NO_ACT);
        file->pos_in_buf = len - write_part;
        memcpy (file->buf, str + write_part, file->pos_in_buf);
    }

    else
    {
        memcpy (file->buf + file->pos_in_buf, str, len);
        file->pos_in_buf += len;
    }

    return len;
}

int BufferedLseek (File* file, off_t offset) // ToDo: SEEK_SET, SEEK_END
{
    assert (file);
    assert (file->fd >= 0);

    if (file->pos_in_buf + offset >= 0 && file->pos_in_buf + offset < BUF_SIZE)
    {
        file->pos_in_buf += offset;
        if (file->pos_in_buf >= file->buf_end)
            return -1;
    }

    else
    {
        int ret = 0;
        SAVE_MODE (ret = lseek (file->fd, offset - 1, SEEK_CUR), NO_ACT); // ToDo: Why -1?
        file->pos_in_buf = 0;
        file->buf_end    = 0;
    }

    return 0;
}

int BufferedFlush (File* file)
{
    assert (file);
    assert (file->fd >= 0);

    SAVE_MODE (write (file->fd, file->buf, file->pos_in_buf), NO_ACT);
    file->pos_in_buf = 0;
    return 0;
} 
