# 🏥 Hospital Management System – Client-Server Project
This project is a Client-Server Hospital Management System implemented using C++ and Winsock. It demonstrates TCP socket programming, multithreading, and file-based CRUD operations for managing Patients and Doctors data.

## 📌 Features
### ✅ Server
- Handles multiple clients using multithreading
- Processes commands from clients to perform:
      - Add, View, Search, and Delete operations on patient.txt and doctor.txt
- Ensures thread-safe access to shared files

### ✅ Client
- Provides a menu-based interface
- Sends commands over TCP to the server
- Displays server responses (success/failure or results)

## 🧠 Learning Outcomes
- Socket programming in Windows using Winsock
- Multithreaded server design
- Designing command protocols for communication
- File handling and parsing in C++
- Real-time communication between multiple clients and a server

## 🖥️ Technologies Used
- C++ (Visual Studio 2022)
- Winsock2 for TCP socket communication
- Multithreading (std::thread)
- File I/O (fstream)

## 🛠️How It Works
 ### Server
- Listens for incoming client connections on port 8080
- For each new client, spawns a new thread
- Processes commands such as:
     - PATIENT_ADD:<id,name,age,disease>
     - DOCTOR_SEARCH:<name or id>
     - DOCTOR_DELETE:<id>
     - And others...
- Stores all data in text files (patient.txt, doctor.txt)
### Client
- Connects to server using 127.0.0.1:8080
- Offers menus for:
     - Patient Management
    - Doctor Management
- Sends user commands to the server and displays the response

## 🚀 How to Run
1. Compile the Server
2. Compile the Client
3. Run the Server First
   
![image](https://github.com/user-attachments/assets/023b3d34-30cc-4683-be63-cb00dbdc4666)
4. Then Start One or More Clients

![image](https://github.com/user-attachments/assets/6fef028d-d770-4aff-a885-c1e63e9c65f6)

![image](https://github.com/user-attachments/assets/dd95f662-5066-4a64-81aa-9da60407216f)

   
## 📸 Screenshots
👨‍⚕️ Add Patient from Client1

![image](https://github.com/user-attachments/assets/e6003759-d89c-42d2-8c21-33c48a421d3d)

![image](https://github.com/user-attachments/assets/549a67c4-9f80-41d7-bc99-3005a89554d6)


👩‍⚕️ View Patients from Client2

![image](https://github.com/user-attachments/assets/0e8d3d89-937c-41af-9fcf-230c5e6f26b4)

![image](https://github.com/user-attachments/assets/a8c04aed-eac3-4b87-a1e9-48e9b1768ee7)


❌ Delete Doctor from Client1

![image](https://github.com/user-attachments/assets/9e389c1b-5363-4b06-b75d-9c3e864c7552)

![image](https://github.com/user-attachments/assets/17a3b45f-60fb-4ff1-be84-06439dc8e5c9)


🔍 Search by ID from Client2

![image](https://github.com/user-attachments/assets/25a8f689-f250-4c63-b6ca-f7f5771c6871)

![image](https://github.com/user-attachments/assets/a636fde9-6a30-4c57-af69-faf6991e76c2)


🔁 Closing client1 then client2 console window

![image](https://github.com/user-attachments/assets/c8e7b4d4-c25b-4b16-92fa-3e932f5f5fc0)


## 📁 Data Files
- patient.txt – stores patient records in CSV format
- doctor.txt – stores doctor records in CSV format

## Author
Laiba
