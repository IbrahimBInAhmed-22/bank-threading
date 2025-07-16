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
    boost::asio::io_context so_context;
    unique_ptr<tcp::acceptor> acceptor;

    int request_counter = 0;
    void handleClient(unique_ptr<tcp::socket> socket) {
        try {

            string authenticated_user = "";
            bool is_authenticated = false;
            while(true){
                boost::asio::streambuf buffer;
                boost::asio::read_until(*socket, buffer, "\n");

                istream is(&buffer);
                string request;
                getline(is, request);
                cout << "Received request: " << request << endl;
                string response;

                if(!is_authenticated){
                    response = handle_authentication(request, authenticated_user, is_authenticated);
                }
                else{
                   // response = handle_banking_request(request, authenticated_user);
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

    string handle_authentication(const string& request, string& authenticated_user, bool& is_authenticated) 
    {
        if(request.substr(0, 5) == "AUTH:") {
            size_t first_colon = request.find(':', 5);

            if(first_colon == string::npos ) {
                return "AUTH_FAILED:Invalid request format";
            }
           
                string card_number = request.substr(5, first_colon - 5);
                string pin = request.substr(first_colon+1);

                auto it = cardDatabase.find(card_number);
                if(it != cardDatabase.end() && (it->second.second == pin))
                {
                    authenticated_user = it->second.first;
//-------------------------------------------------------------uncommment for proper functionality--------------------------------------------//
                    // is_authenticated = true;
                    return "AUTH_SUCCESS:" + authenticated_user;    
                         
                } else {
                     return "AUTH_FAILED: Invalid card number or PIN";
                 }
cout << "Pin: " << pin << endl;
        }
       
            return "AUTH_FAILED; Unkown command";
        
        
    }
    //string handle_banking_request(const string& request, const string& authenticated_user) {}
public:
    BankServer(int port) {
        acceptor = make_unique<tcp::acceptor>(so_context, tcp::endpoint(tcp::v4(), port));
    }
    void start() {
        cout << "Bank server started, waiting for connections..." << endl;
        while(true){
            auto socket = make_unique<tcp::socket>(so_context);
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