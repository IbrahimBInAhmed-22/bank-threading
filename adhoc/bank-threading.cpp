#include<iostream>
#include<memory>
#include<vector>
#include<string>
#include<pthread.h>
#include<unistd.h>

using namespace std;

class BankAccount{
    int balance;
    pthread_mutex_t mutex;
    public:
    BankAccount(int amount = 1000) : balance(amount)
    {
        pthread_mutex_init(&mutex, nullptr);
    }

    bool deposit(int amount)
    {
        if(amount <= 0)
        {
            cerr << "Deposit amount should be greater than 0." << "Amount: " << amount << endl;
            return false;
        }
        pthread_t threadId = pthread_self();

        pthread_mutex_lock(&mutex);
        usleep(5000);
        cout << "Depositing amount: " << amount << endl;
        balance += amount;
        cout << "Thread ID: " << threadId << "Updated balance after deposit is: " << balance << endl;
        pthread_mutex_unlock(&mutex);

        return true;
    }
    bool withDraw(int amount)
    {
        if (amount <= 0)
        {
            cout << "The amount should be than 0. " << endl;
            return false;
        }

        pthread_t threadId = pthread_self();

        pthread_mutex_lock(&mutex);
        cout << "withdrawing amount: " << amount << endl;
        if(amount >  balance)
        {
            cout << "Insufficeint balance. BALANCE: " << balance <<  endl;
            pthread_mutex_unlock(&mutex);
            return false;
        }
        usleep(5000);
        
        balance -= amount;
        cout << "Thread Id: " << threadId << " Updated balance after withdraw is: " << balance << endl;
        pthread_mutex_unlock(&mutex);

        return true;
    }
    int getBalance()
    {
        return balance;
    }
    ~BankAccount()
    {
        pthread_mutex_destroy(&mutex);
    }
};
enum TransactionType{
    WITHDRAW,
    DEPOSIT
};
struct ThreadData{
    int id;
    BankAccount* account;
    TransactionType type;
    int amount;
    ThreadData(int transaction_id, TransactionType typ, int am, BankAccount* acc) : id(transaction_id), type(typ), amount(am), account(acc){

    }
    
};
class ThreadManager{
public:
    static void* bankWorker( void* arg)
    {
        unique_ptr<ThreadData> acc (static_cast<ThreadData*>(arg));;
        cout << "ThreadId: " << pthread_self() << " started with transaction Id:" << acc->id << " Amount: " << acc->amount;

        if(acc->type == WITHDRAW)
            {
                cout <<" WITHDRAW" << endl;
                acc->account->withDraw(acc->amount);
            }
        else if (acc->type == DEPOSIT)
            {
                cout << " DEPOSIT" << endl;
                acc->account->deposit(acc->amount);
            }
        return nullptr;
    }
    void executeThread(BankAccount &account, vector<pair<int, TransactionType>>& transactions)
    {
        int n = transactions.size();
        vector<pthread_t> threads(n);
        for(int i = 0; i < n; i++)
        {
            unique_ptr<ThreadData> data (new ThreadData(i, transactions[i].second, transactions[i].first, &account));
            if(pthread_create(&threads[i], nullptr, bankWorker,data.release()) != 0)
            {
                cerr << "Couldn't create thread transactionId: " << i << endl;
            }
        }

        for (auto&t: threads)
        {
            pthread_join(t,nullptr);
        }

    }   

};

class DemoClass{
    BankAccount account;
    ThreadManager manager;
    public:
    DemoClass(int balance =1000):account(balance)
    {}
    void runDemo()
    {
        cout << " Initial balance: "<< account.getBalance() << endl;
        vector<pair<int, TransactionType>> transactions = {
        {200, WITHDRAW},
        {300, DEPOSIT},
        {400, WITHDRAW},
        {1000, WITHDRAW}};

        manager.executeThread(account, transactions);

        cout << " Amount after executing transcations is " << account.getBalance() << endl;
    }
};

int main()
{
    DemoClass().runDemo();
}