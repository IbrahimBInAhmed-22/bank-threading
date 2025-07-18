#include <iostream>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include <memory>
#include <pthread.h>
#include <queue>
using namespace std;
using boost::asio::ip::tcp;


enum BankList{
    MEEZAN,
    HBL,
    ASKARI
};
class BankAccount {
    string name;
    string cardNo;
    string pin;
    pthread_mutex_t mutex;
public:
    BankAccount(string n, string c, string p) : name(n), cardNo(c), pin(p) 
        {
        pthread_mutex_init(&mutex, nullptr);    
    }
    bool deposit(int amount)
    {
        if(amount <= 0)
        {
            cerr << "Deposit amount should be greater than 0." << endl;
            return false;
        }

        pthread_mutex_lock(&mutex);
        cout << "Deposting amount: " << amount << endl;
        // Simulate deposit operation
        // show updated balance
        pthread_mutex_unlock(&mutex);
        usleep(5000);
        return true;
    }

    bool withdraw(int amount)
    {
        if (amount <= 0 ){
            cerr << "the amount for withdrawal should be greater than 0." << endl;
            return false;
        }
        pthread_mutex_lock(&mutex);
        // getbalance from the server 
        // should be greater than amount
        // simulate withdrawal operation
        // show updated balance
        pthread_mutex_unlock(&mutex);
        usleep(5000);
        return true;

    }
    
    ~BankAccount()
        {
        pthread_mutex_destroy(&mutex);
        }   

};




class middleLayer{

private:
    boost::asio::io_context ioContext;
    unique_ptr<tcp::socket> layerSocket;
    unique_ptr<tcp::acceptor> acceptor;
    BankList bank;
    
    int limit = 5;
    queue<string> queries;
    int currentQueries;


        // 8090 meezan, 8091 HBL, 8092 Askari
    bool connectToBankServer(string host, int port) {
        try {
            layerSocket = make_unique<tcp::socket>(ioContext);
            tcp::resolver resolver(ioContext);
            auto endpoints = resolver.resolve((host, to_string(port)));
            boost::asio::connect(*layerSocket, endpoints);
            cout << "Connected to bank server at " << host << ":" << port << endl;
            return true;
        }
        catch(const exception & e) {
            cerr << "Error connecting to bank server: " << e.what() << endl;
            return false;
        }
    }
    // 0000 for Meezan, 0001 for HBL, 0002 for Askari
    bool bankAuthentication(string cardNo)
    {
        int bankindex = stoi(cardNo.substr(0,4));

        switch(bankindex) {
            case MEEZAN:
                cout << "Authenticating with Meezan Bank" << endl;
                bank = MEEZAN;
                return connectToBankServer("localhost", 8090+bankindex);
                break;
            case HBL:
                cout << "Authenticating with HBL" << endl;
                bank = HBL;
                return connectToBankServer("localhost", 8090+bankindex);
                break;
            case ASKARI:
                cout << "Authenticating with Askari Bank" << endl;
                bank =  ASKARI;
                return connectToBankServer("localhost", 8090+bankindex);
                break;
            default:
                cerr << "Unknown bank index: " << bankindex << endl;
                return false;
        }
    }

    bool sendMessageToBank(const string& message) 
    {
        try{
            string request = message + "\n";
            boost::asio::write(*layerSocket, boost::asio::buffer(request));
            return true;
        }
        catch(const boost::system::system_error& e) {
            cerr << "Error sending message from middle layer to bank server: " << e.what() << endl;
            return false;
        }   
    }

    string receiveMessageFromBank() 
    {
        try {
            boost::asio::streambuf buffer;
            read_until(*layerSocket, buffer, "\n");
            istream is(&buffer);
            string response;
            getline(is, response);
            return response;

        }
        catch(const boost::system::system_error& e) {
            cerr << "Error receiving message from bank server: " << e.what() << endl;
            return "";
        }
    }
public:
    middleLayer(int port):  layerSocket(make_unique<tcp::socket>(ioContext)), 
                            acceptor(make_unique<tcp::acceptor>(ioContext, tcp::endpoint(tcp::v4(), port))),
                            currentQueries(0) 
        {
        cout << "Middle layer initialized on port: " << port << endl;
        }

