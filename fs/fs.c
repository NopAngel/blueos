/*
 * BlueOS / fs / fs.c
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 *
 * Description:
 * Static table-based filesystem implementation.
 * Supports directory hierarchy using parent_dir indexing.
 */

#include <include/printk.h>
#include <include/colors.h>
#include <include/lib/string.h>
#include <include/fs/fs.h>
#include <include/fs/vfs.h>

unsigned int directory_count = 0;
unsigned int file_count = 0;
extern char data_blocks[VFS_MAX_ENTRIES][VFS_MAX_CONTENT];

static unsigned int current_directory = 0;



/* Tables stored in Kernel memory */
DirectoryEntry directory_table[MAX_DIRECTORIES];
FileEntry file_table[MAX_FILES];



/**
 * fs_init() - Initialize the root filesystem.
 * * Must be called during kernel boot.
 */
void fs_init() {
    /* Create the Root (/) directory at index 0 */
    strcpy(directory_table[0].name, "/");
    directory_table[0].parent_dir = 0; 
    directory_count = 1;
    current_directory = 0;
}

/**
 * mkdir() - Create a new directory within the current scope.
 * @dirname: Name of the new directory.
 */
int mkdir(const char *dirname) {
    if (directory_count >= MAX_DIRECTORIES) return -1;

    /* Check if directory already exists in the current level */
    for (unsigned int i = 0; i < directory_count; i++) {
        if (directory_table[i].parent_dir == current_directory && 
            strcmp(directory_table[i].name, dirname) == 0) {
            printk(RED, "\nERR: Directory exists.\n");
            return -1;
        }
    }

    /* Set the metadata and the PARENT relationship */
    strcpy(directory_table[directory_count].name, dirname);
    directory_table[directory_count].parent_dir = current_directory; 
    directory_count++;

    printk(GREEN, "\nDir '%s' created.\n", dirname);
    return 0;
}

/**
 * touch() - Create or update a file in the current directory.
 */
int touch(const char *filename, const char *content) {
    /* Search in current directory only */
    for (unsigned int i = 0; i < file_count; i++) {
        if (file_table[i].parent_dir == current_directory &&
            strcmp(file_table[i].name, filename) == 0) {
            strcpy(file_table[i].content, content);
            file_table[i].size = strlen(content);
            return 0;
        }
    }

    if (file_count < MAX_FILES) {
        strcpy(file_table[file_count].name, filename);
        strcpy(file_table[file_count].content, content);
        file_table[file_count].size = strlen(content);
        file_table[file_count].parent_dir = current_directory; 
        file_count++;
        return 0;
    }
    return -1;
}

/**
 * list_items() - List content of the current directory (ls).
 */
void list_items() {
    printk(WHITE, "\n");
    printk(CYAN, "  .  \n  .. "); 
    unsigned int count = 0;
    for (unsigned int i = 0; i < directory_count; i++) {
        if (directory_table[i].parent_dir == current_directory) {
            if (strlen(directory_table[i].name) > 0) {
                printk(CYAN, "\n  %s/", directory_table[i].name);
            }
        }
    }



    for (unsigned int i = 0; i < file_count; i++) {
        if (file_table[i].parent_dir == current_directory) {
            printk(WHITE, "\n  %s", file_table[i].name);
    
            int len = strlen(file_table[i].name);
            int spaces = 20 - len; 
            if (spaces < 1) spaces = 1; 
            
            for (int s = 0; s < spaces; s++) {
                printk(WHITE, " ");
            }

            if (file_table[i].size < 1024) {
                printk(YELLOW, "%d B", file_table[i].size);
            } else {
                printk(YELLOW, "%d KB", file_table[i].size / 1024);
            }
        }
    }
    printk(WHITE, "\n");
}

/**
 * cd() - Change current directory index.
 */
int cd(const char *dirname) {
    if (strcmp(dirname, "/") == 0) {
        current_directory = 0;
        return 0;
    }

    if (strcmp(dirname, "..") == 0) {
        current_directory = directory_table[current_directory].parent_dir;
        return 0;
    }

    for (unsigned int i = 0; i < directory_count; i++) {
        if (directory_table[i].parent_dir == current_directory &&
            strcmp(directory_table[i].name, dirname) == 0) {
            current_directory = i;
            return 0;
        }
    }

    printk(RED, "\nERR: Not found: %s\n", dirname);
    return -1;
}

/**
 * pwd() - Print Working Directory using recursive path resolution.
 */
void pwd() {
    if (current_directory == 0) {
        printk(BLUE, "\n/\n");
        return;
    }
    /* Simple pwd for table-based FS */
    printk(BLUE, "\n/%s\n", directory_table[current_directory].name);
}

void cat (const char *filename) {
    for (unsigned int i = 0; i < file_count; i++) {
        if (file_table[i].parent_dir == current_directory &&
            strcmp(file_table[i].name, filename) == 0) {
            printk(WHITE, "\n%s\n", file_table[i].content);
            return;
        }
    }
    printk(RED, "\nERR: Not found: %s\n", filename);
}

void fs_rm(const char *name) {
    int found = 0;

    for (unsigned int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].name, name) == 0 && 
            file_table[i].parent_dir == current_directory) {
            
            for (unsigned int j = i; j < file_count - 1; j++) {
                file_table[j] = file_table[j + 1];
            }

            file_count--; 
            found = 1;
            printk(GREEN, "\nFile '%s' deleted successfully.\n", name);
            break;
        }
    }

    if (!found) {
        printk(RED, "\nfs_rm: file '%s' not found.\n", name);
    }
}

void fs_rmdir(const char *name) {
    int found = 0;

    for (unsigned int i = 0; i < directory_count; i++) {
        if (i == 0 && strcmp(name, "/") == 0) {
            printk(RED, "\nfs_rmdir: cannot remove root directory.\n");
            return;
        }

        if (strcmp(directory_table[i].name, name) == 0 && 
            directory_table[i].parent_dir == current_directory) {
            
         
            for (unsigned int f = 0; f < file_count; f++) {
                if (file_table[f].parent_dir == i) {
                    printk(RED, "\nfs_rmdir: directory not empty.\n");
                    return;
                }
            }

            for (unsigned int j = i; j < directory_count - 1; j++) {
                directory_table[j] = directory_table[j + 1];
            }

            directory_count--;
            found = 1;
            printk(GREEN, "\nDirectory '%s' removed.\n", name);
            break;
        }
    }

    if (!found) {
        printk(RED, "\nfs_rmdir: directory '%s' not found.\n", name);
    }
}

