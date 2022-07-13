//Disk File System Simulation
//Authored by YoelABK6

#include <iostream>

#include <vector>

#include <map>

#include <string.h>

#include <math.h>

#include <assert.h>

#include <sys/types.h>

#include <unistd.h>

#include <sys/stat.h>

#include <fcntl.h>

using namespace std;

#define DISK_SIZE 40
#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

class FsFile {
    int file_size;

    int block_in_use;

    int index_block;

    int block_size;

public:

    FsFile(int _block_size);

    int getfile_size() const;

    int getIndexBlock() const;

    void setIndexBlock(int indexBlock);

    void updateFileSize(int size);

    int getBlockInUse() const;

    void setBlockInUse(int blockInUse);
};

class FileDescriptor {
    string file_name;
    FsFile * fs_file;
    bool inUse;
public:
    FsFile * getFsFile() const {
        return fs_file;
    }

public:
    FileDescriptor(string FileName, FsFile * fsi) {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }

    bool isInUse() const {
        return inUse;
    }

    void setInUse(bool inUse) {
        FileDescriptor::inUse = inUse;
    }

    string getFileName() {
        return file_name;
    }
};

class fsDisk {

    FILE * sim_disk_fd;

    bool is_formated;

    int BitVectorSize;

    int * BitVector;

    // filename and one fsFile.
    vector < FileDescriptor * > MainDir;

    /*OpenFileDescriptors --  when you open a file,
    the operating system creates an entry to represent that file
    his entry number is the file descriptor.*/
    FileDescriptor ** OpenFileDescriptors;

    int direct_enteris;

    int block_size;

    int maxSize;

    int freeBlocks;

    int availableSize;

    int availableBLock();

    int availableFD();

    int findFileByName(string fileName);

    void decToBinary(int n, char & c);

    int endOfFileIndex(FsFile * currFS, int emptyAllocatedSpace);

    int WriteToFileRec(int fd, char * buf, int len, int emptyAllocatedSpace, FsFile * currFS);

    int DelDiskData(int indexBlock, FsFile * currFS);

public:
    fsDisk();

    ~fsDisk();

    void listAll();

    void fsFormat(int blockSize);

    int CreateFile(string fileName);

    int OpenFile(string fileName);

    string CloseFile(int fd);

    int WriteToFile(int fd, char * buf, int len);

    int DelFile(string FileName);

    int ReadFromFile(int fd, char * buf, int len);

};

FsFile::FsFile(int _block_size) {

    file_size = 0;

    block_in_use = 0;

    block_size = _block_size;

    index_block = -1;

}

int FsFile::getBlockInUse() const {
    return block_in_use;
}

void FsFile::setBlockInUse(int blockInUse) {
    block_in_use = blockInUse;
}

void FsFile::updateFileSize(int size) {
    file_size += size;
}

int FsFile::getIndexBlock() const {
    return index_block;
}

void FsFile::setIndexBlock(int indexBlock) {
    index_block = indexBlock;
}

int FsFile::getfile_size() const {
    return file_size;
}

fsDisk::fsDisk() {
    sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
    assert(sim_disk_fd);

    for (int i = 0; i < DISK_SIZE; i++) {
        int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
        ret_val = (int) fwrite("\0", 1, 1, sim_disk_fd);
        assert(ret_val == 1);
    }

    fflush(sim_disk_fd);

    direct_enteris = 0;

    block_size = 0;

    is_formated = false;

    availableSize = DISK_SIZE;
}

fsDisk::~fsDisk() {
    while (!MainDir.empty())
        DelFile(MainDir[0] -> getFileName());
    delete[] BitVector;
    delete[] OpenFileDescriptors;
    MainDir.clear();
    fclose(sim_disk_fd);
}

//Prints the whole disk in the requested format
void fsDisk::listAll() {
    int i;
    for (i = 0; i < (int) MainDir.size(); i++) {
        cout << "index: " << i << ": FileName: " << MainDir[i] -> getFileName() << " , isInUse: " << MainDir[i] -> isInUse() <<
        endl;
    }

    char buff;
    cout << "Disk content: '";
    for (i = 0; i < DISK_SIZE; i++) {
        int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
        ret_val = (int) fread( & buff, 1, 1, sim_disk_fd);
        cout << buff;
    }
    cout << "'" << endl;

}

