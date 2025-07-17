#include<iostream>
#include<string>
#include<vector>
using namespace std;
class bank
{
public:
bank(string name){
    cout << "Bank name: " << name << endl;
}
};
int banks()
{
    bank("Meezan");
    bank("HBL");
    bank("Askari");
    return 0;
}
int middle()
{
    vector<string> banks = {"Meezan", "HBL", "Askari"};
    string receiveMessage;
    bool login;
    bool transaction;
}