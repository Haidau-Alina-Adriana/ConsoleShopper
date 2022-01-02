#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#define SIZE 1000
extern int errno;
int port;
char loginMessage[] = "You have to be logged first!\nType \"exit\" to leave the app or enter your username: ";
char userNotFound[] = "Couldn't find you in our database. Try again.\nYour username: ";
char succesLogin[] = "\nYou're succesfully logged in!";
char categoryNotFound[] = "Category does not exists!\n";
int main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in server;
    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }
    port = atoi(argv[2]);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    char message[SIZE] = "";
    char response[SIZE] = "";
    int length;
    while (1)
    {
        bzero(message, SIZE);
        if (read(sd, &length, sizeof(int)) < 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }
        if (read(sd, &message, sizeof(int) * length) < 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }

        printf("%s", message);
        fflush(stdout);
        if (strcmp(message, succesLogin) == 0)
        {
            break;
        }

        bzero(response, SIZE);
        read(0, response, 100);
        response[strcspn(response, "\n")] = '\0';
        length = strlen(response);
        if (write(sd, &length, sizeof(int)) <= 0)
        {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }
        if (write(sd, &response, sizeof(int) * length) <= 0)
        {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }
        if (strcmp(response, "exit") == 0)
        {
            break;
        }
    }
    //login reusit sau exit
    if (strcmp(response, "exit") == 0)
    {
        printf("See you next time!\n");
        fflush(stdout);
        close(sd);
        return 0;
    }
    while (1)
    {
        bzero(message, SIZE);
        if (read(sd, &length, sizeof(int)) < 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }

        if (read(sd, &message, sizeof(int) * length) < 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }
        printf("\n%s", message);
        fflush(stdout);
        bzero(response, SIZE);
        read(0, response, 100);
        response[strcspn(response, "\n")] = '\0';
        length = strlen(response);
        if (write(sd, &length, sizeof(int)) <= 0)
        {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }
        if (write(sd, &response, sizeof(int) * length) <= 0)
        {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }
        if (strcmp(response, "1") == 0)
        {
            while (1) //intru in categorii
            {
                bzero(message, SIZE);
                if (read(sd, &length, sizeof(int)) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }

                if (read(sd, &message, sizeof(int) * length) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                printf("%s", message);

                fflush(stdout);
                bzero(response, SIZE);
                read(0, response, 100);
                response[strcspn(response, "\n")] = '\0';
                length = strlen(response);
                if (write(sd, &length, sizeof(int)) <= 0)
                {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                if (write(sd, &response, sizeof(int) * length) <= 0)
                {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                if (strcmp(response, "6") == 0)
                {
                    break;
                }
            }
        }
        else if (strcmp(response, "3") == 0)
        {
            bzero(message, SIZE);
            if (read(sd, &length, sizeof(int)) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }

            if (read(sd, &message, sizeof(int) * length) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            printf("\n%s", message);
            fflush(stdout);
            bzero(response, SIZE);
            read(0, response, 100);
            response[strcspn(response, "\n")] = '\0';
            length = strlen(response);
            if (write(sd, &length, sizeof(int)) <= 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            if (write(sd, &response, sizeof(int) * length) <= 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
        }
        else if (strcmp(response, "4") == 0)
        {
            while (1)
            {
                bzero(message, SIZE);
                if (read(sd, &length, sizeof(int)) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                if (read(sd, &message, sizeof(int) * length) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                if (strstr(message, "Item adeed to cart!") != 0)
                {
                    break;
                }
                printf("\n%s", message);
                fflush(stdout);
                bzero(response, SIZE);
                read(0, response, 100);
                response[strcspn(response, "\n")] = '\0';
                length = strlen(response);
                if (write(sd, &length, sizeof(int)) <= 0)
                {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                if (write(sd, &response, sizeof(int) * length) <= 0)
                {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }
            }
        }
        else if (strcmp(response, "5") == 0)
        {

            bzero(message, SIZE);
            if (read(sd, &length, sizeof(int)) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (read(sd, &message, sizeof(int) * length) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            printf("\n%s", message);
            fflush(stdout);
            bzero(response, SIZE);
            if (strstr(message, "Empty cart!") == NULL)
            {

                read(0, response, 100);
                response[strcspn(response, "\n")] = '\0';
                length = strlen(response);
                if (write(sd, &length, sizeof(int)) <= 0)
                {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                if (write(sd, &response, sizeof(int) * length) <= 0)
                {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                bzero(message, SIZE);
                if (read(sd, &length, sizeof(int)) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                if (read(sd, &message, sizeof(int) * length) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                printf("\n%s", message);
                fflush(stdout);
            }
        }
        else if (strcmp(response, "7") == 0)
        {
            bzero(message, SIZE);
            if (read(sd, &length, sizeof(int)) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }

            if (read(sd, &message, sizeof(int) * length) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            printf("\n%s", message);
            fflush(stdout);
            bzero(response, SIZE);
            read(0, response, 100);
            response[strcspn(response, "\n")] = '\0';
            length = strlen(response);
            if (write(sd, &length, sizeof(int)) <= 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            if (write(sd, &response, sizeof(int) * length) <= 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
        }
        else if (strcmp(response, "8") == 0)
        {

            bzero(message, SIZE);
            if (read(sd, &length, sizeof(int)) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (read(sd, &message, sizeof(int) * length) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            printf("%s\n", message);
            fflush(stdout);
            return 0;
        }
        else
        {
            continue;
        }
    }
    close(sd);
    return 0;
}