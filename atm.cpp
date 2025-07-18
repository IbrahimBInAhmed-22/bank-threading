#include<iostream>
#include<memory>
#include<string>
#include<boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

class Atm{
    bool isAuthenticated = false;
    string cardNumber;
    string currentUser;
    boost::asio::io_context ioContext;
    unique_ptr<tcp::socket> socket;

    string authenticate()
    {
        // cout << "----Authentication----" << endl;
        // string cardNo, pin;
        // do{
        // cout << "Enter card number "<< endl;
        // cin >> cardNo;
        // if(cardNo.length() < 12)
        // {cout << "The cards number should contain 12 digits" << endl;}
        // }while(cardNo.length() < 12)
        // cout << "Enter PIN: " << endl;
        // cin >> pin;
            
        string n;
            cout << "\n\n----Authentication----" << endl;
            string cardNo ="0", pin = "1234";
            cout << "Enter to continue " << endl;
            getline(cin, n);


        string request = "AUTH:"+cardNo+":"+pin;

        if(!sendMessage(request))
        {
            cerr << "Failed to send authentication request. " << endl;

            isAuthenticated = false;
            currentUser = "";
            cardNumber = "";
            return "AUTH_FAILED: Failed to send authentication request.";
        }
        string response = receiveMessage();
        if(response.substr(0,12) == "AUTH_SUCCESS")
        {

            isAuthenticated = true;
            size_t secondColon = response.find(":");

            currentUser = response.substr(secondColon+1);
            //cout << "curretUser: " << currentUser<<endl;
            cardNumber = cardNo;

            return "Authentication successful. Welcome! " + currentUser + ".";
        }
        else 
        isAuthenticated = false;
        currentUser = "";
        cardNumber = "";
        return response;
    }
    string receiveMessage() 
    {
    try{
        boost::asio::streambuf buffer;
        boost::asio::read_until(*socket, buffer, "\n");

        istream is(&buffer);
        string response;
        getline(is, response);
        return response;

    }
    catch(const boost::system::system_error& e)
    {
        cerr << "Error receiving message " << e.what();
        return "";
    }
    }
    bool sendMessage(const string& message) 
    {
        try{
            string request = message +"\n";
            boost::asio::write(*socket, boost::asio::buffer(request));
            return true;
        }
        catch(const boost::system::system_error& e)
        {
            cerr << "Error sending message " << e.what();
            return false;
        }
    }   
    void transaction()
    {
        try{
        string transactionType, amount;
        string n;
        cout << "\nPRESS\n 1 for withdrawal \n 2 for deposit. \n";

        bool inValid = false;
        do{
            inValid = false;
        getline(cin, n);
        cout << "Choice: " << n << endl;
        if(n == "1")
        transactionType = "WITHDRAW";
        else if ( n == "2")
        transactionType = "DEPOSIT";
        else 
        {
            cout << " Invalid choice. Enter a valid choice. "<< endl;
            inValid = true;}
        }while(inValid);
        int amnt = 0;
        do{
        cout << " Enter amount: " << endl;
        getline(cin,amount);
        amnt = stoi(amount);
        if(amnt < 0)
        cout << "Amount should be greater than 0. Enter valid amount. " << endl;
        }while(amnt <= 0);

        string request = "TXN:" + transactionType + ":" + cardNumber + ":" + amount;
        int tries = 0;
        while(!sendMessage(request) && tries < 3)
        {
            cout << "Error sending message. Trying again..." << endl;
            tries++;
        }
        if(tries >= 3)
        {
            cout << "Transaction failed after multiple attempt "<< endl;
            return;
        }
        cout << "Request sent successfully." << endl;
        string response = receiveMessage();
        cout << "Response: " << response;
    }
    catch(const boost::system::system_error& e)
    {Response:
        cout << "Error while executing transaction. Error " << e.what() << endl;
        return;
    }
    }
public:
    Atm():socket(make_unique<tcp::socket>(ioContext)){}
    bool connectTOBank(const string& host, int port)
    {
    try{
    tcp::resolver resolver(ioContext);
    auto endPoints = resolver.resolve(host,to_string(port) );
    boost::asio::connect(*socket, endPoints);
    cout << "Connected to bank server at" << host << ":" << port << endl;
    return true;
    }
    catch(const boost::system::system_error& e)
    {
        cerr << "Error while connecting to bank." << endl;
        return false;
    }

    }



    void start()
{
    cout << "\n\nWelcome to the ATM client!" << endl;

    while(true)
    {
        string response = authenticate();
        if(!isAuthenticated)
        {cout << "Authentication failed. Please try again. " << endl;}
        if(isAuthenticated)
        {
            cout << response << endl;
            transaction();
        }
    }



}
};

int main()
{
    try{
    Atm atm;
    if(!atm.connectTOBank("localhost",8080))
    {
        cerr << "Failed to connect to the bank server. " << endl;
        return -1;
    }
    atm.start();
    }
    catch(const boost::system::system_error& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1; 
    }
    return 0;

}