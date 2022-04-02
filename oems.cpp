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

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // If processors rank is master process, read the numbers file
    if (rank == 0)
        readNumbersFile();

    MPI_Finalize();
    return EXIT_SUCCESS;
}