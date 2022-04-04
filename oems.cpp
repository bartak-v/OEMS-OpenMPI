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

void receive_input(int src,int tag1,int tag2,MPI_Request req){

    MPI_Irecv(&x, 1, MPI_INT, src, tag1, MPI_COMM_WORLD, &req);
    MPI_Irecv(&y, 1, MPI_INT, src, tag2, MPI_COMM_WORLD, &req);
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
        int receiverL,receiverH; //1 send L, 2 send H
        // Input
        MPI_Irecv(&x, 1, MPI_INT, MASTER, TAG_X, MPI_COMM_WORLD, &request);
        MPI_Irecv(&y, 1, MPI_INT, MASTER, TAG_Y, MPI_COMM_WORLD, &request);
        //receive_input(MASTER,TAG_X,TAG_Y,request);
        compare_and_save();
        printf("I am %d X %d   Y %d  H %d L %d \n", rank, x, y, H, L);

        //Send it to the two 2x2 networks | output L H
        if (rank==0 || rank==1) {receiverL = 4; receiverH = 5;}
        if (rank==2 || rank==3) {receiverL = 6; receiverH = 7;}
        MPI_Isend(&L, 1, MPI_INT, receiverL, MY_TAG, MPI_COMM_WORLD, &request); // send lower value
        MPI_Isend(&H, 1, MPI_INT, receiverH, MY_TAG, MPI_COMM_WORLD, &request); // send higher  value
    }
    MPI_Barrier(MPI_COMM_WORLD);
    

    // Udělat 2x síť 2x2
    if(rank < 8 && rank > 3){

        // Input from 1x1 network
        int sourceX,sourceY,receiverL,receiverH;
        if (rank==4 || rank==5) {sourceX = 0; sourceY = 1;}
        if (rank==6 || rank==7) {sourceX = 2; sourceY = 3;}
        MPI_Irecv(&x, 1, MPI_INT, sourceX, MY_TAG, MPI_COMM_WORLD, &request);
        MPI_Irecv(&y, 1, MPI_INT, sourceY, MY_TAG, MPI_COMM_WORLD, &request);
        compare_and_save();
        printf("I am %d X %d   Y %d  H %d L %d \n", rank, x, y, H, L);

        // Send it to 8,9,10,13
        if (rank==4) {receiverL = 10; receiverH = 8;}
        if (rank==5) {receiverL = 8; receiverH = 13;}
        if (rank==6) {receiverL = 10; receiverH = 9;}
        if (rank==7) {receiverL = 9; receiverH = 13;}
        MPI_Isend(&L, 1, MPI_INT, receiverL, MY_TAG, MPI_COMM_WORLD, &request); // send lower value
        MPI_Isend(&H, 1, MPI_INT, receiverH, MY_TAG, MPI_COMM_WORLD, &request); // send higher  value
    }
    // Middle of the 2x2 network
    if(rank == 8 || rank == 9){

    }

    // Udělat 4x4 síť


    MPI_Finalize();
    return EXIT_SUCCESS;
}