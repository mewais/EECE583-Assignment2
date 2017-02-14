#ifndef INFILE_READER_HPP
#define INFILE_READER_HPP

#include <fstream>
#include <iostream>

#include <string>
#include <vector>
#include <utility>
#include <tuple>

#include "Placer.hpp"

#define IN_BLANK -1

namespace INFILE
{
    void readInFile(std::string& FileName, bool Verbose);
}

#endif // INFILE_READER_HPP
