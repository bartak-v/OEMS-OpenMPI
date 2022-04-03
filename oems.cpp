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

#define MASTER 0
#define MY_TAG 0
#define TAG_X 1
#define TAG_Y 2

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
int x,y,L,H=0;
void compare_and_save()
{
    if(x < y){
        H = y;
        L = x;
    }else if (x==y){
        H=x;
        L=x;
    }else{
        H=x;
        L=y;
    }
}

int main(int argc, char *argv[])
{
    int rank;
    int size;
    //[x,y] inputs to the node | [L]ower output of the comparator | [H]igher output of the comparator

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    MPI_Request request;

    if (rank == 0)
    {
        readNumbersFile();
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        int receiver;
        for (int i = 0; i < in_numbers.size(); i++)
        {
            int a = in_numbers[i];
            int b = in_numbers[i + 1];
            if (i == 0)
                receiver = 0;
            if (i == 2)
                receiver = 1;
            if (i == 4)
                receiver = 2;
            if (i == 6)
                receiver = 3;
            MPI_Isend(&a, 1, MPI_INT, receiver, TAG_X, MPI_COMM_WORLD, &request);
            MPI_Isend(&b, 1, MPI_INT, receiver, TAG_Y, MPI_COMM_WORLD, &request);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 4 1x1 sítě
    if (rank < 4)
    {
        // Input
        MPI_Irecv(&x, 1, MPI_INT, MASTER, TAG_X, MPI_COMM_WORLD, &request);
        MPI_Irecv(&y, 1, MPI_INT, MASTER, TAG_Y, MPI_COMM_WORLD, &request);
        compare_and_save();
        printf("I am %d X %d   Y %d  H %d L %d \n", rank, x, y, H, L);

        //Send it to the two 2x2 networks | output L H
        MPI_Isend(&a, 1, MPI_INT, receiver, TAG_X, MPI_COMM_WORLD, &request);
        MPI_Isend(&b, 1, MPI_INT, receiver, TAG_Y, MPI_COMM_WORLD, &request);
    }

    

    // Udělat 2x síť 2x2

    // Udělat 4x4 síť


    MPI_Finalize();
    return EXIT_SUCCESS;
}