#ifndef PLACER_HPP
#define PLACER_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <utility>
#include <cstdlib>
#include <math.h>

#include <pthread.h>
#include <unistd.h>

#include "Tools.hpp"
#include "Layout.hpp"

namespace LAYOUT
{
    class LayoutWidget;
}

namespace PLACER
{
    // I usually hate typedefs, I really do, but some of those became really
    // long, I had to. God, forgive me!
    typedef std::vector<std::vector<uint32_t>> GridType;
    typedef std::vector<std::vector<uint32_t>> ConnectionsType;
    typedef std::vector<std::pair<uint32_t, uint32_t>> LocationsType;

    // The grid elements are filled by ints reprsenting what is in there.
    // 0 means this location is empty, other numbers are block IDs.
    // By default this vector is initalized to 0.
    // I only use vectors because they're easy, but it is in fact a 2D array.
    extern GridType Grid;

    // every entry in this vector contains a vector of the node IDs
    // it's connected to.
    // Unlike the data structure above, this is not a 2D array. This lengths
    // are variable.
    extern ConnectionsType Blocks;

    // This is a locations vector, contains the locations of every block
    // in the grid. It is only used when drawing the nets or calculating
    // the cost. It is quicker than doing search every time we need to
    // do those tasks.
    extern LocationsType Locations;
    extern LocationsType EmptyLocations;

    // This a work queue for the worker threads. Each entry of this queue
    // contains: a vector of move candidates and a vector of blank candidates
    // in case of a sparse placement, or a vector of swap candidates and a
    // vector of victim swap candidates.
    // The size of those vector depend on the number of threads.
    extern TOOLS::WorkQueue<std::tuple<std::vector<uint32_t>,std::vector<uint32_t>,uint32_t>>* WorkQueue;

    extern uint32_t NumThreads;

    extern LAYOUT::LayoutWidget* MainWindow;
    extern bool Verbose;

    extern pthread_t *Workers;
    extern pthread_mutex_t Mutex;

    // Our move is different if the placement is sparse or not.
    extern bool Sparse;

    void setNumberOfBlocks(uint32_t Num, bool Verbose);
    void setPlaceSize(uint32_t Rows, uint32_t Cols, bool Verbose);
    void connectBlocks(uint32_t First, uint32_t Second, bool Verbose);
    void initPlacement(uint32_t NThreads, LAYOUT::LayoutWidget *Window, bool BeVerbose);

    std::pair<uint32_t, uint32_t> getGridSize();
    uint32_t getGridElement(uint32_t X, uint32_t Y);
    std::vector<uint32_t> getConnectionsSize();
    uint32_t getConnection(uint32_t X, uint32_t Y);
    std::pair<uint32_t, uint32_t> getLocation(uint32_t X);

    // Those are the actual simulated annealing stuff
    // We use a producer consumer model, the main thread picks non-conflicting
    // blocks to be removed, pushed those to a work queue, then we have consumer
    // worker threads that do the actual moving, cost function, accepting or
    // regecting the move.
    // the number of threads selected by the user is actually the number of
    // worker threads.
    void pickMoveCandidates(uint32_t Temperature);
    void *move(void* ThreadID);
    void simulateAnnealing();

    // This is a simple cost function, just calculates distance between two points.
    uint32_t distanceCostFunction(uint32_t First, uint32_t Second);
}

#endif // PLACER_HPP
