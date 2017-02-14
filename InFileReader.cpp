#include "InFileReader.hpp"

namespace INFILE
{
    void readInFile(std::string& FileName, bool Verbose)
    {
        // Open file
        std::ifstream InFile(FileName);

        if(!InFile)
        {
            std::cout << "EPlacer: Invalid File Name.\n";
            exit(0);
        }

        // Start by setting the number of blocks, grid size.
        uint32_t NumBlocks, NumNets, NumRows, NumCols;
        InFile >> NumBlocks >> NumNets >> NumRows >> NumCols;
        PLACER::setNumberOfBlocks(NumBlocks, Verbose);
        PLACER::setPlaceSize(NumRows, NumCols, Verbose);

        // Now we should initialize the connections.
        if (Verbose)
            std::cout << "\tEPlacer: Will read " << NumNets << " nets.\n";

        uint32_t NumConnections, First, Second;
        for (uint32_t i = 0; i < NumNets; i++)
        {
            InFile >> NumConnections;
            InFile >> First;
            for (uint32_t j = 0; j < NumConnections-1; j++)
            {
                InFile >> Second;
                PLACER::connectBlocks(First, Second, Verbose);
                First = Second;
            }
        }
    }
}
