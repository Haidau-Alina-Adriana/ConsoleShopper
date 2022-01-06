#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define PORT 2020
#define SIZE 1000
extern int errno;
char loginMessage[] = "You have to be logged first!\nType \"exit\" to leave the app or enter your username: ";
char userNotFound[] = "Couldn't find you in our database. Try again.\nYour username: ";
char succesLogin[] = "\nYou're succesfully logged in!";
char succesAdmin[] = "\nYou're logged as admin!";
char menu[] = "Options:\n[1]see categories\n[2]see cart\n[3]save cart\n[4]add item in cart\n[5]remove item from cart\n[6]place order\n[7]cancel order\n[8]logout\nYour option: ";
char succesLogout[] = "You're succesfully logged out!";
char adminMenu[] = "[1]add item to list\n[2]remove item from list\n[3]add user\n[4]remove user\n[5]logout\nYour option: ";
char categories[] = "\nAvailable categories:\n[1]appliances\n[2]sport\n[3]house\n[4]garden\n[5]technology\n[6]go back\nYour choice:";
char cmdNotFound[] = "Command not available!Try again: \n";
char categoryNotFound[] = "Category does not exists!";
char productsFromCategory[300];
char cart[300];
char path[50];
char currentUser[20];
time_t p0;
time_t p1;
int cartState = 1;
int placedOrder = 0;
char resultCommand[1000];

