#include<memory>
#include<iostream>
#include<vector>
#include<string>
#include<boost/asio.hpp>
#include <unordered_map>
#include <unistd.h>

using namespace std;
using boost::asio::ip::tcp;

class BankServer {
    // cardNumber -> (cardHolderName, pin)
    private:
    unordered_map<string, pair<string, string>> cardDatabase = {
        {"1234567", {"John Doe", "1234"}},
        {"2345678", {"Jane Smith", "5678"}},
        {"3456789", {"Alice Brown", "4321"}},
        {"4567890", {"Bob White", "8765"}}
        };
    boost::asio::io_context ioContext;
    unique_ptr<tcp::acceptor> acceptor;

    int requestCounter = 0;
    void handleClient(unique_ptr<tcp::socket> socket) {
        try {

            string authenticatedUser = "";
            bool isAuthenticated = false;
            while(true){
                boost::asio::streambuf buffer;
                boost::asio::read_until(*socket, buffer, "\n");

                istream is(&buffer);
                string request;
                getline(is, request);
                cout << "Received request: " << request << endl;
                string response;

                if(!isAuthenticated){
                    response = handle_authentication(request, authenticatedUser, isAuthenticated);
                }
                else{
                   // response = handle_banking_request(request, authenticatedUser);
                   usleep(10000);
                   cout << "Disconnecting client after processing request." << endl;;
                }
                response += "\n";
                boost::asio::write(*socket, boost::asio::buffer(response));
            }
        }
        catch (const boost::system::system_error& e){
            cout << "Client disconnected: " << e.what() << endl;
        }
        }

    string handle_authentication(const string& request, string& authenticatedUser, bool& isAuthenticated) 
    {
        if(request.substr(0, 5) == "AUTH:") {
            size_t firstColon = request.find(':', 5);

            if(firstColon == string::npos ) {
                return "AUTH_FAILED:Invalid request format";
            }
           
                string card_number = request.substr(5, firstColon-);
                string pin = request.substr(firstColon+1);
                cout << "Received authentication request for card number: " << card_number << endl;
                auto it = cardDatabase.find(card_number);
                if(it != cardDatabase.end() && (it->second.second == pin))
                {
                    authenticatedUser = it->second.first;
//-------------------------------------------------------------uncommment for proper functionality--------------------------------------------//
                    // isAuthenticated = true;
                    return "AUTH_SUCCESS:" + authenticatedUser;    
                         
                } else {
                     return "AUTH_FAILED: Invalid card number or PIN";
                 }
        }
       
            return "AUTH_FAILED; Unkown command";
        
        
    }
    //string handle_banking_request(const string& request, const string& authenticatedUser) {}
public:
    BankServer(int port) {
        acceptor = make_unique<tcp::acceptor>(ioContext, tcp::endpoint(tcp::v4(), port));
    }
    void start() {
        cout << "Bank server started, waiting for connections..." << endl;
        while(true){
            auto socket = make_unique<tcp::socket>(ioContext);
            acceptor->accept(*socket);
            cout << "ATM connected from " << socket->remote_endpoint().address().to_string() << ":" 
            << socket->remote_endpoint().port() << endl;
            handleClient(move(socket));}
    }
};  

int main() {
    try{
        BankServer server(8080);
        server.start();
    }
    catch(const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}