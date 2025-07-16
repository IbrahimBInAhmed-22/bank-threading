#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

class ATMClient {
    private:
    boost::asio::io_context io_context;
    unique_ptr<tcp::socket> socket;
    string current_user = "";
    bool is_authenticated = false;

    bool send_message(const string& message) {
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

    string receive_message() {
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
        cout << "----Authentication----" << endl;
        string card_number, pin;
        cout << "Enter Card Number: ";
        getline(cin, card_number);
        cout << "Enter PIN: ";
        getline(cin, pin);

        string request = "AUTH:" + card_number + ":" + pin + "\n";   
        if(!send_message(request)) {
            cerr << "Failed to send authentication request." << endl;
            return false;
        
        }
        
        string response = receive_message();
        // cout << "Rsponse: " << response << endl;
        if((response.substr(0, 13 ) == "AUTH_SUCCESS:")) {
            is_authenticated = true;
            current_user = response.substr(13);
            return true;
        }
        else {
            cout << "Authentication failed: " << response << endl;
            return false;
        }
    }
public:
    ATMClient():socket(make_unique<tcp::socket>(io_context)){}

    bool connect_to_Bank(const string& host, int port) {
        try{
            tcp::resolver resolver(io_context);
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
    void start_session()
    {
        cout << "Welcome to the ATM client!" << endl;
        while(!is_authenticated)
        {
            if(!authenticate())
            {
                cout << "Authentication failed. Please try again." << endl;
                continue;
            }
            else{
                cout << " Authentication successful! Welcome, " << current_user << "!" << endl;

                // commmented out for testing the program, for infinite looop
                is_authenticated = false;
                continue;
            }

        }
        
    }

};

int main() {
    try{
        ATMClient client;
        if(!client.connect_to_Bank("localhost", 8080)) {
            cerr << "Failed to connect to the bank server." << endl;
            return 1;
        }
        client.start_session();
    }
    catch(const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}