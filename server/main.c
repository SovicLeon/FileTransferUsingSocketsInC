#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>

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

int calculateHash(const char *data, int len) {
    uint32_t sumHash = 0;
    for (size_t i = 0; i < len; i++) {
        sumHash += data[i];
    }
    return sumHash;
}

void* handle_client(void* arg) {
    int newfd = *(int*)arg;
    char buf[sizeof(struct setS)] = {0};
    struct setS *ggg;
	int numbytes;

    // receive struct sendStruct from client...
    if ((numbytes = recv(newfd, buf, sizeof(buf), 0)) == -1) {
        perror("recv");
        exit(1);
    }

    ggg = (struct setS *)buf;

    /*printf("meta: %u\n", ggg->meta);
    printf("lenDir: %u\n", ggg->lenDir);
    printf("size: %u\n", ggg->size);
    printf("hash: %u\n", ggg->hash);*/

    struct sendStruct *instance = malloc(sizeof(struct sendStruct) + ggg->lenDir + ggg->size);

    instance->meta = ggg->meta;
    instance->lenDir = ggg->lenDir;
    instance->size = ggg->size;
    instance->hash = ggg->hash;

    char *directory = malloc(ggg->lenDir);

    if ((numbytes = recv(newfd, directory, ggg->lenDir, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    instance->directory = directory;

    char *data = malloc(ggg->size);

    if ((numbytes = recv(newfd, data, ggg->size, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    instance->data = data;

    //printf("directory: %s\n", instance->directory);
    //printf("data: %s\n", instance->data);

	if (instance->meta == 2147483648) {
		uint32_t checkHash = calculateHash(instance->data, instance->size);

		if (checkHash != instance->hash) {
			printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
		}

        FILE *fp;

        fp = fopen(instance->directory, "wb");
        if (fp == NULL) {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
            //printf("Error opening output file.\n");
        }

        // Write the data array to the output file
        fwrite(data, instance->size, 1, fp);

        // Close the files and free the memory
        fclose(fp);

		/* Success message */
        printf("STATUS USPEL \t TIP %s \t POT %s \t VELIKOST %u\n", "ZBIRKA", instance->directory, instance->size);
	} else if(instance->meta == 1073741824) {
        int status = mkdir(instance->directory, 0777);
    
        if (status == 0) {
            printf("STATUS USPEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        } else {
            printf("STATUS SPODLETEL \t TIP %s \t POT %s \t VELIKOST %u\n", "IMENIK", instance->directory, instance->size);
        }
    }

    free(directory);
    free(data);
    free(instance);
    
    close(newfd);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	int sockfd, newfd, numbytes; // socekt file descriptor, new file descriptor
	socklen_t length;  // socket length (length of clinet address)
	struct sockaddr_in saddr, caddr; // server address, client address
	time_t itime; // time format
    struct sendStruct recv_data; // array holding the string to be received via TCP
	char buf[sizeof(struct setS)] = {0};
	char *tstr; // var holding the string to be send via TCP

	if(argc != 2) {
		write(0, "Uporaba: TCPtimes vrata (vrata 0-1024 so rezervirana za jedro)\n\0", 25);
		exit(1);
	}
	
	// create socket
	if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
	}

	saddr.sin_family = AF_INET; // IPv4 
	saddr.sin_addr.s_addr = INADDR_ANY; // localhost
	saddr.sin_port = htons(atoi(argv[1])); // port converted from ascii to integer

    // binds the socket file description to the actual port (similar to open)
	if (bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		perror("bind");
	}

	// start listening on the given port
	if(listen(sockfd, 0) < 0) {
		perror("listen");
	}

	length = sizeof(caddr); // length of client address

	struct setS *ggg;

	while(1) {
        // accept new client (wait for him!)
        if((newfd = accept(sockfd, (struct sockaddr *)&caddr, &length)) < 0) {
            perror("accept");
        }

		pthread_t tid;
		pthread_create(&tid, NULL, handle_client, &newfd);
        pthread_detach(tid);
	}

	close(sockfd);
	
	return 0;
}