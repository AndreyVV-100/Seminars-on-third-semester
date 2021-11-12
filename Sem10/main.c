#include <stdio.h>
#include <dirent.h>

// print list of dirs
int main (int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf (stderr, "Bad number of arguments.\n");
        return 0;
    }

    DIR* directory = NULL;
    if ((directory = opendir (argv[1])) == NULL)
    {
        perror (argv[1]);
        return 0;
    }

    struct dirent* file_info = NULL;
    while ((file_info = readdir (directory)) != NULL)
    {
        if (file_info->d_type & DT_DIR)
            printf ("\"%s\"\n", file_info->d_name);
    }
    
    closedir (directory);
    return 0;
}
