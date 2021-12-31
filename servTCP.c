#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define PORT 2020
#define SIZE 500
extern int errno;
char loginMessage[] = "You have to be logged first!\nType \"exit\" to leave the app or enter your username: ";
char userNotFound[] = "Couldn't find you in our database. Try again.\nYour username: ";
char succesLogin[] = "You're succesfully logged in!";
char menu[] = "Options:\n[1]see categories\n[2]see cart\n[3]save cart\n[4]add item in cart\n[5]remove item from cart\n[6]place order\n[7]logout\nYour option: ";
char succesLogout[] = "You're succesfully logged out!";
char categories[] = "\nAvailable categories:\n[1]appliances\n[2]sport\n[3]house\n[4]garden\n[5]it\n[6]go back\nYour choice:";
char cmdNotFound[] = "Command not available!Try again: \n";
char categoryNotFound[] = "Category not found!";
char productsFromCategory[300];
char cart[300];
char cartName[50];
int cartState = 1;
char resultCommand[1000];
FILE *fileCartName;

void seeCart()
{
    int bufferLen = 255;
    char buffer[bufferLen];
    fileCartName = fopen(cartName, "r");
    memset(cart, 0, 300);
    strcat(cart, "You currently have in your cart: \n");
    if (fileCartName == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    while (fgets(buffer, bufferLen, fileCartName))
    {
        strcat(cart, buffer);
    }
    fclose(fileCartName);
}
void createCart(int customerNumber)
{
    char buffer[2];
    char path[50];
    memset(path, 0, 50);
    strcpy(path, "cart");
    buffer[0] = customerNumber + '0';
    strcat(path, buffer);
    strcat(path, ".txt");
    fileCartName = fopen(path, "w");
    if (fileCartName == NULL)
    {
        perror("Error at creating the file/writing in file!");
    }
    strcpy(cartName, path);
    fclose(fileCartName);
}
void deleteCart()
{
    if (remove(cartName) != 0)
    {
        perror("The file could not be deleted!");
    }
}
void saveCart(int option)
{
    memset(resultCommand, 0, 1000);
    if (option == 1)
    {
        strcpy(resultCommand, categories);
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
            strcpy(resultCommand, "Add some items to your cart!\n");
        }
        else
        {
            strcpy(resultCommand, "Your order has been registered!\n");
        }
    }
    else
    {
        strcpy(resultCommand, "Please save your cart first to place the order!\n");
    }
}

int addItem(char *item)
{
    fileCartName = fopen(cartName, "a");
    if (fileCartName == NULL)
    {
        perror("Error at opening the file for appending!");
    }
    fputs(item, fileCartName);
    fputs("\n", fileCartName);
    fclose(fileCartName);
}
void removeItem(char *item)
{
    int bufferLen = 255;
    char buffer[bufferLen];
    memset(buffer, 0, bufferLen);
    fileCartName = fopen(cartName, "r");
    if (fileCartName == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    char cartContent[500] = "";
    while (fgets(buffer, bufferLen, fileCartName))
    {
        strcat(cartContent, buffer);
    }

    char result[500] = "";
    const char delim[2] = "\n";
    char *token;
    token = strtok(cartContent, delim);
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
    fileCartName = fopen(cartName, "w");
    fputs(result, fileCartName);
    fclose(fileCartName);
    
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
            createCart(id);
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
                            strcpy(message, categoryNotFound);
                        }
                        strcat(message, "\n");
                        strcat(message, categories);
                    }
                }
                else if (strcmp(response, "2") == 0)
                {
                    strcpy(message, cart);
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
                    deleteCart();
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
