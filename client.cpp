#include <stdio.h>
#include <iostream>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "Utils/json.hpp"
#include "Utils/json_fwd.hpp"
#include "Utils/helpers.h"

#define IP (char*)"34.254.242.81"
#define PORT 8080
#define IP_PORT (char*)"34.254.242.81:8080"

int main() {

    char* message;
    char* response;
    int sockfd;

    int logged_in = 0; // ce se intampla daca sunt logat si dau register?
    std::string userCookie;

    int access = 0;
    std::string tokenJWT;

    while(1) {
        std::string command;
        std::cin >> command;
        
        if(command == "register") { // merge cu == pt ca e string
            // POST REEQUEST
            std::string username;
            std::cout << "username:";
            // std::cin.ignore(); // citeste un caracter si il ignora
            // std::getline(std::cin,  username); // citeste toata linia
            std::cin >> username;

            std::string password;
            std::cout << "password:";
            // std::cin.ignore();
            // std::getline(std::cin,  password);
            std::cin >> password;

            nlohmann::json user = {
                {"username", username},
                {"password", password}
            };
            std::cout << user.dump() << std::endl;

            // generez mesajul de register
            message = compute_post_request(
                IP_PORT,
                (char *)"/api/v1/tema/auth/register",
                (char *)"application/json",
                user.dump(),
                1,
                NULL,
                0
            );

            // deschid conexiunea cu serverul
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific daca am primit eroare
            if(strstr(response, "error")) {
                std::cout << "Error: The username " << username << " is taken!\n";
            } else {
                std::cout << "User added succesfully!" << std::endl;
            }

            // inchid conexiunea cu serverul
            close_connection(sockfd);


        }else if (command == "login") {
            // POST REQUEST
            std::string username;
            std::cout << "username:";
            // std::cin.ignore();
            // std::getline(std::cin,  username);
            std::cin >> username;

            std::string password;
            std::cout << "password:";
            // std::cin.ignore();
            // std::getline(std::cin,  password);
            std::cin >> password;

            nlohmann::json user = {
                {"username", username},
                {"password", password}
            };

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            // generez mesajul de login
            message = compute_post_request(
                IP_PORT,
                (char *)"/api/v1/tema/auth/login",
                (char *)"application/json",
                user.dump(),
                1,
                NULL,
                0
            );

            // trimit cerere login
            send_to_server(sockfd, message);

            // primesc raspuns login
            response = receive_from_server(sockfd);

            // verific daca am primit eroare
            if(strstr(response, "error")) {
                std::cout << "Error: Credentials are not good!" << std::endl;
            } else {
                std::cout << "Login succesful!" << std::endl;
                logged_in = 1;

                unsigned int start = std::string(response).find("Cookie: ");
                unsigned int end = std::string(response).find(";", start);

                userCookie = std::string(response).substr(start, end - start);
            }

            // inchid conexiunea cu serverul
            close_connection(sockfd);

        } else if(command == "get_access") {
            // GET REQUEST
            if(logged_in == 0) {
                std::cout << "Error: You are not logged in!" << std::endl;
                continue;
            }

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            // fac cookies
            std::string cookies[10];
            cookies[0] = userCookie;

            // generez mesajul de get_access
            message = compute_get_request(
                IP_PORT,
                (char *)"/api/v1/tema/library/access",
                NULL,
                cookies,
                1
            );

            // trimit cererea
            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);
            if(strstr(response, "error")) { // nu ar trebui sa intre pe cazul asta
                std::cout << "Error: You are not logged in!" << std::endl;
            } else {
                std::cout << "Access granted!" << std::endl;
                access = 1;

                unsigned int start = std::string(response).find("token"); // {"token":"..."}
                unsigned int end = std::string(response).find("}", start);

                tokenJWT = "Authorization: Bearer " + 
                                    std::string(response).substr(start + strlen("token") + 3, // 3 pt ":"
                                                         end - start - strlen("token") - 4); // 4 pt ":"}
            }

            // inchid conexiunea cu sv
            close_connection(sockfd);

        } else if(command == "get_books") {
            // GET REQUEST

            if(!access) { // verific daca am access la carti
                std::cout << "Error: Authorization header is missing!" << std::endl;
                continue;
            }

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            std::string tokens[10];
            tokens[0] = tokenJWT;

            // generez mesajul de get_books
            message = compute_get_request(
                IP_PORT,
                (char *)"/api/v1/tema/library/books",
                NULL,
                tokens,
                1
            );

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            if(strstr(response, "error")) {
                std::cout << "Error: Authorization header is missing!" << std::endl;
            } else {
                unsigned int start = std::string(response).find("[");
                unsigned int end = std::string(response).find("]", start);

                std::string books = std::string(response).substr(start, end - start + 1);
                nlohmann::json carti = nlohmann::json::parse(books);
                std::cout<< carti << std::endl;
                
            }

        } else if(command == "get_book") {

        } else if(command == "add_book") {
            // POST REQUEST
            if(!access) {
                std::cout << "Error: Authorization header is missing!" << std::endl;
                continue;
            }

            std::string title;
            std::cout << "title:";
            std::cin.ignore();
            std::getline(std::cin, title);

            std::string author;
            std::cout << "author:";
            std::cin.ignore();
            std::getline(std::cin, author);

            std::string genre;
            std::cout << "genre:";
            std::cin.ignore();
            std::getline(std::cin, genre);

            std::string page_count;
            std::cout << "page_count:";
            std::cin >> page_count;

            std::string publisher;
            std::cout << "publisher:";
            std::cin.ignore();
            std::getline(std::cin, publisher);

            nlohmann::json book = {
                {"title", title},
                {"author", author},
                {"genre", genre},
                {"page_count", page_count},
                {"publisher", publisher}
            };

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            std::string tokens[10];
            tokens[0] = tokenJWT;

            message = compute_post_request(
                IP_PORT,
                (char *)"/api/v1/tema/library/books",
                (char *)"application/json",
                book.dump(),
                1,
                tokens,
                1
            );

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            if(strstr(response, "error")) {
                std::cout << "Error: Wrong Format" << std::endl;
            } else {
                std::cout << "Book added!" << std::endl;

            }

            // inchid conexiunea cu sv
            close_connection(sockfd);


        } else if (command == "exit") {
            break; // vreau sa ies
        }
    }

    return 0;
}