//Formats the Disk
void fsDisk::fsFormat(int blockSize) {
    this -> block_size = blockSize;
    this -> maxSize = (int) pow(blockSize, 2);
    this -> BitVectorSize = DISK_SIZE / blockSize;
    this -> freeBlocks = BitVectorSize;
    if (is_formated) {
        while (!MainDir.empty())
            DelFile(MainDir[0] -> getFileName());
        delete[] OpenFileDescriptors;
        delete[] BitVector;
        MainDir.clear();
    }
    BitVector = new int[BitVectorSize];
    OpenFileDescriptors = new FileDescriptor * [BitVectorSize];
    for (int i = 0; i < BitVectorSize; i++) {
        BitVector[i] = 0;
        OpenFileDescriptors[i] = nullptr;
    }
    is_formated = true;
}

//Receives a file name, creates it and returns its FD. Returns '-1' if the disk is not formatted or there's no room
int fsDisk::CreateFile(string fileName) {
    if (is_formated && freeBlocks > 0 && (int) MainDir.size() < BitVectorSize) {
        FsFile * tmpFs = new FsFile(block_size);
        FileDescriptor * tmpFD = new FileDescriptor(fileName, tmpFs);
        int FdNum = availableFD();
        if (FdNum != -1) {
            MainDir.emplace_back(tmpFD);
            OpenFileDescriptors[FdNum] = tmpFD;
            return FdNum;
        }
    }
    return -1;
}

//Returns the index of the first available FD number and '-1' id there is non
int fsDisk::availableFD() {
    int i = 0;
    while (i < BitVectorSize && OpenFileDescriptors[i++]);
    return (i <= BitVectorSize) ? (i - 1) : -1;
}

//Returns the index of the first available block, and '-1' if there's non
int fsDisk::availableBLock() {
    for (int i = 0; i < BitVectorSize; i++) {
        if (BitVector[i] == 0) {
            BitVector[i] = 1; //The block is now in use
            return i;
        }
    }
    return -1;
}

//Receives a file name, if it's not already open - opens and returns its FD. If it is open - returns '-1'
int fsDisk::OpenFile(string fileName) {
    if (is_formated) {
        for (auto & i: MainDir) {
            if (i -> getFileName() == fileName) {
                if (i -> isInUse()) {
                    return -1;
                } else {
                    int FDnum = availableFD();
                    i -> setInUse(true);
                    OpenFileDescriptors[FDnum] = i;
                    return FDnum;
                }
            }
        }
    }
    return -1;
}

//Returns the index of fileName in MainDir if exists and '-1' if not
int fsDisk::findFileByName(string fileName) {
    for (int i = 0; i < (int) MainDir.size(); i++) {
        if (MainDir[i] -> getFileName() == fileName)
            return i;
    }
    return -1;
}

//Closes the file who's fd is received and returns the corresponding file name
string fsDisk::CloseFile(int fd) {
    if (is_formated && OpenFileDescriptors[fd]) {
        string fileName = OpenFileDescriptors[fd] -> getFileName();
        OpenFileDescriptors[fd] = nullptr;
        MainDir[findFileByName(fileName)] -> setInUse(false);
        return fileName;
    }
    return "-1";
}

//Writes a buffer of length len to the file with the file descriptor fd
int fsDisk::WriteToFile(int fd, char * buf, int len) {
    FileDescriptor * currFD = OpenFileDescriptors[fd];
    if (is_formated && len <= availableSize && currFD && currFD -> getFsFile() -> getfile_size() + len <= maxSize) {
        FsFile * currFS = currFD -> getFsFile();
        if (currFS -> getIndexBlock() == -1) {
            int indexBlock = availableBLock();
            if (indexBlock != -1) {
                currFS -> setIndexBlock(indexBlock);
                freeBlocks--;
                availableSize -= block_size;
            } else
                return -1;
        }
        int emptyAllocatedSpace = block_size * currFS -> getBlockInUse() - currFS -> getfile_size();
        int written = WriteToFileRec(fd, buf, len, emptyAllocatedSpace, currFS);
        if (written == len) {
            currFS -> updateFileSize(len);
            return written;
        }
    }
    return -1;
}

