#include <iostream>
#include <string>

#include "WeatherClient.h"
#include "AuthService.h"
#include "Database.h"
#include "HttpServer.h"

// I keep usage printing separate so argument handling stays readable.
void printUsage() {
    std::cout << "Usage:\n"
              << "  WeatherApp --cli\n"
              << "  WeatherApp --server <address> <port>\n";
}

int main(int argc, char* argv[]) {
    // I require an explicit mode to avoid ambiguous startup behavior.
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string mode = argv[1];

    try {
        // I create shared services once and reuse them across modes.
        Database db("weather.db");
        AuthService auth(db);

        if (mode == "--cli") {
            WeatherClient weather;

            std::cout << "Welcome to WeatherApp CLI\n";

            // I run the CLI as a simple loop to keep interaction state explicit.
            while (true) {
                std::cout << "\n1. Register\n2. Login\n3. Exit\nChoice: ";
                int choice;
                std::cin >> choice;

                if (choice == 3) break;

                std::string username, password;
                std::cout << "Username: ";
                std::cin >> username;
                std::cout << "Password: ";
                std::cin >> password;

                if (choice == 1) {
                    if (auth.registerUser(username, password)) {
                        std::cout << "Registration successful.\n";
                    } else {
                        std::cout << "Registration failed.\n";
                    }
                }
                else if (choice == 2) {
                    int userId = auth.loginUser(username, password);
                    if (userId < 0) {
                        std::cout << "Login failed.\n";
                        continue;
                    }

                    std::cout << "Login successful.\n";

                    // I keep the post-login loop separate so session state is clear.
                    while (true) {
                        std::cout << "\nEnter city (or 'back'): ";
                        std::string city;
                        std::cin >> city;
                        if (city == "back") break;

                        std::string summary = weather.getWeather(city);
                        std::cout << summary << "\n";

                        db.logQuery(userId, city, summary);
                    }
                }
            }
        }
        else if (mode == "--server") {
            // I validate arguments early to fail fast on misconfiguration.
            if (argc != 4) {
                printUsage();
                return 1;
            }

            std::string address = argv[2];
            int port = std::stoi(argv[3]);

            // I hand ownership of shared services to the server via references.
            HttpServer server(address, port, db, auth);
            server.run();
        }
        else {
            printUsage();
            return 1;
        }
    }
    catch (const std::exception& ex) {
        // I treat any exception here as fatal since recovery is undefined.
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
