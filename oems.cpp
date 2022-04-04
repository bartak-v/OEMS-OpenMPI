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
#define MASTER_TAG 3

vector<int> in_numbers;
vector<int> sorted_numbers;
int one, x, y, L, H = 0;

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

void compare_and_save(int rank)
{
    if (x < y)
    {
        H = y;
        L = x;
    }
    else if (x == y)
    {
        H = x;
        L = x;
    }
    else
    {
        H = x;
        L = y;
    }
    // printf("I am %d X %d   Y %d  H %d L %d \n", rank, x, y, H, L);
}

void send_output_to_master(int a, int tag, MPI_Request req)
{
    MPI_Isend(&a, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &req);
}

void receive_one_input(int src, int tag, MPI_Request req)
{
    MPI_Irecv(&one, 1, MPI_INT, src, tag, MPI_COMM_WORLD, &req);
}

// Send two outputs of the comparator a,b, to receiverL and receiverH with custom tags tag1 and tag2 (they can equal)
void send_output(int a, int b, int receiverL, int receiverH, int tag1, int tag2, MPI_Request req)
{
    MPI_Isend(&a, 1, MPI_INT, receiverL, tag1, MPI_COMM_WORLD, &req);
    MPI_Isend(&b, 1, MPI_INT, receiverH, tag2, MPI_COMM_WORLD, &req);
}

// Receive two inputs x,y, of the previous two comparators src1 src2 with custom tags tag1 and tag2 (they can equal)
void receive_input(int src1, int src2, int tag1, int tag2, MPI_Request req)
{
    MPI_Irecv(&x, 1, MPI_INT, src1, tag1, MPI_COMM_WORLD, &req);
    MPI_Irecv(&y, 1, MPI_INT, src2, tag2, MPI_COMM_WORLD, &req);
}

void receive_input_master(MPI_Request req)
{
    receive_one_input(10, 0, req);
    sorted_numbers.push_back(one);
    for (int i = 0; i < 3; i++)
    {
        receive_input(16 + i, 16 + i, 16 + i, 16 + i + 5, req);
        sorted_numbers.push_back(x);
        sorted_numbers.push_back(y);
    }
    receive_one_input(13, 0, req);
    sorted_numbers.push_back(one);

    for (int number : sorted_numbers)
    {
        cout << number << endl;
    }
}

int main(int argc, char *argv[])
{
    int rank;
    int size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Status status;
    MPI_Request request;

    if (rank == 0)
    {
        readNumbersFile(); // Reading the files from the numbers file into a vector
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
            send_output(a, b, receiver, receiver, TAG_X, TAG_Y, request); // Sending the numbers to the input of the sorting network
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 4 1x1 comparators
    if (rank < 4)
    {
        int receiverL, receiverH; // 1 send L, 2 send H
        // Input
        receive_input(MASTER, MASTER, TAG_X, TAG_Y, request);
        compare_and_save(rank);

        // Send it to the two 2x2 networks | output L H
        if (rank == 0 || rank == 1)
        {
            receiverL = 4;
            receiverH = 5;
        }
        if (rank == 2 || rank == 3)
        {
            receiverL = 6;
            receiverH = 7;
        }
        send_output(L, H, receiverL, receiverH, MY_TAG, MY_TAG, request);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 2 2x2 networks input part
    if (rank < 8 && rank > 3)
    {
        // Input from 1x1 network
        int sourceX, sourceY, receiverL, receiverH;
        if (rank == 4)
        {
            sourceX = 0;
            sourceY = 1;
            receiverL = 10;
            receiverH = 8;
        }
        if (rank == 5)
        {
            sourceX = 0;
            sourceY = 1;
            receiverL = 8;
            receiverH = 13;
        }
        if (rank == 6)
        {
            sourceX = 2;
            sourceY = 3;
            receiverL = 10;
            receiverH = 9;
        }
        if (rank == 7)
        {
            sourceX = 2;
            sourceY = 3;
            receiverL = 9;
            receiverH = 13;
        }

        receive_input(sourceX, sourceY, MY_TAG, MY_TAG, request);
        compare_and_save(rank);
        send_output(L, H, receiverL, receiverH, MY_TAG, MY_TAG, request); // Send it to 8,9,10,13
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Middle and output of the 2x2 network
    if (rank == 8 || rank == 9)
    {
        int sourceX, sourceY;
        int receiverL = 12;
        int receiverH = 11;

        if (rank == 8)
        {
            sourceX = 4;
            sourceY = 5;
        }
        if (rank == 9)
        {
            sourceX = 6;
            sourceY = 7;
        }

        receive_input(sourceX, sourceY, MY_TAG, MY_TAG, request);
        compare_and_save(rank);
        send_output(L, H, receiverL, receiverH, MY_TAG, MY_TAG, request);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 4x4 input part
    if (rank < 14 && rank > 9)
    {
        // Input from 1x1 network
        int sourceX, sourceY, receiverL, receiverH;
        if (rank == 10)
        {
            sourceX = 4;
            sourceY = 6;
            receiverL = 0;
            receiverH = 14;
            send_output_to_master(L, rank, request);
        } // 10 and 13 sends L to MASTER
        if (rank == 11)
        {
            sourceX = 9;
            sourceY = 8;
            receiverL = 14;
            receiverH = 18;
        }
        if (rank == 12)
        {
            sourceX = 8;
            sourceY = 9;
            receiverL = 16;
            receiverH = 15;
        }
        if (rank == 13)
        {
            sourceX = 5;
            sourceY = 7;
            receiverL = 15;
            receiverH = 0;
            send_output_to_master(H, rank, request);
        }

        receive_input(sourceX, sourceY, MY_TAG, MY_TAG, request);
        compare_and_save(rank);
        send_output(L, H, receiverL, receiverH, MY_TAG, MY_TAG, request); // Send it to 14,15,16,18, 0
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Middle of the 4x4 network
    if (rank == 14 || rank == 15)
    {
        int sourceX, sourceY, receiverL, receiverH;
        if (rank == 14)
        {
            sourceX = 10;
            sourceY = 11;
            receiverL = 16;
            receiverH = 17;
        }
        if (rank == 15)
        {
            sourceX = 12;
            sourceY = 13;
            receiverL = 17;
            receiverH = 18;
        }

        receive_input(sourceX, sourceY, MY_TAG, MY_TAG, request);
        compare_and_save(rank);
        send_output(L, H, receiverL, receiverH, MY_TAG, MY_TAG, request); // Send it to 16,17,18
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Output of the 4x4 network
    if (rank > 15)
    {
        int sourceX, sourceY;
        if (rank == 16)
        {
            sourceX = 14;
            sourceY = 12;
        }
        if (rank == 17)
        {
            sourceX = 14;
            sourceY = 15;
        }
        if (rank == 18)
        {
            sourceX = 11;
            sourceY = 15;
        }

        receive_input(sourceX, sourceY, MY_TAG, MY_TAG, request);
        compare_and_save(rank);
        send_output(L, H, 0, 0, rank, rank + 5, request); // Send it to 0
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Ibarrier(MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

    // Gathering + printing sorted numbers
    if (rank == 0)
        receive_input_master(request);

    MPI_Finalize();
    return EXIT_SUCCESS;
}