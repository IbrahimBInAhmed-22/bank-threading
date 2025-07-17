#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

class AtmClient {
    private:
    boost::asio::io_context ioContext;
    unique_ptr<tcp::socket> socket;
    string currentUser = "";
    bool isAuthenticated = false;

    bool sendMessage(const string& message) {
        try{
        string request = message + "\n";
        boost::asio::write(*socket, boost::asio::buffer(request));
        return true;
    }
        catch(const boost::system::system_error& e) {
            cerr << "Error sending message: " << e.what() << endl;
            return false;
        }


    }

    string receiveMessage() {
        try {
            boost::asio::streambuf buffer;
            boost::asio::read_until(*socket, buffer, "\n");

            istream is(&buffer);
            string response;
            getline(is, response);
            return response;
        }
        catch(const boost::system::system_error& e) {
            cerr << "Error receiving message: " << e.what() << endl;
            return "";
        }
    }
    bool authenticate()
    {
        int n;
        cout << "----Authentication----" << endl;
        string cardNumber ="0000111122223333", pin = "1234";
        cout << "Enter to continue " << endl;
        cin >> n;
        // string cardNumber , pin;
        // cout << "Enter Card Number: ";
        // getline(cin, cardNumber);
        // cout << "Enter PIN: ";
        // getline(cin, pin);

        string request = "AUTH:" + cardNumber + ":" + pin + "\n";   
        if(!sendMessage(request)) {
            cerr << "Failed to send authentication request." << endl;
            return false;
        
        }
        
        string response = receiveMessage();
        // cout << "Rsponse: " << response << endl;
        if((response.substr(0, 13 ) == "AUTH_SUCCESS:")) {
            isAuthenticated = true;
            currentUser = response.substr(13);
            return true;
        }
        else {
            cout << "Authentication failed: " << response << endl;
            return false;
        }
    }
public:
    AtmClient():socket(make_unique<tcp::socket>(ioContext)){}

    bool connect_to_Bank(const string& host, int port) {
        try{
            tcp::resolver resolver(ioContext);
            auto endpoints = resolver.resolve(host, to_string(port));
             boost::asio::connect(*socket, endpoints);

            cout << "Connected to bank server at " << host << ":" << port << endl;
            return true;
        }
        catch(const exception& e)
        {
            cerr << e.what() << "\n";
            return false;
        }
    }
    void startSession()
    {
        cout << "Welcome to the ATM client!" << endl;
        while(!isAuthenticated)
        {
            if(!authenticate())
            {
                cout << "Authentication failed. Please try again." << endl;
                continue;
            }
            else{
                cout << " Authentication successful! Welcome, " << currentUser << "!" << endl;

                // commmented out for testing the program, for infinite looop
                isAuthenticated = false;
                continue;
            }

        }
        
    }

};

int main() {
    try{
        AtmClient client;
        if(!client.connect_to_Bank("localhost", 8080)) {
            cerr << "Failed to connect to the bank server." << endl;
            return 1;
        }
        client.startSession();
    }
    catch(const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}