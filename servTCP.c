#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define PORT 2020
#define SIZE 1000
extern int errno;
char loginMessage[] = "You have to be logged first!\nType \"exit\" to leave the app or enter your username: ";
char userNotFound[] = "Couldn't find you in our database. Try again.\nYour username: ";
char succesLogin[] = "You're succesfully logged in!";
char menu[] = "Options:\n[1]see categories\n[2]see cart\n[3]save cart\n[4]add item in cart\n[5]remove item from cart\n[6]place order\n[7]logout\nYour option: ";
char succesLogout[] = "You're succesfully logged out!";
char categories[] = "\nAvailable categories:\n[1]appliances\n[2]sport\n[3]house\n[4]garden\n[5]it\n[6]go back\nYour choice:";
char cmdNotFound[] = "Command not available!Try again: \n";
char categoryNotFound[] = "Category does not exists!\n";
char productsFromCategory[300];
char cart[300];
char path[50];
int cartState = 1;
char resultCommand[1000];

void saveCart(int option)
{
    memset(resultCommand, 0, 1000);
    if (option == 1)
    {
        return;
    }
    else if (option == 2)
    {
        strcpy(resultCommand, "Cart saved!\n");
        cartState = 1;
    }
    else
    {
        strcpy(resultCommand, "Option not available!\n");
    }
}
void placeOrder()
{
    memset(resultCommand, 0, 1000);
    if (cartState == 1)
    {
        if (strcmp(cart, "") == 0)
        {
            strcpy(resultCommand, "Add some items to your cart!\n\n");
        }
        else
        {
            strcpy(resultCommand, "Your order has been registered!\n\n");
        }
    }
    else
    {
        strcpy(resultCommand, "Please save your cart first to place the order!\n\n");
    }
}
int checkIfCategExists(char *category)
{
    memset(resultCommand, 0, 1000);
    memset(path, 0, 50);
    strcpy(path, category);
    strcat(path, ".txt");
    struct stat findFile;
    if (stat(path, &findFile) == -1)
    {
        strcpy(resultCommand, categoryNotFound);
        return -1;
    }
    return 1;
}
int addItem(char *item)
{
    memset(resultCommand, 0, 1000);
    FILE *filename;
    int bufferLen = 255;
    char buffer[bufferLen];
    filename = fopen(path, "r");
    if (filename == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    while (fgets(buffer, bufferLen, filename))
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, item) == 0)
        {
            strcat(cart, item);
            strcat(cart, "\n");
            cartState = 0;
            strcpy(resultCommand, "Item adeed to cart!\n");
            fclose(filename);
            return 1;
        }
    }
    strcpy(resultCommand, "This item does not exists in this category!Try again!\n");
    fclose(filename);
    return -1;
}
void removeItem(char *item)
{
    memset(resultCommand, 0, 1000);
    char result[500] = "";
    const char delim[2] = "\n";
    char *token;
    token = strtok(cart, delim);
    int ok = 0;
    while (token != NULL)
    {
        if (strcmp(token, item) == 0 && ok == 0)
        {
            token = strtok(NULL, delim);
            ok = 1;
            continue;
        }
        strcat(result, token);
        strcat(result, "\n");
        token = strtok(NULL, delim);
    }
    if (ok == 0)
    {
        memset(cart, 0, 300);
        strcpy(cart, result);
        strcpy(resultCommand, "Couldn't find the item in cart!\n");
    }
    else
    {
        memset(cart, 0, 300);
        strcpy(cart, result);
        strcpy(resultCommand, "Item removed from cart!\n");
    }
}
int findUser(char *word)
{
    FILE *configFile;
    int bufferLen = 255;
    char buffer[bufferLen];
    configFile = fopen("configFile.txt", "r");
    if (configFile == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    while (fgets(buffer, bufferLen, configFile))
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(word, buffer) == 0)
        {
            fclose(configFile);
            return 1;
        }
    }
    fclose(configFile);
    return -1;
}
int getProductsFromCategory(char *category)
{
    FILE *filename;
    int bufferLen = 255;
    char buffer[bufferLen];
    char path[100] = "";
    memset(productsFromCategory, 0, 300);
    strcpy(path, category);
    strcat(path, ".txt");
    filename = fopen(path, "r");
    if (filename == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    strcpy(productsFromCategory, "Available products:\n");
    while (fgets(buffer, bufferLen, filename))
    {
        strcat(productsFromCategory, "  ");
        strcat(productsFromCategory, buffer);
    }
    fclose(filename);
}

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in from;
    char message[SIZE] = "";
    char response[SIZE] = "";
    int sd, id = 0;

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
    printf("Server is on, waiting for connections..\n");
    fflush(stdout);

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
        id++;
        printf("Customer %d connected to server!\n", id);
        fflush(stdout);

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
                    printf("Customer %d logged succesfully!\n", id);
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
                    printf("Customer %d logged out!\n", id);
                    fflush(stdout);
                    exit;
                }

                bzero(message, SIZE);
                if (findUser(response) == 1)
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
                    perror("Eroarela write() catre client.\n");
                }
                if (write(client, &message, sizeof(int) * length) <= 0)
                {
                    perror("Eroare la write() catre client.\n");
                }
                bzero(response, SIZE);
                bzero(message, SIZE);
                if (read(client, &length, sizeof(int)) <= 0)
                {
                    perror("Eroare la read() de la client.\n");
                }
                if (read(client, &response, sizeof(int) * length) <= 0)
                {
                    perror("Eroare la read() de la client.\n");
                }
                if (strcmp(response, "1") == 0)
                {
                    strcpy(message, categories);
                    while (1) //intru in categorii
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
                        bzero(message, SIZE);
                        if (read(client, &length, sizeof(int)) <= 0)
                        {
                            perror("Eroare la read() de la client.\n");
                        }
                        if (read(client, &response, sizeof(int) * length) <= 0)
                        {
                            perror("Eroare la read() de la client.\n");
                        }
                        if (strcmp(response, "6") == 0)
                        {
                            strcpy(message, menu);
                            break;
                        }
                        if (strcmp(response, "1") == 0)
                        {
                            getProductsFromCategory("appliances");
                            strcpy(message, productsFromCategory);
                        }
                        else if (strcmp(response, "2") == 0)
                        {
                            getProductsFromCategory("sport");
                            strcpy(message, productsFromCategory);
                        }
                        else if (strcmp(response, "3") == 0)
                        {
                            getProductsFromCategory("house");
                            strcpy(message, productsFromCategory);
                        }
                        else if (strcmp(response, "4") == 0)
                        {
                            getProductsFromCategory("garden");
                            strcpy(message, productsFromCategory);
                        }
                        else if (strcmp(response, "5") == 0)
                        {
                            getProductsFromCategory("it");
                            strcpy(message, productsFromCategory);
                        }
                        else
                        {
                            strcpy(message, "\n");
                            strcat(message, categoryNotFound);
                        }
                        strcat(message, "\n");
                        strcat(message, categories);
                    }
                }
                else if (strcmp(response, "2") == 0)
                {
                    if (strcmp(cart, "") == 0)
                    {
                        strcpy(message, "Your cart is empty!\n\n");
                    }
                    else
                    {
                        strcpy(message, "You currently have in your cart: \n");
                        strcat(message, cart);
                        strcat(message, "\n");
                    }
                    strcat(message, menu);
                }
                else if (strcmp(response, "3") == 0)
                {

                    if (strcmp(cart, "") == 0)
                    {
                        strcpy(message, "Your cart is empty!\n\n");
                    }

                    strcat(message, "Do you want to add anything else to your cart?\n[1]Yes\n[2]No\nResponse: ");

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
                    bzero(message, SIZE);
                    if (read(client, &length, sizeof(int)) <= 0)
                    {
                        perror("Eroare la read() de la client.\n");
                    }
                    if (read(client, &response, sizeof(int) * length) <= 0)
                    {
                        perror("Eroare la read() de la client.\n");
                    }
                    if (strstr(response, "1") || strstr(response, "2"))
                    {
                        int opt = atoi(response);
                        saveCart(opt);
                        strcpy(message, resultCommand);
                        strcat(message, "\n");
                        strcat(message, menu);
                    }
                    else
                    {
                        strcpy(message, "Not a valid option!\nTry again!\n");
                        strcat(message, menu);
                    }
                }
                else if (strcmp(response, "4") == 0)
                {

                    while (1)
                    {
                        strcat(message, "Choose the number of the category for your item: ");
                        strcat(message, categories);
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
                        bzero(message, SIZE);
                        if (read(client, &length, sizeof(int)) <= 0)
                        {
                            perror("Eroare la read() de la client.\n");
                        }
                        if (read(client, &response, sizeof(int) * length) <= 0)
                        {
                            perror("Eroare la read() de la client.\n");
                        }
                        if (strcmp(response, "1") == 0)
                        {
                            bzero(response, SIZE);
                            strcpy(response, "appliances");
                        }
                        else if (strcmp(response, "2") == 0)
                        {
                            bzero(response, SIZE);
                            strcpy(response, "sport");
                        }
                        else if (strcmp(response, "3") == 0)
                        {
                            bzero(response, SIZE);
                            strcpy(response, "house");
                        }
                        else if (strcmp(response, "4") == 0)
                        {
                            bzero(response, SIZE);
                            strcpy(response, "garden");
                        }
                        else if (strcmp(response, "5") == 0)
                        {
                            bzero(response, SIZE);
                            strcpy(response, "it");
                        }
                        else if (strcmp(response, "6") == 0)
                        {
                            strcpy(message, menu);
                            break;
                        }
                        else
                        {
                            strcpy(message, categoryNotFound);
                        }

                        if (checkIfCategExists(response) == -1)
                        {
                            strcpy(message, resultCommand);
                            continue;
                        }
                        else
                        {
                            strcpy(message, "What item do you want to add to cart?: ");
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
                            bzero(message, SIZE);
                            if (read(client, &length, sizeof(int)) <= 0)
                            {
                                perror("Eroare la read() de la client.\n");
                            }
                            if (read(client, &response, sizeof(int) * length) <= 0)
                            {
                                perror("Eroare la read() de la client.\n");
                            }
                            if (addItem(response) == 1)
                            {
                                strcpy(message, "Item adeed to cart!\n\n");
                                strcat(message, menu);
                                length = strlen(message);
                                if (write(client, &length, sizeof(int)) <= 0)
                                {
                                    perror("Eroare la write() catre client.\n");
                                }
                                if (write(client, &message, sizeof(int) * length) <= 0)
                                {
                                    perror("Eroare la write() catre client.\n");
                                }
                                break;
                            }
                            else
                            {
                                strcpy(message, resultCommand);
                                continue;
                            }
                        }
                    }
                }
                else if (strcmp(response, "5") == 0)
                {
                   
                    if (strcmp(cart, "") == 0)
                    {
                        strcpy(message, "Empty cart!\n\n");
                    }
                    else
                    {
                        strcpy(message, "What item do you want to remove?: ");
                    }

                    length = strlen(message);
                    if (write(client, &length, sizeof(int)) <= 0)
                    {
                        perror("Eroare la write() catre client.\n");
                    }
                    if (write(client, &message, sizeof(int) * length) <= 0)
                    {
                        perror("Eroare la write() catre client.\n");
                    }

                    if (strstr(message, "Empty cart!") == NULL)
                    {
                        bzero(response, SIZE);
                        bzero(message, SIZE);
                        if (read(client, &length, sizeof(int)) <= 0)
                        {
                            perror("Eroare la read() de la client.\n");
                        }
                        if (read(client, &response, sizeof(int) * length) <= 0)
                        {
                            perror("Eroare la read() de la client.\n");
                        }
                        removeItem(response);
                        strcpy(message, resultCommand);
                        strcat(message, "\n");
                        length = strlen(message);
                        if (write(client, &length, sizeof(int)) <= 0)
                        {
                            perror("Eroare la write() catre client.\n");
                        }
                        if (write(client, &message, sizeof(int) * length) <= 0)
                        {
                            perror("Eroare la write() catre client.\n");
                        }
                    }
                    bzero(message, SIZE);
                    strcat(message, menu);
                }
                else if (strcmp(response, "6") == 0)
                {

                    placeOrder();
                    strcpy(message, resultCommand);
                    strcat(message, menu);
                }
                else if (strcmp(response, "7") == 0)
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
                    printf("Customer %d logged out!\n", id);
                    fflush(stdout);

                    return 0;
                }
                else
                {
                    strcpy(message, cmdNotFound);
                    strcat(message, menu);
                }
            }
        }
        else // parinte
        {
            close(client);
        }
    }
}
