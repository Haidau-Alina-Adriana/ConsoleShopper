#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define PORT 2023
#define SIZE 300
extern int errno;
char loginMessage[] = "You have to be logged first!\nType \"exit\" to leave the app or enter your username: ";
char userNotFound[] = "Couldn't find you in our database. Try again.\nYour username: ";
char succesLogin[] = "You're succesfully logged in!\n";
char menu[] = "Options:\n[1]see categories\n[2]see cart\n[3]save cart\n[4]add item in cart\n[5]remove item from cart\n[6]place order\n[7]logout\nYour option: ";
char succesLogout[] = "You're succesfully logged out!";

int findWord(char *word)
{
    FILE *dictionary;
    int bufferLen = 255;
    char buffer[bufferLen];
    dictionary = fopen("configFile.txt", "r");
    if (dictionary == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    while (fgets(buffer, bufferLen, dictionary))
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(word, buffer) == 0)
        {
            fclose(dictionary);
            return 1;
        }
    }
    fclose(dictionary);
    return -1;
}

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in from;
    char message[SIZE] = "";
    char response[SIZE] = "";
    int sd;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    if (listen(sd, 5) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    while (1)
    {
        int client;
        int length = sizeof(from);
        fflush(stdout);

        client = accept(sd, (struct sockaddr *)&from, &length);
        if (client < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        printf("Customer connected to server!\n");
        strcpy(message, loginMessage);
        int forkPid = fork();

        if (forkPid == 0) // copil
        {
            while (1)
            {
                length = strlen(message);
                if (write(client, &length, sizeof(int)) <= 0)
                {
                    perror("Eroare la write() catre client.\n");
                }
                if (write(client, &message, sizeof(int) * length) <= 0)
                {
                    perror("Eroare la write() catre client.\n");
                }

                if (strcmp(message, succesLogin) == 0)
                {
                    printf("Customer logged succesfully\n");
                    fflush(stdout);
                    break;
                }

                bzero(response, SIZE);
                if (read(client, &length, sizeof(int)) <= 0)
                {
                    perror("Eroare la read() de la client.\n");
                }
                if (read(client, &response, sizeof(int) * length) <= 0)
                {
                    perror("Eroare la read() de la client.\n");
                }
                if (strcmp(response, "exit") == 0)
                {
                    printf("Customer logged off.\n");
                    fflush(stdout);
                    exit;
                }

                bzero(message, SIZE);
                if (findWord(response) == 1)
                {
                    strcat(message, succesLogin);
                }
                else
                {
                    strcat(message, userNotFound);
                }
            }

            ////login reusit
            strcpy(message, menu);
            while (1)
            {
                length = strlen(message);
                if (write(client, &length, sizeof(int)) <= 0)
                {
                    perror("Eroare la write() catre client.\n");
                }
                if (write(client, &message, sizeof(int) * length) <= 0)
                {
                    perror("Eroare la write() catre client.\n");
                }
                bzero(response, SIZE);
                if (read(client, &length, sizeof(int)) <= 0)
                {
                    perror("Eroare la read() de la client.\n");
                }
                if (read(client, &response, sizeof(int) * length) <= 0)
                {
                    perror("Eroare la read() de la client.\n");
                }
                // printf("Resp: %s", response);
                if (strcmp(response, "7") == 0)
                {
                    strcpy(message, succesLogout);
                    length = strlen(message);
                    if (write(client, &length, sizeof(int)) <= 0)
                    {
                        perror("Eroare la write() catre client.\n");
                    }
                    if (write(client, &message, sizeof(int) * length) <= 0)
                    {
                        perror("Eroare la write() catre client.\n");
                    }
                    printf("Customer logged off.\n");
                    fflush(stdout);
                    return 0;
                }
            }
        }
        else // parinte
        {
            close(client);
        }
    }
}
