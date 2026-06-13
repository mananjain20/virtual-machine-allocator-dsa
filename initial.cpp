#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <limits>
using namespace std;
// Simple VM record with id, OS and RAM
class VM
{
public:
    string vmId;
    string os;
    int ram;

    VM() {}
    VM(string id, string o, int r) : vmId(id), os(o), ram(r) {}
};

// Simple server record holding resources and hosted VMs
class Server
{
public:
    string serverId;
    int cpu;
    int ram;
    vector<string> vmList;

    Server() {}
    Server(string id, int c, int r) : serverId(id), cpu(c), ram(r) {}
};

// History entry to allow undoing a VM resize
struct ResizeHistory
{
    string vmId;
    int oldRam;
};

// Cloud manager that keeps servers, VMs, network and operations
class CloudManager
{
private:
    // Map serverId -> Server for fast lookup
    unordered_map<string, Server> servers;
    // Map vmId -> VM for fast lookup
    unordered_map<string, VM> vms;

    // Queue for incoming VM requests (FIFO)
    queue<VM> requestQueue;
    // Stack to record resize operations for undo (LIFO)
    stack<ResizeHistory> resizeStack;

    // Allowed OS names (kept in a vector for easy sorting/search)
    vector<string> validOS = {"UBUNTU", "WINDOWS", "LINUX", "DEBIAN", "FEDORA", "MACOS"};

    // Network graph as adjacency list: server -> list of (neighbor, latency)
    unordered_map<string, vector<pair<string, int>>> graph;

public:
    // Add a new server with user-provided specs
    void addServer()
    {
        string id;
        int cpu, ram;

        cout << "Server ID: ";
        cin >> id;
        cout << "CPU Cores: ";
        cin >> cpu;
        cout << "RAM (GB): ";
        cin >> ram;

        servers[id] = Server(id, cpu, ram);
        cout << "Server added.\n";
    }

    // Print all servers and their basic info
    void viewServers()
    {
        if (servers.empty())
        {
            cout << "No servers found.\n";
            return;
        }

        for (auto &entry : servers)
        {
            Server &s = entry.second;
            cout << "\nServer: " << s.serverId
                 << "\nCPU: " << s.cpu
                 << "\nRAM: " << s.ram
                 << "\nVM Count: " << s.vmList.size()
                 << "\n-----------------\n";
        }
    }
    // Queue a VM creation request from user input
    void addVMRequest()
    {
        string id, os;
        int ram;

        cout << "VM ID: ";
        cin >> id;
        cout << "OS: ";
        cin >> os;
        cout << "RAM (GB): ";
        cin >> ram;

        requestQueue.push(VM(id, os, ram));
        cout << "Request added to queue.\n";
    }

    // Assign next queued VM to the least-loaded server
    void processVMRequest()
    {
        if (requestQueue.empty())
        {
            cout << "No pending requests.\n";
            return;
        }

        if (servers.empty())
        {
            cout << "Add a server first.\n";
            return;
        }

        VM vm = requestQueue.front();
        requestQueue.pop();
        // Find server with minimum VM count
        auto bestServer = servers.begin();

        for (auto it = servers.begin(); it != servers.end(); it++)
        {
            if (it->second.vmList.size() <
                bestServer->second.vmList.size())
            {
                bestServer = it;
            }
        }

        bestServer->second.vmList.push_back(vm.vmId);

        vms[vm.vmId] = vm;

        cout << "VM " << vm.vmId
             << " assigned to Server "
             << bestServer->second.serverId
             << "\n";
    }

    // List all VMs with their properties
    void viewVMs()
    {
        if (vms.empty())
        {
            cout << "No VMs available.\n";
            return;
        }

        for (auto &entry : vms)
        {
            VM &vm = entry.second;

            cout << "\nVM ID: " << vm.vmId
                 << "\nOS: " << vm.os
                 << "\nRAM: " << vm.ram << " GB"
                 << "\n-----------------\n";
        }
    }
    // Change RAM of a VM and record history for undo
    void resizeVM()
    {
        string id;
        int newRam;

        cout << "VM ID: ";
        cin >> id;

        if (vms.find(id) == vms.end())
        {
            cout << "VM not found.\n";
            return;
        }

        cout << "New RAM: ";
        cin >> newRam;

        resizeStack.push({id, vms[id].ram});
        vms[id].ram = newRam;

        cout << "VM resized.\n";
    }

    // Revert the last VM resize using the resize stack
    void undoResize()
    {
        if (resizeStack.empty())
        {
            cout << "Nothing to undo.\n";
            return;
        }

        ResizeHistory last = resizeStack.top();
        resizeStack.pop();

        vms[last.vmId].ram = last.oldRam;

        cout << "Undo successful.\n";
    }

    // Verify whether an OS is in the allowed list
    void systemCheck()
    {
        // sort OS list once so we can binary search it quickly
        sort(validOS.begin(), validOS.end());

        string os;
        cout << "Enter OS to verify: ";
        cin >> os;

        if (binary_search(validOS.begin(), validOS.end(), os))
            cout << "OS verified.\n";
        else
            cout << "OS not found.\n";
    }

