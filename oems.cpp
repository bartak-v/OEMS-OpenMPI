/* 
    Project: Odd Even Merge Sort using OpenMPI (PRL)
    Author:  Vít Barták
    Date:    02.04.2022
*/

#include <iostream>
#include <stdio.h>
#include <mpi.h>
#include <fstream>
#include <vector>

using namespace std;

vector<int> in_numbers;
vector<int> sorted_numbers;

void readNumbersFile()
{
    string filename("numbers");
    vector<char> bytes;
    char byte;
    ifstream input_file(filename);

    // Read the bytes from the file
    while (input_file.get(byte))
        bytes.push_back(byte);
    input_file.close();    
    // Push the numbers into a vector as int
    for (const auto &i : bytes)
    {
        int num = ((int)i < 0) ? 127 - (int)i : (int)i; // Map the loaded chars into range 0-255
        in_numbers.push_back(num);
        cout << num << " ";
    }
    cout << endl;
}

struct node {
  int rank;
  int size;
  int in1;
  int in2;
  int H;
  int L;
};

int main(int argc, char *argv[])
{
    //int rank, size;
    node n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &n.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n.size);

    // If processors rank is master process, read the numbers file
    if (n.rank == 0)
        readNumbersFile();

    MPI_Finalize();
    return EXIT_SUCCESS;
}