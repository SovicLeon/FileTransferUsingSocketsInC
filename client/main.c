#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <dirent.h>
#include <glob.h>


struct setS {
    uint32_t meta;
    uint32_t lenDir;
    uint32_t size;
    uint32_t hash;
};

struct sendStruct {
    uint32_t meta;
    uint32_t lenDir;
    uint32_t size;
    uint32_t hash;
    char *directory;
    char *data;
};

 int isDirectory(const char *path) {
    DIR *dir;
    dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    }
    return 0;
}

int calculateHash(const char *data, int len) {
    uint32_t sumHash = 0;
    for (size_t i = 0; i < len; i++) {
        sumHash += data[i];
    }
    return sumHash;
}

int sendData(const char *name, int argc, const char *address, const char *port) {
    uint32_t lenDir;
    uint32_t size;

    FILE *fp; // file var https://www.guru99.com/c-file-input-output.html
    fp = fopen(name,"rb"); // file open
    if (fp == NULL) {
        printf("Error opening input file.\n");
        return 0;
    }

    fseek(fp, 0, SEEK_END); // seek to end of file
    size = ftell(fp); // get current file pointer https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
    fseek(fp, 0, SEEK_SET); // seek back to beginning of file

    lenDir = strlen(name) + 1; // + 1 for the '\0' at the end

    struct sendStruct *instance = malloc(sizeof(struct sendStruct) + lenDir + size); // set size of struct 

    instance->size = size;
    instance->lenDir = lenDir;

    int isDir = isDirectory(name);

    char *dirBuffer = malloc(lenDir);
    memcpy(dirBuffer, name, lenDir);
    dirBuffer[lenDir-1] = '\0';
    instance->directory = dirBuffer;

    //char *buffer;

    unsigned char byte;

    char *data = malloc(size);
    if (data == NULL) {
        printf("Error allocating memory.\n");
        fclose(fp);
        return 0;
    }

    // Read each byte from the input file and save it to the data array
    for (int i = 0; i < size; i++) {
        fread(&byte, sizeof(byte), 1, fp);
        data[i] = byte;
    }

    instance->data = data;

    fclose(fp);

    if (isDir) {
        //printf("%s is a directory.\n", name);
        instance->meta = 1073741824; // when directory
        instance->hash = 0;
        instance->data = 0;
        instance->size = 0;
    } else {
        //printf("%s is a file.\n", name);
        instance->meta = 2147483648; // when file
        instance->hash = calculateHash(instance->data, instance->size);
    }

    /*printf("Struct.meta: %u\n", instance->meta);
    printf("Struct.lenDir: %u\n", instance->lenDir);
    printf("Struct.size: %u\n", instance->size);
    printf("Struct.hash: %u\n", instance->hash);
    printf("Struct.directory: %s\n", instance->directory);
    printf("Struct.data: %s\n\n", instance->data);*/

    int sockfd, numbytes;  // socekt file descriptor, new file descriptor
	struct hostent *he;    // pointer to the structure hostent (returned by gethostbyname) 
	struct sockaddr_in their_addr; // server address

    socklen_t length = sizeof(their_addr);
	
	if (argc != 4) {
		write(0,"Uporaba: demo_client ime vrata file\n\0", 29);
        if (isDir) {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        } else {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
        }
		exit(1);
	}
	
	// get the server IP address address = argv[1]
	if ((he = gethostbyname(address)) == NULL) { 
		//herror("gethostbyname");  // prints string + value of h_error variable [HOST_NOT_FOUND || NO_ADDRESS or NO_DATA || NO_RECOVERY || TRY_AGAIN]
        if (isDir) {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        } else {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
        }
		exit(1);		  
	}
	
	// create socket
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		//perror("socket");
        if (isDir) {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        } else {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
        }
		exit(1);
	}
	
	their_addr.sin_family = AF_INET; // family to Address Family InterNET
	their_addr.sin_port = htons(atoi(port)); // get server's port number - should be checked for input errors (not a number, etc.) argv[2]
 	their_addr.sin_addr = *((struct in_addr *)he->h_addr); // server's IP address from hostent structure pointed to by he variable...
	memset(&(their_addr.sin_zero), '\0', 8); // for conversion between structures only, a trick to ensure pointer casting...
	
	
	// connect to the server... (no bind necessary as we are connecting to remote host... Kernel will find a free port for us and will bind it to sockfd)
	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
		//perror("connect");
        if (isDir) {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        } else {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
        }
		exit(1);
	}
    
    struct setS *sss = malloc(sizeof(struct setS));
    sss->hash = instance->hash;
    sss->lenDir = instance->lenDir;
    sss->size = instance->size;
    sss->meta = instance->meta;
    
    if(sendto(sockfd, sss, 16, 0, (struct sockaddr *)&their_addr, length) < 0) {
		//perror("sendto");
        if (isDir) {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        } else {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
        }
	}

    if(sendto(sockfd, instance->directory, instance->lenDir, 0, (struct sockaddr *)&their_addr, length) < 0) {
		//perror("sendto");
        if (isDir) {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        } else {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
        }
	}

    if(sendto(sockfd, instance->data, instance->size, 0, (struct sockaddr *)&their_addr, length) < 0) {
		//perror("sendto");
        if (isDir) {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        } else {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
        }
	}

	// close socket
	close(sockfd);

    if (isDir) {
        printf("STATUS USPEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
    } else {
        printf("STATUS USPEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
    }

    free(data);
    free(dirBuffer);
    free(instance);
    free(sss);
    return 1;
}


int main(int argc, char *argv[]) {
    char path[20];
    strcpy(path,argv[3]);
    char *token;
    char *delim = "/";  // The delimiter character
    char dir[100] = "";
    char isJPG[100] = "";
    char lastFour[5] = "";
    int first = 1;

    int count = 0;
    for (int i = 0; i < strlen(path); i++) {
        if (path[i] == '/')
        {
            count++;
        }
    }
    
    // Use strtok() to split the path into tokens
    int dirCounter = 0;
    token = strtok(path, delim);
    while (token != NULL) {
        strcat(dir, token);

        // send first file/directory
        if (first) {
            sendData(dir, argc, argv[1], argv[2]);
            
            first = 0;
        }
        if(isDirectory(dir)) {
            dirCounter++;
            if (dirCounter == count) {
                if (!isDirectory(argv[3])) {
                    break;
                }
            }
            // to depth of 5
            if (dirCounter == 6) {
                break;
            }
            
            // set names
            glob_t glob_result;
            char dir_name[100] = "";
            strcpy(dir_name,dir);
            strcat(dir_name,"/*");

            // open directory
            if (glob(dir_name, 0, NULL, &glob_result) != 0) {
                printf("Could not read directory %s\n", dir_name);
                return 1;
            }

            // send everything inside directory
            for (int i = 0; i < glob_result.gl_pathc; i++) {
                sleep(1);
                strcpy(isJPG,glob_result.gl_pathv[i]);
                if (strlen(isJPG) >= 4) {
                    strcpy(lastFour, &isJPG[strlen(isJPG) - 4]);
                    if (strcmp(lastFour,".jpg")) {
                        sendData(glob_result.gl_pathv[i], argc, argv[1], argv[2]);
                    }
                }
            }

            globfree(&glob_result);
        }
        strcat(dir,delim);
        token = strtok(NULL, delim);
    }

    // if after last directory is a file
    if (!isDirectory(argv[3]) && count > 0) {
        sendData(argv[3], argc, argv[1], argv[2]);
    }
	return 0;
}