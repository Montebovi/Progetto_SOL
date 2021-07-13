#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include "fileutilities.h"

#define DIR_SEPARATOR '/'

//--------------------------------------------------------------------------------
static inline char* getFileNameFromPath(char* path)
{
    for(size_t i = strlen(path) - 1; i; i--)
    {
        if (path[i] == '/')
            return &path[i+1];
    }
    return path;
}

//--------------------------------------------------------------------------------
static inline char * combinePathAndFile(const char *dirname, const char *filename) {
    char * combined = malloc(strlen(dirname) + strlen(filename) + 2);

    if (dirname && *dirname) {
        int len = strlen(dirname);
        strcpy(combined, dirname);

        if (combined[len - 1] == DIR_SEPARATOR) {
            if (filename && *filename) {
                strcpy(combined + len, (*filename == DIR_SEPARATOR) ? (filename + 1) : filename);
            }
        }
        else {
            if (filename && *filename) {
                if (*filename == DIR_SEPARATOR)
                    strcpy(combined + len, filename);
                else {
                    combined[len] = DIR_SEPARATOR;
                    strcpy(combined + len + 1, filename);
                }
            }
        }
    }
    else if (filename && *filename)
        strcpy(combined, filename);
    else
        combined[0] = '\0';
    return combined;
}

//--------------------------------------------------------------------------------
int loadFile(char *pathname, void** buffer, size_t*dataLen){
    struct stat st;
    stat(pathname, &st);
    int size = st.st_size;

    FILE *fp = fopen(pathname, "r");
    if (fp == NULL)
        return -1;
    *buffer = malloc(size);
    *dataLen = fread(*buffer, 1, size, fp);
    if ( ferror( fp ) != 0 )
        return -1;
    fclose(fp);
    return 0;
}

//--------------------------------------------------------------------------------
int saveFile(char * dirname, char *pathname, void *data, int dataLen){
    char *filename = getFileNameFromPath(pathname);
    char* fullFilename = combinePathAndFile(dirname,filename);

    FILE *f_dst;
    f_dst  = fopen (fullFilename, "wb");
    if (f_dst == NULL){
        free(fullFilename);
        fullFilename = NULL;
        return -1;
    }

    if (fwrite(data, 1, dataLen,f_dst) != dataLen)
    {
        free(fullFilename);
        return -1;
    }
    fclose(f_dst);
    f_dst = NULL;

    free(fullFilename);
    return 0;
}

//--------------------------------------------------------------------------------
bool directoryExists(char * name){
    DIR* dir = opendir(name);
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------
int getNumOfFilesOfDirectoryRec(char * name){
        DIR *dir;
        struct dirent *entry;

        if (!(dir = opendir(name)))
            return -1;

        int totale = 0;

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                char path[1024];
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;
                snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
                int num = getNumOfFilesOfDirectoryRec(path);
                if (num == -1)
                    return -1;
                totale+=num;
            } else
                totale++;
        }
        closedir(dir);
    return totale;
}

//--------------------------------------------------------------------------------
void getFilesOfDirectoryRec(const char *name, char** fNames, int *idx, int maxFiles)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (*idx >= maxFiles)
            break;
        if (entry->d_type != DT_DIR) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            fNames[*idx] = strdup(path);
            (*idx)++;
        }
    }
    closedir(dir);

    if (*idx >= maxFiles)
        return;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR)
        continue;

        char path[1024];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        getFilesOfDirectoryRec(path,fNames, idx,maxFiles);
        if (*idx >= maxFiles)
            break;
    }
    closedir(dir);
}

