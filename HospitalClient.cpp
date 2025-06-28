#include <iostream>  //standard input/output streams i.e. cin/cout
#include <winsock2.h>  // Windows Sockets API 
// WSAStartup, socket, connect, send, recv, closesocket, WSACleanup etc
// This header provides access to the Windows Sockets API (Winsock),
// allowing the program to handle network communications such as creating 
// and using sockets on Windows.
#include <ws2tcpip.h>  // Windows-specific TCP/IP functions // InetPton etc
//This is another Windows-specific networking header that provides newer functions like
//InetPton (used to convert IP addresses), which is not available in older winsock2.h.
#include <string>   // string class for handling strings // std::string
#include<fstream>  // file input/output streams i.e. ifstream/ofstream

#pragma comment(lib, "Ws2_32.lib")//linking library having implementation of Winsock functions

using namespace std;

#define SERVER_IP "127.0.0.1"//localhost IP address
#define PORT 8080// Port number for the server
#define BUFFER_SIZE 1024// Size of the buffer for receiving data


void uploadImage(SOCKET sock) {
    string filename;
    cout << "Enter image file path to upload: ";
    getline(cin, filename);

    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cout << "Error opening image file.\n";
        return;
    }

    // Send upload command
    string command = "IMAGE_UPLOAD:" + filename.substr(filename.find_last_of("/\\") + 1);
    send(sock, command.c_str(), command.length(), 0);

    // Wait for server ready signal
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytesReceived <= 0 || string(buffer).find("READY_FOR_IMAGE") == string::npos) {
        cout << "Server not ready for upload.\n";
        return;
    }

    // Send file in chunks
    char fileBuffer[BUFFER_SIZE];
    while (!file.eof()) {
        file.read(fileBuffer, BUFFER_SIZE);
        int bytesRead = file.gcount();
        send(sock, fileBuffer, bytesRead, 0);
    }
    file.close();

    // Send end marker
    string endMarker = "IMAGE_END";
    send(sock, endMarker.c_str(), endMarker.length(), 0);

    // Get server response
    bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        cout << "Server: " << buffer << endl;
    }
}

void downloadImage(SOCKET sock) {
    string filename;
    cout << "Enter image filename to download: ";
    getline(cin, filename);

    // Send download command
    string command = "IMAGE_DOWNLOAD:" + filename;
    send(sock, command.c_str(), command.length(), 0);

    // Receive file size
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytesReceived <= 0) {
        cout << "Error receiving file size.\n";
        return;
    }
    buffer[bytesReceived] = '\0';

    string response(buffer);
    if (response.find("FILE_SIZE:") == string::npos) {
        cout << "Error: " << response << endl;
        return;
    }

    size_t fileSize = stoull(response.substr(10));
    cout << "Downloading file of size: " << fileSize << " bytes\n";

    // Send ready signal
    string readyMsg = "READY_FOR_DATA";
    send(sock, readyMsg.c_str(), readyMsg.length(), 0);

    // Receive file
    ofstream outFile("downloaded_" + filename, ios::binary);
    if (!outFile.is_open()) {
        cout << "Error creating output file.\n";
        return;
    }

    size_t totalReceived = 0;
    while (totalReceived < fileSize) {
        bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) break;

        // Check for end marker
        if (string(buffer, bytesReceived).find("IMAGE_END") != string::npos) {
            bytesReceived -= 9; // length of "IMAGE_END"
            if (bytesReceived > 0) {
                outFile.write(buffer, bytesReceived);
                totalReceived += bytesReceived;
            }
            break;
        }

        outFile.write(buffer, bytesReceived);
        totalReceived += bytesReceived;
    }
    outFile.close();

    cout << "Download complete. Saved as downloaded_" << filename << endl;
}

void sendCommand(SOCKET sock, const string& command) // Function to send commands to the server
// This function takes a socket and a command string as input, sends the command to the server,
// and waits for a response. It handles the sending of the command and receiving the server's response.
// It uses the send() function to send the command and recv() to receive the response.
// Here the string is passed by reference to avoid unnecessary copying, and the command is sent as a C-style string using c_str().
{
	send(sock, command.c_str(), (int)command.length(), 0);
    //declaration of send function
    //int send(SOCKET s, const char* buf, int len, int flags);
	// s: the socket to send data on
	// buf: a pointer to the buffer containing the data to be sent
	// len: the length of the data to be sent
	// flags: additional options for sending data (usually set to 0)

    char buffer[BUFFER_SIZE] = { 0 };
	// declaration of buffer to store the response from the server
	// It is initialized to zero to ensure it is empty before receiving data.
	// It is an array of characters with a size defined by BUFFER_SIZE, which is set to 1024.
	
    // declaration of recv function
    int bytesReceived = recv(sock, buffer, BUFFER_SIZE - 1, 0);
	// int recv(SOCKET s, char* buf, int len, int flags);
	// s: the socket to receive data from
	// buf: a pointer to the buffer where the received data will be stored
	// len: the maximum number of bytes to receive (BUFFER_SIZE - 1 to leave space for null terminator)
    // flags: additional options for receiving data(usually set to 0)

	//if bytesReceived > 0, it means data was received successfully.
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        cout << "\n[Server Response]:\n" << buffer << endl;
    }
    else {
        cout << "No response from server or connection closed." << endl;
    }
}