//Recursively writes the data to the file
int fsDisk::WriteToFileRec(int fd, char * buf, int len, int emptyAllocatedSpace, FsFile * currFS) {
    if (len <= emptyAllocatedSpace) { //If the existing space is enough
        if (fseek(sim_disk_fd, endOfFileIndex(currFS, emptyAllocatedSpace), SEEK_SET) != 0)
            return -1;
        if ((int) fwrite(buf, len, 1, sim_disk_fd) < 1 && len != 0)
            return -1;
        availableSize -= len;
        return len;
    } else if (emptyAllocatedSpace > 0) { //If there's space but it's not enough
        if (fseek(sim_disk_fd, endOfFileIndex(currFS, emptyAllocatedSpace), SEEK_SET) != 0)
            return -1;
        if (fwrite(buf, emptyAllocatedSpace, 1, sim_disk_fd) < 1)
            return -1;
        availableSize -= emptyAllocatedSpace;
        return emptyAllocatedSpace +
        WriteToFileRec(fd, buf += emptyAllocatedSpace, len - emptyAllocatedSpace, 0, currFS);
    } else { //New block needs to be allocated
        int newBlockInt = availableBLock();
        if (newBlockInt == -1)
            return -1;
        int indexBlock = currFS -> getIndexBlock();
        int blockInUse = currFS -> getBlockInUse();
        if (fseek(sim_disk_fd, indexBlock * block_size + blockInUse, SEEK_SET) != 0)
            return -1;
        char newBlockIndex = 0;
        decToBinary(newBlockInt, newBlockIndex);
        if (fwrite( & newBlockIndex, 1, 1, sim_disk_fd) < 1)
            return -1;
        currFS -> setBlockInUse(++blockInUse);
        freeBlocks--;
        int sizeToWrite = (block_size < len) ? block_size : len;
        if (fseek(sim_disk_fd, (int) newBlockIndex * block_size, SEEK_SET) != 0)
            return -1;
        if (fwrite(buf, sizeToWrite, 1, sim_disk_fd) < 1)
            return -1;
        availableSize -= sizeToWrite;
        return sizeToWrite +
        WriteToFileRec(fd, buf += sizeToWrite, len - sizeToWrite, block_size - sizeToWrite, currFS);
    }
}

//Returns the index of the first empty slot of a file
int fsDisk::endOfFileIndex(FsFile * currFS, int emptyAllocatedSpace) {
    int indexBlock = currFS -> getIndexBlock();
    int lastBlockNumber = indexBlock * block_size + currFS -> getBlockInUse() - 1;
    if (fseek(sim_disk_fd, lastBlockNumber, SEEK_SET) != 0) //sets the cursor at the last block in the index block
        return -1;
    char blockNumber;
    if (fread( & blockNumber, 1, 1, sim_disk_fd) < 1)
        return -1;
    int lastBlockOffset = block_size - emptyAllocatedSpace;
    return (int) blockNumber * block_size + lastBlockOffset;
}

//Reads len bytes from file fd to buffer buf
int fsDisk::ReadFromFile(int fd, char * buf, int len) {
    buf[0] = '\0'; //Avoid the main from printing the last buffer value in case of failure
    FileDescriptor * currFD = OpenFileDescriptors[fd];
    if (is_formated && currFD && len <= currFD -> getFsFile() -> getfile_size()) {
        FsFile * currFS = currFD -> getFsFile();
        int numOfBlocksToReadFrom = (len + block_size - 1) / block_size; //block used
        int sizeToRead = (block_size < len) ? block_size : len; //how many to read from this block
        char currBlock;
        for (int i = 0; i < numOfBlocksToReadFrom; i++) {
            if (fseek(sim_disk_fd, block_size * currFS -> getIndexBlock() + i, SEEK_SET) != 0) //Finds data block number
                return -1;
            if (fread( & currBlock, 1, 1, sim_disk_fd) < 1)
                return -1;
            if (fseek(sim_disk_fd, block_size * (int) currBlock, SEEK_SET) != 0)
                return -1;
            if (fread(buf, sizeToRead, 1, sim_disk_fd) < 1)
                return -1;
            buf += sizeToRead;
            len -= sizeToRead;
            sizeToRead = (block_size < len) ? block_size : len;
        }
        strncpy(buf, "\0", 1);
        return len;
    }
    return -1;
}

