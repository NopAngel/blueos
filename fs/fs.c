#include "../include/printk.h"
#include "../include/colors.h"
#include "../include/string/string.h"
#include "../include/fs/fs.h"

#define MAX_DIRECTORIES 300
#define MAX_NAME_LENGTH 30
#define MAX_CONTENT_LENGTH 1024
#define MAX_LINES 10000
#define MAX_FILES 300
static unsigned int current_directory = 0;

extern int cursor_y;


typedef struct {
    char name[MAX_NAME_LENGTH];
    unsigned int parent_dir;  
    unsigned int start_block;  
    unsigned int size;           
} DirectoryEntry;

DirectoryEntry directory_table[MAX_DIRECTORIES];
unsigned int directory_count = 0;



typedef struct {
    char name[MAX_NAME_LENGTH];    
    unsigned int start_block;     
    unsigned int size;             
    char content[MAX_CONTENT_LENGTH];
} FileEntry;

unsigned int file_count = 0;
FileEntry file_table[MAX_FILES]; 

void create_new_file(const char *filename, const char *content) {
    strcpy(file_table[file_count].name, filename);   
    strcpy(file_table[file_count].content, content); 
    file_table[file_count].size = strlen(content);   
    file_table[file_count].start_block = directory_count * 16 + file_count;
    file_count++;
}

int touch(const char *filename, const char *content) {
    create_new_file(filename, content);
    return 0;
}

int mkdir(const char *dirname) {

    if (directory_count >= MAX_DIRECTORIES) {
        printk("ERR: No more directories can be created", cursor_y++, RED);
        return -1;
    }


    if (strlen(dirname) >= MAX_NAME_LENGTH) {
        printk("ERR: The name is long.", cursor_y++, RED);
        return -1;
    }

    for (unsigned int i = 0; i < directory_count; i++) {
        if (strcmp(directory_table[i].name, dirname) == 0) {
            printk("ERR: The directory already exists.", cursor_y++, RED);
            return -1;
        }
    }

 
    strcpy(directory_table[directory_count].name, dirname); 
    directory_table[directory_count].start_block = directory_count * 16; 
    directory_table[directory_count].size = 1; 
    directory_count++; 

    printk("Directory's created.", cursor_y++, GREEN);
    return 0;
}

void list_items() {
    cursor_y++;

    if (directory_count > 0) {
        printk(".", cursor_y++, BLUE);
        printk("..", cursor_y++, BLUE);
        for (unsigned int i = 0; i < directory_count; i++) {
            printk(directory_table[i].name, cursor_y++, BLUE);
        }
    }


    if (file_count > 0) {
        for (unsigned int i = 0; i < file_count; i++) {
            printk(file_table[i].name, cursor_y++, GRAY);
        }
    }


    if (directory_count == 0 && file_count == 0) {
        printk("Err: No content.", cursor_y++, RED);

    }

    cursor_y++;

}