// Function to show CRUD menu for a specific type (PATIENT or DOCTOR)
void showCrudMenu(SOCKET sock, const string& type) {
    int choice;
    string input;

    while (true) {
        cout << "\n--- " << type << " Management ---\n";
        cout << "1. Add " << type << "\n";
        cout << "2. View All " << type << "s\n";
        cout << "3. Search " << type << "\n";
        cout << "4. Delete " << type << "\n";
        cout << "5. Upload " << type << " Image\n";
        cout << "6. Download " << type << " Image\n";
        cout << "7. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore();

        string command;

        if (choice == 1) {
            string id, name, field1, field2;

            cout << "Enter ID: ";
            getline(cin, id);
            cout << "Enter Name: ";
            getline(cin, name);

            if (type == "PATIENT") {
                cout << "Enter Age: ";
                getline(cin, field1);
                cout << "Enter Disease: ";
                getline(cin, field2);
            }
            else if (type == "DOCTOR") {
                cout << "Enter Specialization: ";
                getline(cin, field1);
                cout << "Enter Experience: ";
                getline(cin, field2);
            }

            command = type + "_ADD:" + id + "," + name + "," + field1 + "," + field2;
        }
        else if (choice == 2) {
            command = type + "_VIEW";
        }
        else if (choice == 3) {
            cout << "Enter ID or Name to search: ";
            getline(cin, input);
            command = type + "_SEARCH:" + input;
        }
        else if (choice == 4) {
            cout << "Enter ID to delete: ";
            getline(cin, input);
            command = type + "_DELETE:" + input;
        }
        else if (choice == 5) {
            uploadImage(sock);
            continue;
        }
        else if (choice == 6) {
            downloadImage(sock);
            continue;
        }
        else if (choice == 7) {
            return;
        }
        else {
            cout << "Invalid choice.\n";
            continue;
        }

        sendCommand(sock, command);
    }
}

int main() {
    // Initialize Windows Sockets API (WSA)
    WSADATA wsaData;
    //It's a structure (struct) defined in <winsock2.h>
    //It holds details about the Windows Sockets implementation (version, system status, etc.).
    //After calling WSAStartup(), this structure is filled with information.
    //You pass its address to WSAStartup() so Winsock can write into it.
    //Even if you don’t use the info inside it directly, it is still required.

	// declaration of WSAStartup function
	// int WSAStartup(WORD wVersionRequested, WSADATA* lpWSAData);
	// wVersionRequested: the version of Winsock to use (2.2 in this case)
	// lpWSAData: a pointer to a WSADATA structure that receives information about the Winsock implementation.
	// This function initializes the Winsock library and must be called before using any Winsock functions.
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    //if successful i.e. winsock is created then it returns 0
    // If WSAStartup fails, it returns a non-zero error code
    //error may be due to missing Winsock DLL,unsupported version mismatch, or other issues.
    {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    // Create a socket for listening
    //AF_INET is an address family  = ipv4, 
    //SOCK_STREAM is Socket Type = stream(TCP, connection based, reliable)
    //0 represents Protocol = Default for this type (IPPROTO_TCP)
    //So, this line creates a TCP/IP socket that works over IPv4.
	//declaration of socket function
	// SOCKET socket(int af, int type, int protocol);
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    //if socket creation fails, it returns INVALID_SOCKET.
    {
        cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    // sockaddr_in is a structure that holds an Internet address
    serverAddr.sin_family = AF_INET;
	// AF_INET indicates that the address is an IPv4 address
    serverAddr.sin_port = htons(PORT);
	// htons converts the port number to network byte order (big-endian)

    if (InetPton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) 
		// InetPton converts a string representation of an IP address to a binary format
		// declaration of InetPton function
		// int InetPton(int af, const char* src, void* dst);
		// af: address family (AF_INET for IPv4)
		// src: source string (IP address)
		// dst: destination buffer to store the binary address
		// If InetPton fails, it returns <= 0
    {
        cerr << "Invalid IP address.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		// declaration of connect function
		// int connect(SOCKET s, const sockaddr* name, int namelen);
		// s: the socket to connect
		// name: a pointer to a sockaddr structure that contains the address to connect to
		// namelen: the length of the sockaddr structure
		// connect() attempts to establish a connection to the server
    {
        cerr << "Connection failed.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    cout << "Connected to Hospital Management Server.\n";

    int mainChoice;
    while (true) {
        cout << "\n--- Main Menu ---\n";
        cout << "1. Patient Management\n";
        cout << "2. Doctor Management\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";
        cin >> mainChoice;
        cin.ignore();

        if (mainChoice == 1) {
            showCrudMenu(sock, "PATIENT");
        }
        else if (mainChoice == 2) {
            showCrudMenu(sock, "DOCTOR");
        }
        else if (mainChoice == 3) {
            cout << "Exiting...\n";
            send(sock, "EXIT", 4, 0);
            break;
        }
        else {
            cout << "Invalid choice.\n";
        }
    }

	closesocket(sock);//release resoursces allocated for the socket
	WSACleanup();// Clean up Winsock resources, shutting down the Winsock library
    return 0;
}
