#include <iostream>
#include <cmath>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <random> // std::default_random_engine
#include <chrono> // std::chrono::system_clock
#include <unordered_map>

using namespace std;

// For debugging and testing
// ##############################################################################
void printArr(int *arr, int L)
{
    cout << "Array: ";
    for (int i = 0; i < L; i++)
    {
        cout << arr[i] << " ";
    }
    cout << endl;
}

void printMap(unordered_map<int, int> &m)
{
    cout << "[ ";
    for (auto &item : m)
    {
        cout << item.first << ":" << item.second << " ";
    }
    cout << "]\n";
}
void printVector(vector<int> path)
{
    for (auto i : path)
        std::cout << i << ' ';
    cout << endl;
}

// ###############################################################################

class btree
{

public:
    btree(int *data, int N);
    ~btree();
    void printTree();
    int getHeight();
    int getTotalNodes();
    int getTotalLeaves();
    int getLeafIndex(int branch);
    int *getPath(int branch);
    unordered_map<int, int> &getPositionMap(); // for test purpose
    int access(int op, int block, int new_data);
    void printData();

private:
    default_random_engine e;
    vector<int> tree;
    int inputSize;
    int height;
    vector<int> userData;
    unordered_map<int, int> pos_map;
    vector<int> stash;
    unordered_map<int, int> stash_data;

    int getParent(int node);
    int randomPath(int node);
    void initPositionMap();
    int readBucket(int block);
    void writeBucket(int block, int new_data);
};

// template <int listSize>
btree::btree(int *data, int N)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    // change seed back to seed
    default_random_engine gen(1);
    e = gen;
    inputSize = N;
    height = ceil(log(inputSize) / log(2)) - 1;

    for (int x = 0; x < inputSize; x++)
    {
        userData.push_back(data[x]);
    }

    // dummy data

    userData.push_back(INT_MIN);

    for (int i = 0; i < getTotalNodes(); i++)
    {
        tree.push_back(inputSize);
    }

    for (int i = 0; i < inputSize; i++)
    {
        tree[i] = i;
    }

    //////debug
    cout << "userData: ";
    printVector(userData);
    cout << "tree: ";
    printVector(tree);
    //////

    std::shuffle(tree.begin(), tree.end(), e);

    //////debug
    cout << "tree: ";
    printVector(tree);
    //////

    initPositionMap();

    //////debug
    cout << "Pos Map: ";
    printMap(pos_map);
    //////
}

btree::~btree()
{
    tree.clear();
}

void btree::printTree()
{
    cout << "Binary Tree: ";
    for (int i : tree)
    {
        cout << i << " ";
    }
    cout << endl;
}

int btree::getHeight()
{
    return height;
}

int btree::getTotalNodes()
{
    return pow(2, height + 1) - 1;
}

int btree::getTotalLeaves()
{
    return pow(2, height);
}

// Changes the branch [0 - no. of leaves] to the actual leaf node [0 - treeSize]
// leaf number to node number of the leaf
int btree::getLeafIndex(int leaf)
{
    return pow(2, height) + leaf - 1;
}

// finds a node's parent
int btree::getParent(int node)
{
    return floor((node - 1) / 2.0);
}

int *btree::getPath(int leaf)
{
    int node = leaf;
    int *path = new int[height + 1];
    for (int i = height; i >= 0; i--)
    {
        path[i] = node;
        node = getParent(node);
    }
    return path;
}

int btree::randomPath(int node)
{
    std::uniform_int_distribution<int> distr(0, 1);
    int rand = distr(e);
    int child1 = 2 * node + 1;
    int child2 = 2 * node + 2;
    // if leaf node, assign itself
    if (child2 > (tree.size() - 1))
    {
        int leaves = (pow(2, height) - 1);
        return node - leaves; // node - no. of leaves
    }
    else
    {
        if (rand == 0)
        {
            return randomPath(child1);
        }
        else
        {
            return randomPath(child2);
        }
    }
}

void btree::initPositionMap()
{
    for (int i = 0; i < tree.size(); i++)
    {
        int block = tree[i];

        if (block != inputSize)
        {
            pos_map[block] = randomPath(i);
        }
    }
}

unordered_map<int, int> &btree::getPositionMap()
{
    return pos_map;
}

int btree::readBucket(int block)
{
    cout << "Reading: " << block << endl;
    return userData[block];
}

void btree::writeBucket(int block, int new_data)
{
    cout << "Writing: " << block << " new data: " << new_data << endl;
    userData[block] = new_data;
}

