// HospitalServer.cpp
#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <thread>
#include <string>
#include <sstream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to handle each client
void handleClient(SOCKET clientSocket, int clientId) {
    char buffer[BUFFER_SIZE];
    string clientTag = "Client" + to_string(clientId);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            cout << clientTag << " disconnected.\n";
            break;
        }

        string command(buffer);
        string response;
        cout << "[" << clientTag << "] " << command << endl;

        // ----- PATIENT management -----
        if (command.rfind("PATIENT_ADD:", 0) == 0) {
            string data = command.substr(12);
            ofstream file("patient.txt", ios::app);
            if (file.is_open()) {
                file << data << endl;
                file.close();
                response = "Patient record added successfully.";
            }
            else {
                response = "Error opening patient.txt.";
            }
        }
        else if (command == "PATIENT_VIEW") {
            ifstream file("patient.txt");
            if (file.is_open()) {
                string line;
                while (getline(file, line)) {
                    response += line + "\n";
                }
                file.close();
                if (response.empty()) response = "No patient records found.";
            }
            else {
                response = "Error opening patient.txt.";
            }
        }
        else if (command.rfind("PATIENT_SEARCH:", 0) == 0) {
            string keyword = command.substr(15);
            ifstream file("patient.txt");
            bool found = false;
            if (file.is_open()) {
                string line;
                while (getline(file, line)) {
                    if (line.find(keyword) != string::npos) {
                        response += line + "\n";
                        found = true;
                    }
                }
                file.close();
                if (!found) response = "No matching patient found.";
            }
            else {
                response = "Error opening patient.txt.";
            }
        }
        else if (command.rfind("PATIENT_DELETE:", 0) == 0) {
            string delId = command.substr(15);
            ifstream file("patient.txt");
            ofstream temp("temp.txt");
            bool deleted = false;
            if (file.is_open() && temp.is_open()) {
                string line;
                while (getline(file, line)) {
                    if (line.substr(0, line.find(',')) == delId) {
                        deleted = true;
                        continue;
                    }
                    temp << line << endl;
                }
                file.close();
                temp.close();
                remove("patient.txt");
                rename("temp.txt", "patient.txt");
                response = deleted ? "Patient deleted." : "Patient not found.";
            }
            else {
                response = "File error during deletion.";
            }
        }

        // ----- DOCTOR management -----
        else if (command.rfind("DOCTOR_ADD:", 0) == 0) {
            string data = command.substr(11);
            ofstream file("doctor.txt", ios::app);
            if (file.is_open()) {
                file << data << endl;
                file.close();
                response = "Doctor record added successfully.";
            }
            else {
                response = "Error opening doctor.txt.";
            }
        }
        else if (command == "DOCTOR_VIEW") {
            ifstream file("doctor.txt");
            if (file.is_open()) {
                string line;
                while (getline(file, line)) {
                    response += line + "\n";
                }
                file.close();
                if (response.empty()) response = "No doctor records found.";
            }
            else {
                response = "Error opening doctor.txt.";
            }
        }
        else if (command.rfind("DOCTOR_SEARCH:", 0) == 0) {
            string keyword = command.substr(14);
            ifstream file("doctor.txt");
            bool found = false;
            if (file.is_open()) {
                string line;
                while (getline(file, line)) {
                    if (line.find(keyword) != string::npos) {
                        response += line + "\n";
                        found = true;
                    }
                }
                file.close();
                if (!found) response = "No matching doctor found.";
            }
            else {
                response = "Error opening doctor.txt.";
            }
        }
        else if (command.rfind("DOCTOR_DELETE:", 0) == 0) {
            string delId = command.substr(14);
            ifstream file("doctor.txt");
            ofstream temp("temp.txt");
            bool deleted = false;
            if (file.is_open() && temp.is_open()) {
                string line;
                while (getline(file, line)) {
                    if (line.substr(0, line.find(',')) == delId) {
                        deleted = true;
                        continue;
                    }
                    temp << line << endl;
                }
                file.close();
                temp.close();
                remove("doctor.txt");
                rename("temp.txt", "doctor.txt");
                response = deleted ? "Doctor deleted." : "Doctor not found.";
            }
            else {
                response = "File error during deletion.";
            }
        }
        // ----- IMAGE handling -----
        else if (command.rfind("IMAGE_UPLOAD:", 0) == 0) {
            string filename = command.substr(13);
            ofstream file(filename, ios::binary);
            if (!file.is_open()) {
                response = "Error creating file.";
                send(clientSocket, response.c_str(), response.length(), 0);
                continue;
            }

            // Send acknowledgement to client to start sending image data
            string ack = "READY_FOR_IMAGE";
            send(clientSocket, ack.c_str(), ack.length(), 0);

            // Receive image data in chunks
            char buffer[BUFFER_SIZE];
            int totalBytes = 0;
            while (true) {
                memset(buffer, 0, BUFFER_SIZE);
                int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesReceived <= 0 || string(buffer).find("IMAGE_END") != string::npos) {
                    break;
                }
                file.write(buffer, bytesReceived);
                totalBytes += bytesReceived;
            }
            file.close();
            response = "Image uploaded successfully. Size: " + to_string(totalBytes) + " bytes";
            }
        else if (command.rfind("IMAGE_DOWNLOAD:", 0) == 0) {
                string filename = command.substr(15);
                ifstream file(filename, ios::binary);
                if (!file.is_open()) {
                    response = "Error opening image file.";
                    send(clientSocket, response.c_str(), response.length(), 0);
                    continue;
                }

                // Get file size
                file.seekg(0, ios::end);
                size_t fileSize = file.tellg();
                file.seekg(0, ios::beg);

                // Send file size first
                response = "FILE_SIZE:" + to_string(fileSize);
                send(clientSocket, response.c_str(), response.length(), 0);

                // Wait for client ready signal
                char readyBuffer[20];
                recv(clientSocket, readyBuffer, 20, 0);
                if (string(readyBuffer).find("READY_FOR_DATA") == string::npos) {
                    response = "Transfer aborted by client.";
                    send(clientSocket, response.c_str(), response.length(), 0);
                    continue;
                }

                // Send file in chunks
                char buffer[BUFFER_SIZE];
                while (!file.eof()) {
                    file.read(buffer, BUFFER_SIZE);
                    int bytesRead = file.gcount();
                    send(clientSocket, buffer, bytesRead, 0);
                }
                file.close();

                // Send end marker
                string endMarker = "IMAGE_END";
                send(clientSocket, endMarker.c_str(), endMarker.length(), 0);
                continue; // Skip the final send at the end of the loop
                }
                // ----- IMAGE Path handling -----
        else if (command.rfind("PATIENT_IMAGE:", 0) == 0) {
            // Format: PATIENT_IMAGE:patientID,imagePath
            size_t colonPos = command.find(':');
            size_t commaPos = command.find(',');

            if (commaPos == string::npos) {
                response = "Invalid PATIENT_IMAGE command format";
            }
            else {
                string patientId = command.substr(colonPos + 1, commaPos - colonPos - 1);
                string imagePath = command.substr(commaPos + 1);

                // Store in patient_images.txt (patientID,imagePath)
                ofstream file("patient_images.txt", ios::app);
                if (file.is_open()) {
                    file << patientId << "," << imagePath << endl;
                    file.close();
                    response = "Patient image path stored successfully";
                }
                else {
                    response = "Error opening patient_images.txt";
                }
            }
            }
        else if (command.rfind("DOCTOR_IMAGE:", 0) == 0) {
                // Similar to PATIENT_IMAGE but for doctors
                size_t colonPos = command.find(':');
                size_t commaPos = command.find(',');

                if (commaPos == string::npos) {
                    response = "Invalid DOCTOR_IMAGE command format";
                }
                else {
                    string doctorId = command.substr(colonPos + 1, commaPos - colonPos - 1);
                    string imagePath = command.substr(commaPos + 1);

                    // Store in doctor_images.txt (doctorID,imagePath)
                    ofstream file("doctor_images.txt", ios::app);
                    if (file.is_open()) {
                        file << doctorId << "," << imagePath << endl;
                        file.close();
                        response = "Doctor image path stored successfully";
                    }
                    else {
                        response = "Error opening doctor_images.txt";
                    }
                }
                }
        else if (command.rfind("GET_PATIENT_IMAGE:", 0) == 0) {
                    string patientId = command.substr(17);
                    ifstream file("patient_images.txt");
                    bool found = false;

                    if (file.is_open()) {
                        string line;
                        while (getline(file, line)) {
                            size_t commaPos = line.find(',');
                            if (commaPos != string::npos && line.substr(0, commaPos) == patientId) {
                                response = line.substr(commaPos + 1);
                                found = true;
                                break;
                            }
                        }
                        file.close();
                        if (!found) response = "No image found for this patient";
                    }
                    else {
                        response = "Error opening patient_images.txt";
                    }
                    }
        else if (command.rfind("GET_DOCTOR_IMAGE:", 0) == 0) {
                        // Similar to GET_PATIENT_IMAGE but for doctors
                        string doctorId = command.substr(16);
                        ifstream file("doctor_images.txt");
                        bool found = false;

                        if (file.is_open()) {
                            string line;
                            while (getline(file, line)) {
                                size_t commaPos = line.find(',');
                                if (commaPos != string::npos && line.substr(0, commaPos) == doctorId) {
                                    response = line.substr(commaPos + 1);
                                    found = true;
                                    break;
                                }
                            }
                            file.close();
                            if (!found) response = "No image found for this doctor";
                        }
                        else {
                            response = "Error opening doctor_images.txt";
                        }
            }
        else {
            response = "Invalid command.";
        }

        // Send response to client
        send(clientSocket, response.c_str(), response.length(), 0);
    }

    closesocket(clientSocket);
}


int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    cout << "Server listening on port " << PORT << "...\n";

	int clientCounter = 0; // Counter for client IDs/unique id counter

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		clientCounter++; // Increment client ID
        cout << "Client" << clientCounter << " connected.\n";
        thread t(handleClient, clientSocket, clientCounter);
        t.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
