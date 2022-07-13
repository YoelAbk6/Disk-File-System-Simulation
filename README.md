# Disk-File-System-Simulation


## Description
The program is a simulation of disk file system management in the Index Allocation method.
Index Allocation is a method in which the disk is virtually divided into even blocks and each file has its index block - a block that keeps the location of all the file blocks. In this simulation we will be able to have only one index block for each file, therefore, the maximal file size is block_size^2.
In this program, the "Disk" is a file in our system, that contains one folder only, and all the files will be created under it.
For this program, we implemented 3 classes - 

1. FsFile - Saves file data as the file size, number of blocks in use, and the index of the index block.
2. FileDescriptor - Saves the name of each file, a pointer to its FsFile, and whether it is in use or not.
3. fsDisk - The disk itself, with the following arguments - 
    BitVector - An array that indicates which blocks are free
    is_formated - That indicates if the disk has been formatted or not
    sim_disk_fd - A pointer to the file that is our disk 
    MainDir - A vector that connects the FD of a file and its name
    OpenFileDescriptors - An array of FileDescriptor* that helps us know which FD are open 


fsDisk has 8 main functions:

1. listAll - prints all the disk content in the requested format, including the index blocks.
2. format - The functions receive the block size and format the disk - initializing OpenFileDescriptors, MainDir, BitVector.
3. CreateFile - Receives a file name and creates a new file in the disk by creating a new FsFile and FileDescriptor. The function returns the new file FD.
4. OpenFile - Receives a file name, opens it if it exists and is not already opened, and returns its FD.
5. CloseFile - Receives a file FD and closes it if it exists and is open. Returns the file name.
6. WriteToFile - Receives an fd, a buffer containing the text we want to write and its length. The functions write the text if possible, using the minimal number of blocks that has to be used. The functions call a helper function, WriteToFileRec, a recursive function that receives the same parameter but also the corresponding FsFile and the empty already allocated space in the file. Another helper function is endOfFileIndex which returns the index of the first empty slot of a file.
7. ReadFromFile - Receives a file fd, a buffer buf, and the length to be read. The functions then read from the file len chars into the buffer buf, if possible.
8. DelFile - Receives a file name and deletes all its related data, and the file itself.

Another function that we use in this program is decToBinary which has been given and helps us represent an integer as a char to be able to save the indexes of the blocks using one char in the index block.


## Program Files
ex7_final_proj.2021.cpp - contains the main, the 3 classes and the methods implementations
README.txt

## How to compile?
compile: g++ ex7_final_proj.2021.cpp -o temp
run: ./temp

## Input:
The user needs to choose a number from the menu, and then enter the required parameters such as fd, file name, or length

## Output:
Writes data to the file created as the disk. It also outputs the requested data to the user.