//Deletes 'FileName' and all of its data, including the FsFile
int fsDisk::DelFile(string FileName) {
    if (is_formated) {
        int fd = findFileByName(FileName);
        if (fd != -1) {
            FileDescriptor * currFD = MainDir[fd];
            FsFile * currFS = currFD -> getFsFile();
            int indexBlock = currFS -> getIndexBlock();
            if (indexBlock != -1) {
                if (DelDiskData(currFS -> getIndexBlock(), currFS) == 0) {
                    if (currFD -> isInUse())
                        OpenFileDescriptors[fd] = nullptr;
                    BitVector[currFS -> getIndexBlock()] = 0;
                    freeBlocks += 1 + currFS -> getBlockInUse();
                    availableSize += currFS -> getfile_size() + block_size;
                }
            }
            delete currFD -> getFsFile();
            delete MainDir[fd];
            MainDir.erase(MainDir.begin() + fd);
            return 1;
        }
    }
    return -1;
}

//Receives an index block number and deletes all of its related data from the disk
int fsDisk::DelDiskData(int indexBlock, FsFile * currFS) {
    char currBlock;
    int fileSize = currFS -> getfile_size();
    int sizeToWrite = (block_size < fileSize) ? block_size : fileSize;
    int numOfBlocksToReadFrom = currFS -> getBlockInUse();
    for (int i = 0; i < numOfBlocksToReadFrom; i++) {
        if (fseek(sim_disk_fd, block_size * indexBlock + i, SEEK_SET) != 0)
            return -1;
        if (fread( & currBlock, 1, 1, sim_disk_fd) < 1)
            return -1;
        if (fseek(sim_disk_fd, block_size * indexBlock + i, SEEK_SET) != 0)
            return -1;
        if (fwrite("\0", 1, 1, sim_disk_fd) < 1)
            return -1;
        if (fseek(sim_disk_fd, block_size * (int) currBlock, SEEK_SET) != 0)
            return -1;
        for (int j = 0; j < sizeToWrite; j++) {
            if (fwrite("\0", 1, 1, sim_disk_fd) < 1)
                return -1;
        }
        BitVector[(int)(unsigned) currBlock] = 0;
        fileSize -= sizeToWrite;
        sizeToWrite = (block_size < fileSize) ? block_size : fileSize;
    }
    return 0;
}

void fsDisk::decToBinary(int n, char & c) {
    // array to store binary number
    int binaryNum[8];
    // counter for binary array
    int i = 0;
    while (n > 0) {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--) {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;
    int written;

    fsDisk * fs = new fsDisk();
    int cmd_;
    while (true) {
        cin >> cmd_;
        switch (cmd_) {
            case 0: // exit
            delete fs;
            exit(0);

            case 1: // list-file
            fs -> listAll();
            break;

            case 2: // format
            cin >> blockSize;
            fs -> fsFormat(blockSize);
            break;

            case 3: // creat-file
            cin >> fileName;
            _fd = fs -> CreateFile(fileName);
            cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

            case 4: // open-file
            cin >> fileName;
            _fd = fs -> OpenFile(fileName);
            cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

            case 5: // close-file
            cin >> _fd;
            fileName = fs -> CloseFile(_fd);
            cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

            case 6: // write-file
            cin >> _fd;
            cin >> str_to_write;
            written = fs -> WriteToFile(_fd, str_to_write, (int) strlen(str_to_write));
            cout << "Writed: " << written << " Char's into File Descriptor #:" << _fd << endl;
            break;

            case 7: // read-file
            cin >> _fd;
            cin >> size_to_read;
            fs -> ReadFromFile(_fd, str_to_read, size_to_read);
            cout << "ReadFromFile: " << str_to_read << endl;
            break;

            case 8: // delete file
            cin >> fileName;
            _fd = fs -> DelFile(fileName);
            cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;
            default:
                break;
        }
    }

}
