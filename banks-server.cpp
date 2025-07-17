#include<iostream>
#include<string>
#include<boost/asio.hpp>
#include<unordered_map>
#include <memory>

using namespace std;
using boost::asio::ip::tcp;


class BankServer{

    boost::asio::io_context ioContext;
    unique_ptr<tcp::acceptor> acceptor;
    unordered_map<string, pair<pair<string, string>, int>> cardDatabase = {
        {"0", {{"John Doe", "1234"}, 0}},
        {"2345678", {{"Jane Smith", "5678"}, 90}},
        {"3456789", {{"Alice Brown", "4321"}, 100}},
        {"4567890", {{"Bob White", "8765"}, 200}}
        };

    string loginHandle(string request)
    {
        try{
            size_t secondColon = request.find(":", 5);
            if(secondColon == string::npos)
            {
                return "AUTH_FAILED: Invalid format";
            }
            string cardNo = request.substr(5, secondColon - 5);
            string pin = request.substr(secondColon+1);
            cout << "Received pin:" << pin;
            cout << "CardNo: "<< cardNo << ">" << endl;
            cout << "Request: " << request << endl;
            
            auto it = cardDatabase.find(cardNo);
            if(it != cardDatabase.end())
            {
            cout << "Card found \n";
            cout << "Stored PIN: " << it->second.first.second << endl;
            }
            if(it != cardDatabase.end() && it->second.first.second == pin)
            {
                return "AUTH_SUCCESS:"+it->second.first.first;
            }
            else{
                return "AUTH_FAILED: Invalid card number or PIN ";
            }
            return "AUTH_FAILED; Unkown command";
        }
        catch(const boost::system::system_error& e)
        {
            cerr << "There was an error while authenticating " << endl;
            return "Authentication Failed at bank server.";
        }
    }
    string deposit(string data)
    {
        try{
        size_t secondColon = data.find(":");
        string cardNo = data.substr(0, secondColon) ;
        string amount = data.substr(secondColon+1);

        auto it = cardDatabase.find(cardNo);
        if(it != cardDatabase.end() )
        {
            int amnt = stoi(amount);
            if(amnt < 0)
            {
                return "TNX_FAILED: Deposit amount must be positive";
            }
            it->second.second += stoi(amount);
            return "TXN_SUCCESS: deposited " + amount + " CurrentBalance: " + to_string(it->second.second);
        }

            return "TXN_FAILED: Invalid card number or PIN ";
        
        }
        catch(const boost::system::system_error& e)
        {

            cerr << "There was an error while making transaction " << endl;
            return "TXN_FAILED: at bank server.";
        }
    }
    string withdraw(string data)
    {
        try{
        size_t secondColon = data.find(":");
        string cardNo = data.substr(0, secondColon) ;
        string amount = data.substr(secondColon+1);

        auto it = cardDatabase.find(cardNo);
        if(it != cardDatabase.end() )
        {
            int amnt = stoi(amount);
            if (amnt <= 0)
            {
                return "TNX_FAILED: Amount must be positive.";
            }
            if((it->second.second) < amnt)
            {
                return "TXN_FAILED: INSUFFICIENT amount. Current balance: " + to_string(it->second.second);
            }
            it->second.second -= stoi(amount);
            return "TXN_SUCCESS: withdrew " + amount + " CurrentBalance:" + to_string(it->second.second);
        }

            return "TXN_FAILED: Invalid card number or PIN ";
        
        }
        catch(const boost::system::system_error& e)
        {

            cerr << "There was an error while making transaction " << endl;
            return "TXN_FAILED: at bank server.";
        }

    }
    string transactionHandle(string request)
    {
        try{
            cout << "Request in transaction handle: "<< request << endl;
            cout << request.substr(0,12) << endl;
        if(request.substr(0,12)=="TXN:WITHDRAW")
        {
            string data = request.substr(13);
            cout <<"DATA in transactionhandle: " << data << endl;
            return withdraw(data);

        }
        else if(request.substr(0,11) == "TXN:DEPOSIT")
        {
            string data = request.substr(12);
            return deposit(data);
        }
        return "TXN_FAILED: at bank server (unknown command).";
        }
        catch(const boost::system::system_error& e)
        {
            cerr << "There was an error while executing transaction." << endl;
            return "TXN_FAILED: at bank server.";
        }
    }
    void handleClient(unique_ptr<tcp::socket> socket)
    {
        try{
            while(true)
            {
                boost::asio::streambuf buffer;
                boost::asio::read_until(*socket, buffer, "\n");

                istream is(&buffer);
                string request;
                getline(is, request);
                string response;
                cout << "Received request: " << request << endl;
                if(request.substr(0, 4)=="AUTH")
                response = loginHandle(request);
                else
                response =  transactionHandle(request);
                response += "\n";
                cout << "Response inside bank: " << response << endl;
                boost::asio::write(*socket, boost::asio::buffer(response));
            }
        }
        catch(const boost::system::system_error & e)
        {
            cerr << "Client disconnected, " << e.what() << endl;
        }
    }
public:
    BankServer(int port)
    {
        acceptor = make_unique<tcp::acceptor>(ioContext, tcp::endpoint(tcp::v4(), port));
    }

    void start()
    {
        cout << "Bank server started, waiting for connections..." << endl;
        while(true)
        {
            auto socket = make_unique<tcp::socket>(ioContext);
            acceptor->accept(*socket);
            cout << "Switch connected from " << socket->remote_endpoint().address().to_string() << ":" << 
            socket->remote_endpoint().port() << endl;
            handleClient(move(socket));
        }
    }

    
};

int main()
{
    BankServer meezan(8090 + 0);
    BankServer hbl(8090 + 1);
    BankServer askari(8090 + 2);

    meezan.start();
    hbl.start();
    askari.start();
}