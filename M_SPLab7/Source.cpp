#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <random>
#include <time.h>
using namespace std;

vector<queue<int>> customerQueues { 
    queue<int>( { 1, 2, 3, 4, 5 } ), 
    queue<int>( {6, 7, 8, 9 } ),
    queue<int>( {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20} )
};
mutex queueMutex;
condition_variable queueCV;
mutex printMutex;

class Trader
{
public:
    Trader(int id) : traderID(id) {}

    void serveCustomers()
    {
        while (true)
        {
            int customerID = 0;
            unique_lock<mutex> lock(queueMutex);

            queueCV.wait(lock, [&]() { return !customerQueues[traderID].empty() || checkOtherQueues(customerQueues); });

            if (!customerQueues[traderID].empty())
            {
                customerID = customerQueues[traderID].front();
                customerQueues[traderID].pop();

                // Обслуживание покупателя
                lock.unlock();
                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> dis(0, 2000);
                int num = 1000 + dis(gen);
                {
                    unique_lock<mutex> lockprint(printMutex);
                    cout << "Торговец " << traderID << " обслужил за " << num << "мс Покупателя " << customerID << endl;
                }             
                this_thread::sleep_for(chrono::milliseconds(num));
            }
            else
            {
                int longestQueueId = -1;
                int maxLength = 0;
                for (int i = 0; i < customerQueues.size(); i++)
                {
                    if (i != traderID && customerQueues[i].size() >= maxLength && !customerQueues[i].empty())
                    {
                        longestQueueId = i;
                        maxLength = customerQueues[i].size();
                    }
                }

                if (longestQueueId != -1)
                {
                    customerID = customerQueues[longestQueueId].front();
                    customerQueues[longestQueueId].pop();
                    lock.unlock();

                    // Обслуживание покупателя
                    random_device rd;
                    mt19937 gen(rd());
                    uniform_int_distribution<> dis(0, 2000);
                    int num = 1000 + dis(gen);
                    {
                        unique_lock<mutex> lockprint(printMutex);
                        cout << "Торговец " << traderID << " забрал у Торговца " << longestQueueId << " и обслужил за " << num << "мс Покупателя " << customerID << endl;
                    }
                    this_thread::sleep_for(std::chrono::milliseconds(num));
                }
                else
                {
                    lock.unlock();
                }
            }
        }
    }

private:
    int traderID;

    bool checkOtherQueues(const vector<queue<int>>& customerQueues)
    {
        for (int i = 0; i < customerQueues.size(); i++)
        {
            if (!customerQueues[i].empty())
            {
                return true;
            }
        }
        return false;
    }
};

int main()
{
    setlocale(0, "");

    vector<Trader> traders{ Trader(0), Trader(1), Trader(2) };

    vector<thread> traderThreads;
    for (int i = 0; i < traders.size(); i++)
    {
        traderThreads.push_back(thread(&Trader::serveCustomers, &traders[i]));
    }

    for (thread& traderThread : traderThreads)
    {
        traderThread.join();
    }

    return 0;
}