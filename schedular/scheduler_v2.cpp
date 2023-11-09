// a1768127, Shue Hong, Lee
// a1761407, Kwan Han, Li
// a1745070, Archie, Verma
// OCTOPUS
/*
created by Andrey Kan
andrey.kan@adelaide.edu.au
2021
*/

#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <algorithm>
#include <stdlib.h>

using namespace std;

const int PRINT_LOG = 0; // print detailed execution trace
int count_customers = 0;
const int promotion_interval = 10;
const int main_time_allowance = 1;

class Customer
{
public:
    string name;
    int customer_id;
    int priority;
    int arrival_time;
    int burst_time; // slots requested
    int wait_time;
    float ratio;
    int playing_since;
    int slots_remaining;

    Customer(string par_name, int par_customer_id, int par_priority, int par_arrival_time, int par_slots_requested)
    {
        name = par_name;
        customer_id = par_customer_id;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_requested;
        burst_time = par_slots_requested;
        wait_time = -1;
        ratio = -1;
        playing_since = -1;
    }
};


void initialize_system(
    ifstream &in_file,
    deque<Customer> &customers)
{
    string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        Customer customer_from_file(name, customer_id, priority, arrival_time, slots_requested);
        customers.push_back(customer_from_file);
        customer_id++;
        count_customers++;
    }
}

void print_state(
    ofstream &out_file,
    int current_time,
    int current_id)
    //const deque<Customer> &customers,
    //const deque<int> &customer_queue)
{
    /************If (PRINT_LOG != 0), the program will not output .txt file*************/
    
    out_file << current_time << " " << current_id << '\n';
    if (PRINT_LOG == 0)
    {
        return;
    }
    
    /***********************************************************************************/

    /*
    cout << current_time << ", " << current_id << '\n';
    for (int i = 0; i < customers.size(); i++)
    {
        cout << "\t" << customers[i].priority << ", " << customers[i].customer_id << ", ";
    }
    cout << '\n';
    for (int i = 0; i < customer_queue.size(); i++)
    {
        cout << "\t" << customer_queue[i] << ", ";
    }
    cout << '\n';
    */
}


void hrrn(deque<Customer> &customers, int current)
{
    // loop through customer queue and find the highest response ratio
    float maxi = -1;
    for(int i=0; i<customers.size(); i++)
    {
        // calculate current response ratio
        customers[i].wait_time = current - customers[i].arrival_time;
        customers[i].ratio = (customers[i].wait_time + customers[i].burst_time) / customers[i].burst_time;

        // keep track of max value
        if(customers[i].ratio > maxi)
        {
            maxi = customers[i].ratio;
        }
    }

    Customer *cmp = nullptr;
    int index = -1;
    for(int i=0; i<customers.size(); i++)
    {
        // find the customer(s) with the highest response ratio 
        if(customers[i].ratio == maxi)
        {
            if(cmp == nullptr)
            {
                cmp = &customers[i];
                index = i;
            }
            // calculate shortest job if more than one customers with same ratio
            else if (customers[i].burst_time < cmp->burst_time)
            {
                cmp = &customers[i];
                index = i;
            }
        }
    }   

    swap(customers[0], customers[index]);
}


// process command line arguments
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Provide input and output file names." << endl;
        return -1;
    }
    ifstream in_file(argv[1]);
    ofstream out_file(argv[2]);
    if ((!in_file) || (!out_file))
    {
        cerr << "Cannot open one of the files." << endl;
        return -1;
    }

    deque<Customer> customers; // information about each customer
    // read information from file, initialize customers queue
    initialize_system(in_file, customers);

    int current_id = -1;

    //Seperates Customers into two queues based on their priority
    deque<Customer> P0; // high priority
    deque<Customer> P1; // low priority
    deque<Customer> main_queue; //All customers

    // step by step simulation of each time slot
    bool all_done = false;
    int time_out = -1;


    for (int current_time = 0; !all_done; current_time++)
    {
        while(!customers.empty() && current_time == customers[0].arrival_time)
        {   
            main_queue.push_back(customers[0]);
            customers.pop_front();
        }

        // while(count < main_time_allowance)
        // {
            
        //     count++;
        // }
        // customers[0].slots_remaining = customers[0].burst_time - main_time_allowance;
        // if(customers[0].slots_remaining > 0)
        // {
        //     if(customers[0].priority == 0)
        //     {
        //         P0.push_back(customers[0]);
                
        //     }
        //     else if(customers[0].priority == 1)
        //     {
        //         P1.push_back(customers[0]);
        //         customers.pop_front();
        //     }
        // }
        // else if(customers[0].slots_remaining <=0)
        // {
        //     customers.pop_front();
        // }

        if(current_id >= 0)
        {
            if(current_time == time_out)
            {
                current_id = -1;
            }
        }

        if (current_id == -1)
        {
            // calculate the response ratio for P0 and P1
            if(!P0.empty())
            {
                hrrn(P0, current_time);
            }
            if(!P1.empty())
            {
                hrrn(P1, current_time);
            }

            // check for P1 promotion
            if(!P1.empty() && P1.front().wait_time >= promotion_interval)
            {
                P1.front().priority = 0;
                P0.push_back(P1.front());
                P1.pop_front();
            }

            if(!P0.empty())
            {
                current_id = P0.front().customer_id;
                time_out = current_time + P0.front().slots_remaining;
                P0.pop_front();
            }
            else if(!P1.empty())
            {
                current_id = P1.front().customer_id;
                time_out = current_time + P1.front().slots_remaining;
                P1.pop_front();
            }
        }
        print_state(out_file, current_time, current_id);
        all_done = (customers.empty() && P0.empty() && P1.empty() && (current_id == -1));
    }
    return 0;
}