void saveCart(int option)
{
    memset(resultCommand, 0, 1000);
    if (option == 2)
    {
        return;
    }
    else if (option == 1)
    {
        strcpy(resultCommand, "Cart saved!\n\n");
        cartState = 1;
    }
    else
    {
        strcpy(resultCommand, "Option not available!\n");
    }
}
void cancelOrder(int option)
{
    time(&p1);

    memset(resultCommand, 0, 1000);
    if (option == 1)
    {
        if (p1 - p0 < 10)
        {
            placedOrder = 0;
            strcpy(resultCommand, "Your order is cancelled!\n\n");
        }
        else
        {
            strcpy(resultCommand, "You can't cancel the order!\n\n");
        }
    }
    else
    {
        strcpy(resultCommand, "Your order is safe!\n\n");
    }
}
void placeOrder()
{
    memset(resultCommand, 0, 1000);
    if (placedOrder == 0)
    {
        if (cartState == 1)
        {
            if (strcmp(cart, "") == 0)
            {
                strcpy(resultCommand, "Add some items to your cart!\n\n");
            }
            else
            {
                strcpy(resultCommand, "Your order has been registered!\nYour order: \n");
                strcat(resultCommand, cart);
                strcat(resultCommand, "\n\n");
                placedOrder = 1;
                time(&p0);
            }
        }
        else
        {
            strcpy(resultCommand, "Please save your cart first to place the order!\n\n");
        }
    }
    else
    {
        strcpy(resultCommand, "Already placed!\n\n");
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
        strcpy(resultCommand, "Item removed from cart!");
    }
}
int login(char *username)
{
    FILE *configFile;
    int bufferLen = 255;
    char buffer[bufferLen];
    char rez[1000];
    int retValue = -1;
    configFile = fopen("configFile.txt", "r");
    if (configFile == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    memset(rez, 0, 1000);
    while (fgets(buffer, bufferLen, configFile))
    {
        char copyLine[100];
        strcpy(copyLine, buffer);
        char *token;
        char delim[3] = "->";
        token = strtok(copyLine, delim);
        if (strcmp(token, username) == 0)
        {
            if (strstr(buffer, "State:0") != 0)
            {
                strcat(rez, username);
                strcat(rez, "->");
                strcat(rez, "State:1\n");
                retValue = 1;
                strcpy(currentUser, username);
            }
            else
            {
                retValue = 2;
                strcat(rez, buffer);
            }
        }
        else
        {
            strcat(rez, buffer);
        }
    }
    if (retValue == 1)
    {
        configFile = fopen("configFile.txt", "w");
        if (configFile == NULL)
        {
            perror("Error at opening the file for reading!");
        }
        fputs(rez, configFile);
    }
    fclose(configFile);
    return retValue;
}
void addUser(char *username)
{
    FILE *filename;
    int bufferLen = 255;
    char buffer[bufferLen];
    char rez[20];
    filename = fopen("configFile.txt", "r");
    if (filename == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    memset(resultCommand, 0, 1000);
    memset(rez, 0, 20);
    int foundIt = 0;
    while (fgets(buffer, bufferLen, filename))
    {
        char copyLine[100];
        strcpy(copyLine, buffer);
        char *token;
        char delim[3] = "->";
        token = strtok(copyLine, delim);
        if (strcmp(token, username) == 0)
        {
            foundIt = 1;
            break;
        }
    }
    if (foundIt == 1)
    {
        strcpy(resultCommand, "User already exists!\n");
    }
    else
    {
        filename = fopen("configFile.txt", "a");
        if (filename == NULL)
        {
            perror("Error at opening the file for reading!");
        }
        strcpy(rez, "\n");
        strcat(rez, username);
        strcat(rez, "->State:0");
        fputs(rez, filename);
        strcpy(resultCommand, "User added!\n");
    }
    fclose(filename);
}
int removeUser(char *username)
{
    FILE *filename;
    int bufferLen = 255;
    char buffer[bufferLen];
    char rez[1000];
    filename = fopen("configFile.txt", "r");
    if (filename == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    memset(rez, 0, 1000);
    memset(resultCommand, 0, 1000);
    int foundIt = 0;
    while (fgets(buffer, bufferLen, filename))
    {
        char copyLine[100];
        strcpy(copyLine, buffer);
        char *token;
        char delim[3] = "->";
        token = strtok(copyLine, delim);
        if (strcmp(token, username) != 0)
        {
            strcat(rez, buffer);
        }
        else
        {
            foundIt = 1;
        }
    }
    if (foundIt == 0)
    {
        strcpy(resultCommand, "User not found!\n");
    }
    else
    {
        filename = fopen("configFile.txt", "w");
        if (filename == NULL)
        {
            perror("Error at opening the file for reading!");
        }
        rez[strlen(rez)] = '\0';
        fputs(rez, filename);
        strcpy(resultCommand, "User removed!\n");
    }
    fclose(filename);
}
int addItemInOptions(char *item)
{
    FILE *filename;
    int bufferLen = 255;
    char buffer[bufferLen];
    filename = fopen(path, "r");
    char rez[20];
    memset(resultCommand, 0, 1000);
    memset(rez, 0, 20);
    if (filename == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    while (fgets(buffer, bufferLen, filename))
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, item) == 0)
        {
            strcat(resultCommand, "Item already exists!\n");
            fclose(filename);
            return -1;
        }
    }
    filename = fopen(path, "a");
    if (filename == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    strcpy(rez, "\n");
    strcat(rez, item);
    fputs(rez, filename);
    strcpy(resultCommand, "Item added!\n");
    fclose(filename);
    return 1;
}

int removeItemFromOptions(char *item)
{
    FILE *filename;
    int bufferLen = 255;
    char buffer[bufferLen];
    filename = fopen(path, "r");
    char rez[500];
    memset(resultCommand, 0, 1000);
    memset(rez, 0, 500);
    if (filename == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    int foundIt = 0;
    while (fgets(buffer, bufferLen, filename))
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, item) != 0)
        {
            strcat(rez, buffer);
            strcat(rez, "\n");
        }
        else
        {
            foundIt = 1;
        }
    }
    if (foundIt == 1)
    {
        filename = fopen(path, "w");
        if (filename == NULL)
        {
            perror("Error at opening the file for reading!");
        }
        rez[strlen(rez)-1] = '\0';
        fputs(rez, filename);
        strcpy(resultCommand, "Item removed!\n");
        fclose(filename);
        return 1;
    }
    else
    {
        strcpy(resultCommand, "Item not found!\n");
    }
    fclose(filename);
    return -1;
}
int getProductsFromCategory(char *category)
{
    FILE *filename;
    int bufferLen = 255;
    char buffer[bufferLen];
    memset(path, 0, 50);
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

void logout()
{
    FILE *configFile;
    int bufferLen = 255;
    char buffer[bufferLen];
    char rez[1000];
    configFile = fopen("configFile.txt", "r");
    if (configFile == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    memset(rez, 0, 1000);
    while (fgets(buffer, bufferLen, configFile))
    {
        char copyLine[100];
        strcpy(copyLine, buffer);
        char *token;
        char delim[3] = "->";
        token = strtok(copyLine, delim);
        if (strcmp(token, currentUser) == 0)
        {
            if (strstr(buffer, "State:1") != 0)
            {
                strcat(rez, currentUser);
                strcat(rez, "->");
                strcat(rez, "State:0\n");
            }
            else
            {
                strcat(rez, buffer);
            }
        }
        else
        {
            strcat(rez, buffer);
        }
    }
    configFile = fopen("configFile.txt", "w");
    if (configFile == NULL)
    {
        perror("Error at opening the file for reading!");
    }
    fputs(rez, configFile);

    fclose(configFile);
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
        printf("Client %d connected to server!\n", id);
        fflush(stdout);

        strcpy(message, loginMessage);
        int forkPid = fork();

        if (forkPid == 0) // copil
        {
            strcpy(message, "You can log as admin or user!\nExit or Log as: ");
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
            if (strcmp(response, "exit") == 0)
            {
                printf("Client %d exited!\n", id);
                fflush(stdout);
                exit;
            }
            strcpy(message, loginMessage);
            response[strcspn(response, "\n")] = '\0';
            if (strcmp(response, "admin")!= 0)
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
                        printf("User %d logged succesfully!\n", id);
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
                        printf("Client %d exited!\n", id);
                        fflush(stdout);
                        exit;
                    }

                    bzero(message, SIZE);
                    int resultFindUser = login(response);
                    if (resultFindUser == -1)
                    {
                        strcat(message, userNotFound);
                    }
                    else if (resultFindUser == 1)
                    {
                        strcat(message, succesLogin);
                    }
                    else
                    {
                        strcat(message, "User already connected!Try a dfifferent one!\nYour username: ");
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
                                getProductsFromCategory("technology");
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

                        strcat(message, "Are you sure?\n[1]Yes\n[2]No\nResponse: ");

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
                        if (strcmp(response, "1") == 0 || strcmp(response, "2") == 0)
                        {
                            int opt = atoi(response);
                            saveCart(opt);
                            strcpy(message, resultCommand);
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
                                strcpy(response, "technology");
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
                        if (placedOrder == 0)
                        {
                            strcpy(message, "Place the order first!Type ok: ");
                            //strcat(resultCommand,"\n\n");
                        }
                        else
                        {
                            strcpy(message, "Do you want to cancel your order?\n[1]Yes\n[2]No\nYour answer: ");
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
                        if (strcmp(response, "1") == 0 || strcmp(response, "2") == 0)
                        {
                            int opt = atoi(response);
                            cancelOrder(opt);
                            strcpy(message, resultCommand);
                            strcat(message, menu);
                        }
                        else if (strcmp(response, "ok") == 0)
                        {
                            strcat(message, menu);
                        }
                        else
                        {
                            strcpy(message, "Not a valid option!\nTry again!\n");
                            strcat(message, menu);
                        }
                    }
                    else if (strcmp(response, "8") == 0)
                    {
                        logout();
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
                        printf("Client %d logged out!\n", id);
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
            else
            {
                strcpy(message, adminMenu);
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
                        while (1)
                        {

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
                                strcpy(response, "technology");
                            }
                            else if (strcmp(response, "6") == 0)
                            {
                                strcpy(message, adminMenu);
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
                                strcpy(message, "What item do you want to add?: ");
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

                                if (addItemInOptions(response) == 1)
                                {
                                    strcpy(message, resultCommand);
                                    strcat(message, "\n");
                                    strcat(message, adminMenu);
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
                                }
                            }
                        }
                    }
                    else if (strcmp(response, "2") == 0)
                    {
                        while (1)
                        {

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
                                strcpy(response, "technology");
                            }
                            else if (strcmp(response, "6") == 0)
                            {
                                strcpy(message, adminMenu);
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
                                strcpy(message, "What item do you want to remove?: ");
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

                                if (removeItemFromOptions(response) == 1)
                                {
                                    strcpy(message, resultCommand);
                                    strcat(message, "\n");
                                    strcat(message, adminMenu);
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
                                }
                            }
                        }
                    }
                    else if (strcmp(response, "3") == 0)
                    {
                        strcpy(message, "User to add: ");
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
                        addUser(response);
                        strcpy(message, resultCommand);
                        strcat(message, "\n");
                        strcat(message, adminMenu);
                    }
                    else if (strcmp(response, "4") == 0)
                    {
                        strcpy(message, "User to remove: ");
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
                        removeUser(response);
                        strcpy(message, resultCommand);
                        strcat(message, "\n");
                        strcat(message, adminMenu);
                    }
                    else if (strcmp(response, "5") == 0)
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
                        printf("Client %d logged out!\n", id);
                        fflush(stdout);
                        return 0;
                    }else
                    {
                        strcpy(message, cmdNotFound);
                        strcat(message, adminMenu);
                    }
                }
            }
        }
        else // parinte
        {
            close(client);
        }
    }
}
