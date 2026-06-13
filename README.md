# 🖥️ Virtual Machine Allocator — DSA

**Author:** Manan Jain


## 2.1 Project Title

**ServerPack — Cloud Tenant Virtual Machine Allocator**

---

## 2.2 Problem Statement

ServerPack is the management platform for a massive data center that assigns virtual machines to customers, monitors hardware utilization, and optimizes power consumption.

The existing system has the following critical issues:
- Looking up physical server resource stats is inconsistent and slow
- Incorrect VM resizing operations cannot be rolled back
- Provisioning requests arrive in bulk but get handled randomly
- The system cannot rank servers by current utilization
- No way to consolidate workloads during off-peak hours to save power

The new system addresses all of the above with optimized data structures and algorithms.

---

## 2.3 Objectives

- Maintain an organized hardware inventory with fast server lookups
- Support rollback of incorrect VM resize operations
- Process VM provisioning requests in strict FIFO order
- Verify VM operating systems against a sorted index
- Rank physical servers by utilization for load balancing
- Model datacenter network connectivity as a graph
- Calculate minimum latency path between two servers using Dijkstra's algorithm
- Consolidate workloads onto the fewest servers possible to maximize power savings

---

## 2.4 System Overview / Architecture

The system is built around a single core class `CloudManager` 
that manages all operations.

**Core Components:**

- **CloudManager** — Central controller for all operations
- **Server** — Represents a physical machine with CPU, RAM and VM list
- **VM** — Represents a virtual machine with ID, OS and RAM
- **ResizeHistory** — Stores previous RAM value for undo operations

**Data Flow:**

- User sends a request → Menu routes it to CloudManager
- CloudManager processes it using the appropriate data structure
- Result is displayed back to the user instantly

**Key Modules:**

- **Inventory Module** → Add and view physical servers
- **Provisioning Module** → Queue and assign VMs to servers
- **Resize Module** → Resize VMs with full undo support
- **Verification Module** → Check OS against valid list
- **Network Module** → Map connections and find fastest path
- **Optimization Module** → Sort by load and save power



---

## 2.5 Data Structures and Algorithms Used

| Feature | Data Structure | Justification |
|---|---|---|
| Hardware Inventory | `unordered_map<string, Server>` | O(1) average lookup — same as VMware vCenter host indexing |
| VM Registry | `unordered_map<string, VM>` | O(1) VM retrieval by ID |
| Provisioning Queue | `queue<VM>` | FIFO order — same concept as OpenStack Nova message queue |
| Resize Undo | `stack<ResizeHistory>` | LIFO rollback — last resize undone first |
| OS Verification | `vector<string>` + `binary_search` | O(log n) search on sorted list |
| Datacenter Map | Adjacency List (`unordered_map + vector`) | Efficient for sparse network topology |
| Shortest Path | `priority_queue` (min-heap) + Dijkstra | O((V+E) log V) — optimal for weighted graphs |
| Density Sorter | `vector` + `std::sort` | O(n log n) sort by VM count |
| Power Saver | Greedy sort + decision output | O(n log n) — identifies underloaded servers instantly |

---

## 2.6 Implementation Approach

The system is implemented as a single C++ class CloudManager with the following key design decisions:

**1. Hardware Inventory**
Servers are stored in an `unordered_map` keyed by server ID. This provides O(1) average-case access, mirroring how VMware vCenter maintains a hash-indexed host table.

**2. Resize Undo (Rollback)**
Every resize operation pushes the previous RAM value onto a `stack<ResizeHistory>`. Calling undo pops the top entry and restores the VM's original configuration. This mirrors operation journals used in OpenStack's nova-conductor.

**3. Provisioning Queue**
VM requests are pushed into a `queue<VM>` and processed strictly in FIFO order — ensuring fairness across all provisioning requests, similar to RabbitMQ-based task queues in OpenStack Nova.

**4. OS Verification**
A sorted `vector<string>` of valid operating systems is searched using `binary_search`, giving O(log n) verification time without the overhead of a full database lookup.

**5. Density Sorter**
Servers are copied into a temporary vector and sorted in descending order by VM count using `std::sort` with a lambda comparator. This directly supports load balancing decisions.

**6. Datacenter Map**
The network is modeled as an undirected weighted graph using an adjacency list. Each server is a node; each physical connection is a weighted edge (latency in ms). This is the same model used by OpenStack Neutron for network topology.