// returns: 0 for write operation and  the value for a read operation
int btree::access(int op, int block, int new_data)
{

    int noOfLeafs = getTotalLeaves();
    int returnData = 0; // should be an encrypted data type
    std::uniform_int_distribution<int> distr(0, noOfLeafs - 1);

    // Steps: 1-2
    int x = pos_map[block];
    // cout << "Previous position: " << x << endl;
    pos_map[block] = distr(e);
    // cout << "New position: " << pos_map[block] << endl;

    // Steps 3-5
    int leafNode = getLeafIndex(x);
    int *path = getPath(leafNode);
    printArr(path, 4);

    for (int i = 0; i <= height; i++)
    {
        int node = path[i];

        int blk = tree[node];
        stash.push_back(blk);
        int bucket = readBucket(blk);
        stash_data[blk] = bucket;
    }
    printVector(stash);
    printMap(stash_data);

    // Steps 6-9
    if (op == 0)
    { // Read operation
        for (auto &x : stash_data)
        {
            // can do re-enc here
            returnData = (x.first == block) ? stash_data[x.first] : returnData;
        }
    }
    else if (op == 1)
    { // Write operation
        for (auto &x : stash_data)
        {
            // can do re-enc here
            stash_data[x.first] = (x.first == block) ? new_data : stash_data[x.first];
            userData[x.first] = (x.first == block) ? new_data : userData[x.first];
        }
    }
    cout << "return: " << returnData << endl;
    for (int i = height; i >= 0; i--)
    // for each bucket
    {
        int bucket = path[i]; // 8
        int n = tree[bucket]; // n is block id
        // writeBucket(n, stash_data[n]);
        cout << endl
             << "bucket: " << bucket << endl;

        int capacity = 1;
        int j = 0;
        while (j < stash.size() && capacity != 0)
        // for each block in stash
        {
            // because capacity  =1
            tree[bucket] = inputSize;
            if (stash[j] == inputSize)
            // if dummy : erase
            {
                cout << "dummy: "
                     << " Stash[" << j << "]: " << stash[j] << endl;
                stash.erase(stash.begin() + j);
            }
            else
            {
                int current_branch = pos_map[stash[j]];
                int x = getLeafIndex(current_branch);
                int *a = getPath(x);
                if (bucket == a[i])
                {
                    cout << "match: "
                         << " Stash[" << j << "]: " << stash[j] << " path[j]: " << a[i] << endl;
                    tree[bucket] = stash[j];
                    stash.erase(stash.begin() + j);
                    capacity--;
                }
                else
                {
                    j++;
                    cout << "NOT: "
                         << " Stash[" << j << "]: " << stash[j] << " path[j]: " << a[i] << endl;
                }
            }
        }
    }
    printTree();
    cout << "stash: ";
    printVector(stash);
    // if (stash.size() == 0)
    // {
    //     stash_data.clear();
    // }
    // else
    // {
    //     unordered_map<int, int> temp;
    //     for (auto &x : stash_data)
    //     {
    //         if (std::find(stash.begin(), stash.end(), x.first) != stash.end())
    //             temp[x.first] = stash_data[x.first];
    //     }
    //     stash_data = temp;
    //     temp.~unordered_map();
    // }

    // cout << "The final stash data: ";
    // printMap(stash_data);
    // cout << "The final stash: ";
    // printVector(stash);
    // cout << "The final tree: ";
    // printTree();
    return returnData;
}

void btree::printData()
{
    cout << "user data: ";
    for (int i = 0; i < inputSize; i++)
        cout << userData[i] << ' ';
    cout << endl;
}

int main()
{
    int arr[] = {00, 11, 22, 33, 44, 55, 66, 77, 88, 99};
    int N = sizeof(arr) / sizeof(arr[0]);

    btree *tree = new btree(arr, N);

    tree->printTree();
    cout << "height: " << tree->getHeight() << " leaves: " << tree->getTotalLeaves() << " nodes: " << tree->getTotalNodes() << endl;
    cout << " Original data: ";
    tree->printData();
    printMap(tree->getPositionMap());

    cout << "Write Operation Output: " << tree->access(1, 2, 200) << endl; // op 1: write the value 200 at index 2
    tree->printData();
    cout << endl
         << endl;
    cout << "Read Operation Output: " << tree->access(0, 5, 0) << endl; // op 0: read the value at index
    tree->printData();

    cout << endl
         << endl;
    cout << "Read Operation Output: " << tree->access(0, 2, 0) << endl; // op 0: read the value at index
    tree->printData();
}