    string handleLogin(const string& request, BankAccount* acc, bool &isAuthenticated ) // No lock required as data is read (no race-condition)
    {
        try
        {
        if(request.substr(0, 4) == "AUTH")
        {
            size_t secondColon = request.find(":", 5);
            if(secondColon != string::npos) 
        {
                string cardNo = request.substr(5, secondColon -5);
                string pin = request.substr(secondColon + 1);
                cout << "Received authentication request for card number: " << cardNo << endl << "PIN:" << pin<<endl;
                if(!bankAuthentication(cardNo)) {
                    return "AUTH_FAILED: Unkown bank";
                }

            queries.push(request);  
            currentQueries++;

            if(currentQueries > limit)
            cout << "query limit reached waiting for response..." << endl;
            bool printed = false;
            do
            {
                if(currentQueries < limit)
                if(sendMessageToBank(request)) 
                {
                    string response = receiveMessageFromBank();
                    cout << "Response: " << response << endl;
                    currentQueries--;
                    return response;
                }
                if(!printed)
                cout << "Limit reached wait for some seconds to login."<< endl;
                printed = true;
             
            }while(currentQueries > limit);
        
        }
      
    
    }
          cout << "AUTH_FAILED: Invalid format" << endl;
            return "AUTH_FAILED: Invalid format";
    }
    catch(const boost::system::system_error &e)
    {
        cerr << "Error ocurred while authenticating." << e.what();
        return "AUTH_FAILED: Invalid format";
    }
}
    string handleTransaction(const string& request, BankAccount* acc) // lock required as data manipulation expected (race-condition)
    {
    try{
        if(request.substr(0, 3) == "TXN")
        {
            size_t secondColon = request.find(':', 4);
            if (secondColon != string::npos){
                string cardNo = request.substr(4, secondColon - 4);

                queries.push(request);
                currentQueries++;
                
                bool printed = false;
                do
                {
                   
                    if(currentQueries < limit)
                    if(sendMessageToBank(request))
                    {
                        string response = receiveMessageFromBank();
                        currentQueries--;
                        cout << "Response: " << response << endl;
                        
                        return response;
                    }               
                    if(!printed)
                    cout << "Limit reached wait for some seconds for you transaction."<< endl;
                    printed = true;
                }while(currentQueries > limit);

            }
        }

        {
            cerr << "TXN_FAILED2: Invalid format.";
            return "TXN_FAILED1: Invalid format.";
        }
    }
    catch(const boost::system::system_error& e)
    {
        cerr << "Error while performing transaction. Error: " << e.what();
        return "";
    }
}

    void handleClient(unique_ptr<tcp::socket> socket)
    {
        try{
            BankAccount *account;
            bool isAuthenticated = false;
            while(true) {

                boost::asio::streambuf buffer;
                boost::asio::read_until(*socket, buffer, "\n");
                istream is(&buffer);
                string request;
                getline(is, request);
                cout << " Received request: " << request << endl;

                string response;
                if(request.substr(0,4)=="AUTH") 
                {
                    response =  handleLogin(request, account, isAuthenticated);
                }
                else if(request.substr(0, 3) == "TXN")
                {
                    response = handleTransaction(request, account);
                }
                else
                {
                    response = "Unknown command.";
                }
                response += "\n";
                boost::asio::write(*socket, boost::asio::buffer(response));

            }

        }
        catch(const boost::system::system_error& e) {
            cerr << "Error handling client: " << e.what() << endl;
        }
    }
    void start()
    {
        cout << "Middle Layer activated, waiting for connections..." << endl;
        while(true) {
            auto socket = make_unique<tcp::socket>(ioContext);
            acceptor->accept(*socket);
            cout << "Connection established with ATM at " << socket->remote_endpoint().address().to_string() << ":" 
            << socket->remote_endpoint().port() << endl;    
            handleClient(move(socket));        
        }
    }







};


enum transactionType{
    WITHDRAW,
    DEPOSIT
};

struct loginThreadData{
    string cardNo;
    string pin;
    loginThreadData(string c, string p) : cardNo(c), pin(p)
    {}

};

struct transactionThreadData{

    int amount;
    loginThreadData*account;
    transactionType type;
    transactionThreadData(loginThreadData* acc, int a) : account(acc), amount(a) {}
};


class threadManager{
    public:
    void * loginWorker(void *arg)
    {
        unique_ptr<loginThreadData> acc(static_cast<loginThreadData*>(arg));
        cout << "ThreadID: " << pthread_self() << "Verifying user cardNo: " << acc->cardNo << " with pin \""<< acc->pin<<"\"" << endl;
        //call to bank server for verification
        //if verification successful, set isAuthenticated to true
        //else set isAuthenticated to false
        //(success): you will receive the name of user which is to be saved and the whole struct needs to be returned to caller
        return nullptr;
    }
    void *transactionWorker(void *arg)
    {
        unique_ptr<transactionThreadData> acc(static_cast<transactionThreadData*>(arg));
        cout << "ThreadID: " << pthread_self() << "Processing transaction for user: "  << " with amount: " << acc->amount << endl;
        //initiate lock for transaction 
        //call to bank server for transaction
        //if transaction successful, return true
        //else return false
        return nullptr;
    }
};

int main()
{
    middleLayer Swittch(8080);
    Swittch.start();
    return 0;
}