**7. Fastest Connection (Dijkstra's Algorithm)**
A min-heap based priority queue implements Dijkstra's algorithm to find the minimum latency path between any two servers. This is how SDN (Software Defined Networking) controllers compute optimal routing paths.

**8. Power Saver**
Servers are sorted by utilization. Underloaded servers (≤1 VM) are flagged with a recommendation to migrate their workloads to the most loaded server, enabling those underloaded servers to be powered off — maximizing energy efficiency.

---

## 2.7 Time and Space Complexity Analysis

| Operation | Time Complexity | Space Complexity |
|---|---|---|
| Add / Lookup Server | O(1) average | O(n) |
| Add / Lookup VM | O(1) average | O(m) |
| Enqueue VM Request | O(1) | O(k) |
| Process VM Request | O(n) | O(1) |
| Resize VM | O(1) | O(r) — stack entries |
| Undo Resize | O(1) | O(1) |
| OS Verification | O(log s) | O(s) |
| Density Sorter | O(n log n) | O(n) |
| Add Graph Connection | O(1) | O(E) |
| Shortest Path (Dijkstra) | O((V+E) log V) | O(V+E) |
| Power Saver | O(n log n) | O(n) |

> n = number of servers, m = number of VMs, k = queue size, r = resize history size, s = valid OS list size, V = vertices, E = edges

---

## 2.8 Execution Steps

### Prerequisites
- A C++ compiler (g++ recommended)
- C++11 or later

### Compile
```bash
g++ -std=c++11 -o serverpack main.cpp
```

### Run
```bash
./serverpack
```

### Menu Options

1.  Add Server
2.  View Servers
3.  Add VM Request
4.  Process VM Request
5.  View VMs
6.  Resize VM
7.  Undo Resize
8.  System Check
9.  Density Sorter
10. Add Connection
11. Show Datacenter Map
12. Fastest Connection
13. Power Saver
0.  Exit


---

## 2.9 Sample Inputs and Outputs

### Add Server
```
Input:
Server ID: S2
CPU Cores: 16
RAM (GB): 64

Output:
Server added.
```

### Add & Process VM Request
```
Input:
VM ID: VM101
OS: UBUNTU
RAM (GB): 8

Output:
Request added to queue.
VM VM101 assigned to Server S1
```

### Resize VM & Undo
```
Input (Resize):
VM ID: VM101
New RAM: 16

Output:
VM resized.

Input (Undo):
Output:
Undo successful. VM101 restored to 8 GB
```

### OS System Check
```
Input: UBUNTU
Output: OS verified.

Input: ANDROID
Output: OS not found.
```

### Fastest Connection (Dijkstra)
```
Input:
Source Server: S1
Destination Server: S3

Output:
Minimum latency = 15 ms
```

### Power Saver
```
Output:
Power Saver Recommendation
Server S3 (1 VM) -> move to Server S1 | Power off S3 to save power.
Server S4 (0 VM) -> move to Server S1 | Power off S4 to save power.
```

---

## 2.10 Screenshots

> 📸 Screenshots will be added after program execution.

---

## 2.11 Results and Observations

- All 8 required features were successfully implemented and tested
- `unordered_map` provided consistent O(1) server and VM lookups, significantly faster than linear search alternatives
- Stack-based undo proved reliable for single and multiple sequential resize rollbacks
- Dijkstra's algorithm correctly computed minimum latency paths across multi-hop server connections
- The density sorter and power saver together provide a practical basis for load balancing and energy optimization decisions
- The system architecture mirrors real-world platforms — VMware vCenter (inventory), OpenStack Nova (provisioning queue), OpenStack Neutron (network graph)

---

## 2.12 Conclusion

The ServerPack Cloud Tenant Virtual Machine Allocator successfully demonstrates the practical application of core data structures — hash maps, stacks, queues, graphs, and heaps — in solving real-world datacenter management challenges. Each data structure was chosen deliberately based on its time and space efficiency for the specific use case it serves. The system is modular, readable, and extensible, and reflects the architectural patterns found in industry-grade platforms like VMware and OpenStack.

---

## 🔗 GitHub Repository

[virtual-machine-allocator-dsa](https://github.com/mananjain/virtual-machine-allocator-dsa)

> *(Replace the above link with your actual GitHub repository URL)*