    // Show servers ordered by number of VMs (high -> low)
    void densitySorter()
    {
        vector<Server> list;

        for (auto &entry : servers)
            list.push_back(entry.second);

        // sort servers by VM count (descending) to show utilization
        sort(list.begin(), list.end(),
             [](Server a, Server b)
             {
                 return a.vmList.size() > b.vmList.size();
             });

        cout << "\nServers by utilization:\n";

        for (auto &s : list)
        {
            cout << s.serverId
                 << " -> " << s.vmList.size()
                 << " VMs\n";
        }
    }

    // Add a bidirectional network link between two servers
    void addConnection()
    {
        string a, b;
        int latency;

        cout << "Server A: ";
        cin >> a;
        cout << "Server B: ";
        cin >> b;
        cout << "Latency: ";
        cin >> latency;

        graph[a].push_back({b, latency});
        graph[b].push_back({a, latency});

        cout << "Connection added.\n";
    }
    // Print adjacency list of the datacenter network
    void showMap()
    {
        for (auto &entry : graph)
        {
            cout << "\n"
                 << entry.first << " -> ";

            for (auto &edge : entry.second)
            {
                cout << "(" << edge.first
                     << ", " << edge.second
                     << "ms) ";
            }
        }
        cout << "\n";
    }

    // Compute minimum-latency path between two servers (Dijkstra)
    void fastestConnection()
    {
        string source, destination;

        cout << "Source Server: ";
        cin >> source;

        cout << "Destination Server: ";
        cin >> destination;

        unordered_map<string, int> dist;

        for (auto &node : graph)
            dist[node.first] = INT_MAX;

        if (graph.find(source) == graph.end())
        {
            cout << "Source not found.\n";
            return;
        }

        priority_queue<pair<int, string>,
                       vector<pair<int, string>>,
                       greater<pair<int, string>>>
            pq;

        dist[source] = 0;
        pq.push({0, source});

        // Dijkstra main loop: relax edges until all shortest paths found
        while (!pq.empty())
        {
            auto current = pq.top();
            pq.pop();

            int currentDist = current.first;
            string currentNode = current.second;

            for (auto &neighbor : graph[currentNode])
            {
                string nextNode = neighbor.first;
                int weight = neighbor.second;

                if (currentDist + weight < dist[nextNode])
                {
                    dist[nextNode] = currentDist + weight;
                    pq.push({dist[nextNode], nextNode});
                }
            }
        }
        if (dist[destination] == INT_MAX)
            cout << "No path found.\n";
        else
            cout << "Minimum latency = "
                 << dist[destination]
                 << " ms\n";
    }
    // Recommend servers to power off by finding lightly used ones
    void powerSaver()
    {
        vector<Server> list;

        for (auto &entry : servers)
            list.push_back(entry.second);

        // sort servers by VM count (descending) to find light ones
        sort(list.begin(), list.end(),
             [](Server a, Server b)
             {
                 return a.vmList.size() > b.vmList.size();
             });
        cout << "\nPower Saver Recommendation\n";

        for (auto &s : list)
        {
            if (s.vmList.size() <= 1)
            {
                     cout<< "Server " << s.serverId
                     << " (" << s.vmList.size() << " VM) → move to Server "
                     << list[0].serverId
                     << " | Power off " << s.serverId << " to save power.\n";
            }
        }
    }
};
// Menu-driven main to interact with the CloudManager
int main()
{
    CloudManager manager;
    int choice;

    do
    {
        cout << "\n===== SERVERPACK CLOUD SYSTEM =====\n";
        cout << "1. Add Server\n";
        cout << "2. View Servers\n";
        cout << "3. Add VM Request\n";
        cout << "4. Process VM Request\n";
        cout << "5. View VMs\n";
        cout << "6. Resize VM\n";
        cout << "7. Undo Resize\n";
        cout << "8. System Check\n";
        cout << "9. Density Sorter\n";
        cout << "10. Add Connection\n";
        cout << "11. Show Datacenter Map\n";
        cout << "12. Fastest Connection\n";
        cout << "13. Power Saver\n";
        cout << "0. Exit\n";
        cout << "Choice: ";
        cin >> choice;
        switch (choice)
        {
        case 1:
            manager.addServer();
            break;
        case 2:
            manager.viewServers();
            break;
        case 3:
            manager.addVMRequest();
            break;
        case 4:
            manager.processVMRequest();
            break;
        case 5:
            manager.viewVMs();
            break;
        case 6:
            manager.resizeVM();
            break;
        case 7:
            manager.undoResize();
            break;
        case 8:
            manager.systemCheck();
            break;
        case 9:
            manager.densitySorter();
            break;
        case 10:
            manager.addConnection();
            break;
        case 11:
            manager.showMap();
            break;
        case 12:
            manager.fastestConnection();
            break;
        case 13:
            manager.powerSaver();
            break;
        case 0:
            cout << "Thank You!\n";
            break;
        default:
            cout << "Invalid Choice.\n";
        }
    } while (choice != 0);
    return 0;
}