int cat(const char *filename) {
    cursor_y++;
    for (unsigned int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            char temp_buffer[MAX_CONTENT_LENGTH];
            unsigned int buffer_index = 0;
            unsigned int line_number = 1;
            cursor_y++;
            
            for (unsigned int j = 0; j < file_table[i].size; j++) {
                char current_char = file_table[i].content[j];
                
                if (current_char == '\n' || buffer_index >= MAX_CONTENT_LENGTH - 1) {
                    temp_buffer[buffer_index] = '\0';
                    
                    char line_info[50];
                    char num_str[10];
                    
                    
                    unsigned int temp_num = line_number;
                    int digit_index = 0;
                    if (temp_num == 0) {
                        num_str[digit_index++] = '0';
                    } else {
                        while (temp_num > 0) {
                            num_str[digit_index++] = '0' + (temp_num % 10);
                            temp_num /= 10;
                        }
                        for (int k = 0; k < digit_index / 2; k++) {
                            char temp = num_str[k];
                            num_str[k] = num_str[digit_index - 1 - k];
                            num_str[digit_index - 1 - k] = temp;
                        }
                    }
                    num_str[digit_index] = '\0';
                    
                    strcpy(line_info, "[");
                    strcpy(line_info + 1, num_str);
                    strcpy(line_info + 1 + digit_index, "] ");
                    
                    printk(line_info, cursor_y, YELLOW);
                    printk(temp_buffer, cursor_y, WHITE);
                    cursor_y++;
                    
                    buffer_index = 0;
                    if (current_char == '\n') {
                        line_number++;
                    } else {
                        temp_buffer[buffer_index++] = current_char;
                    }
                } else {
                    temp_buffer[buffer_index++] = current_char;
                }
            }
            

            if (buffer_index > 0) {
                temp_buffer[buffer_index] = '\0';
                char line_info[50];
                char num_str[10];
                
                unsigned int temp_num = line_number;
                int digit_index = 0;
                if (temp_num == 0) {
                    num_str[digit_index++] = '0';
                } else {
                    while (temp_num > 0) {
                        num_str[digit_index++] = '0' + (temp_num % 10);
                        temp_num /= 10;
                    }
                    for (int k = 0; k < digit_index / 2; k++) {
                        char temp = num_str[k];
                        num_str[k] = num_str[digit_index - 1 - k];
                        num_str[digit_index - 1 - k] = temp;
                    }
                }
                num_str[digit_index] = '\0';
                
                strcpy(line_info, "[");
                strcpy(line_info + 1, num_str);
                strcpy(line_info + 1 + digit_index, "] ");
                
                printk(line_info, cursor_y, YELLOW);
                printk(temp_buffer, cursor_y, WHITE);
                cursor_y++;
            }
            
            char size_info[50];
            char size_str[10];
            
            unsigned int temp_size = file_table[i].size;
            int size_digit_index = 0;
            if (temp_size == 0) {
                size_str[size_digit_index++] = '0';
            } else {
                while (temp_size > 0) {
                    size_str[size_digit_index++] = '0' + (temp_size % 10);
                    temp_size /= 10;
                }
                for (int k = 0; k < size_digit_index / 2; k++) {
                    char temp = size_str[k];
                    size_str[k] = size_str[size_digit_index - 1 - k];
                    size_str[size_digit_index - 1 - k] = temp;
                }
            }
            size_str[size_digit_index] = '\0';
            

            return 0;
        }
    }
    
    printk("ERR: File not found: ", cursor_y++, RED);
    printk(filename, cursor_y++, RED);
    return -1;
}

void pwd() {
    if (current_directory == 0) {
        cursor_y++;
        printk("/", cursor_y++, BLUE);
        return;
    }

    char path[MAX_NAME_LENGTH * 10] = "";
    unsigned int dir = current_directory;
    
    strcpy(path, directory_table[dir].name);
    
    while (directory_table[dir].parent_dir != 0) {
        dir = directory_table[dir].parent_dir;
        
        char temp_path[MAX_NAME_LENGTH * 10] = "";
        strcpy(temp_path, directory_table[dir].name);
        strcpy(temp_path + strlen(directory_table[dir].name), "/");
        strcpy(temp_path + strlen(directory_table[dir].name) + 1, path);
        strcpy(path, temp_path);
    }

    char final_path[MAX_NAME_LENGTH * 10] = "/";
    strcpy(final_path + 1, path);
    
    printk(final_path, cursor_y++, BLUE);
}

int cd(const char *dirname) {
    if (strcmp(dirname, "..") == 0) {
        if (current_directory != 0) {
            current_directory = directory_table[current_directory].parent_dir;
            return 0;
        } else {
            return 0;
        }
    }
    
    if (strcmp(dirname, ".") == 0 || strcmp(dirname, "/") == 0) {
        if (strcmp(dirname, "/") == 0) {
            current_directory = 0;
        } else {
        }
        return 0;
    }
    

    for (unsigned int i = 0; i < directory_count; i++) {
        if (strcmp(directory_table[i].name, dirname) == 0) {
            if (directory_table[i].parent_dir == current_directory) {
                current_directory = i;
                
                char msg[100] = "Change directory: ";
                strcpy(msg + 23, dirname);
                printk(msg, cursor_y++, GREEN);
                return 0;
            } else {
                printk("ERR: Directory not found", cursor_y++, RED);
                return -1;
            }
        }
    }
    
    printk("ERR: File not found ", cursor_y++, RED);
    return -1;
}