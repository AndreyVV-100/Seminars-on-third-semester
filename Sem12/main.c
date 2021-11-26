#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

void MyPrintf (const char* format, ...);
size_t itoa (int num, char* buf);

// my printf with %d, %c, %s
int main()
{
    MyPrintf ("Londo%c is the %s no %d of Great Britain!\n", 'n', "Capital", 1);
    return 0;
}

void MyPrintf (const char* format, ...)
{
    va_list list = {};
    va_start (list, format);

    while (*format)
    {
        const char* str_now = strchr (format, '%');

        if (!str_now)
        {
            write (STDOUT_FILENO, format, strlen (format));
            break;
        }

        write (STDOUT_FILENO, format, str_now - format);

        switch (str_now[1])
        {
            case 'c':
            {
                char symb = (char) va_arg (list, int);
                write (STDOUT_FILENO, &symb, 1);
                break;
            }

            case 's':
            {
                char* str = va_arg (list, char*);
                write (STDOUT_FILENO, str, strlen (str));
                break;
            }

            case 'd':
            {
                int num = va_arg (list, int);
                char buf[32] = "";
                size_t buf_size = itoa (num, buf);
                write (STDOUT_FILENO, buf, buf_size);
                break;
            }
        }
        
        format = str_now + 2;
    }

    va_end (list);
    return;
}

size_t itoa (int num, char* buf)
{
    size_t buf_len = 0;

    do
    {
        buf[buf_len] = (num % 10) + '0';
        num /= 10;
        buf_len++;
    } 
    while (num > 0);

    buf[buf_len] = '\0';
    return buf_len;
}
