#include "Placer.hpp"

namespace PLACER
{

    GridType Grid;

    ConnectionsType Blocks;

    LocationsType Locations;
    LocationsType EmptyLocations;

    // TOOLS::WorkQueue<std::tuple<std::vector<uint32_t>,std::vector<uint32_t>, uint32_t>>* WorkQueue;

    uint32_t NumThreads;

    LAYOUT::LayoutWidget *MainWindow;
    bool Verbose;

    // pthread_t *Workers;
    // pthread_mutex_t Mutex;
    // pthread_mutex_t PrintMutex;

    bool Sparse;

    void setNumberOfBlocks(uint32_t Num, bool Verbose)
    {
        Blocks.resize(Num);
        Locations.resize(Num);
        if (Verbose)
            std::cout << "\tEPlacer: Initializing " << Num << " blocks";
    }

    void setPlaceSize(uint32_t Rows, uint32_t Cols, bool Verbose)
    {
        Grid.resize(Rows);
        for (uint32_t i = 0; i < Grid.size(); i++)
        {
            Grid[i].resize(Cols);
        }
        if(Verbose)
            std::cout << "\tEPlacer: Created grid of size (" << Rows << "," << Cols << ").\n";
    }

    void connectBlocks(uint32_t First, uint32_t Second, bool Verbose)
    {
        Blocks[First].push_back(Second);
        Blocks[Second].push_back(First);

        if(Verbose)
            std::cout << "\tEPlacer: Connecteing Blocks " << First << " and " << Second << ".\n";
    }

    void initPlacement(uint32_t NThreads, LAYOUT::LayoutWidget *Window, bool BeVerbose)
    {
        MainWindow = Window;
        Verbose = BeVerbose;

        // This is the same Grid as above, rearranged in 1D instead of 2D.
        // Helps for shuffling.
        std::vector<uint32_t> _1DGrid;
        _1DGrid.resize(Grid.size()*Grid[0].size());

        // Fill this 1D Grid with all the blocks, in order, doesn't matter.
        for(uint32_t i = 1; i <= Blocks.size(); i++)
        {
            _1DGrid[i-1] = i;
        }

        // Place every block in a random place in the grid.
        // I'm too lazy to do this myself, so this is a standard way to
        // shuffle a vector.
        // The default_random_engine produces the same output everytime the
        // placer is called, needs a seed. produced by random_device
        std::random_device seed;
        std::default_random_engine Engine(seed());
        std::shuffle(std::begin(_1DGrid), std::end(_1DGrid), Engine);

        // Now fill the 2D grid from the 1D grid.
        uint32_t RowID;
        uint32_t ColID;
        for(uint32_t i = 0; i < _1DGrid.size(); i++)
        {
            ColID = i % Grid[0].size();
            RowID = i / Grid[0].size();
            Grid[RowID][ColID] = _1DGrid[i];

            if (_1DGrid[i])
            {
                Locations[_1DGrid[i]-1] = std::make_pair(RowID, ColID);
            }
            else
            {
                EmptyLocations.push_back(std::make_pair(RowID, ColID));
            }

            if(Verbose)
                std::cout << "\tEPlacer: Gird Location " << RowID << ", " << ColID
                << " has element " << Grid[RowID][ColID] << "\n";
        }

        // Can't have more threads than the number of blocks!!
        // Divide by four to give them room for picking candidates
        // and targets.
        // NumThreads = NThreads>Blocks.size()? Blocks.size()/4:NThreads;
        NumThreads = 1;

        // Is it sparse?
        // The logic behind this is, if the number of blank locations is less
        // than the number of threads, then not all threads will be able to
        // move stuff, they have to swap. Otherwise, threads can simply move
        // blocks.
        Sparse = (_1DGrid.size()-(Blocks.size()/_1DGrid.size())) > NumThreads? true:false;

        // // Create Worker Threads
        // if(Verbose)
        //     std::cout << "EPlacer: Creating " << NumThreads << " Worker Threads.\n";
        //     // Size the queue
        // WorkQueue = new TOOLS::WorkQueue<std::tuple<std::vector<uint32_t>,std::vector<uint32_t>,uint32_t>>(NumThreads);
        // // WorkQueue->resize(10);
        // uint32_t *ID = new uint32_t;
        // Workers = new pthread_t[NumThreads];
        // for(uint32_t i = 0; i < NumThreads; i++)
        // {
        //     ID = new uint32_t;
        //     *ID = i;
        //     if(Verbose)
        //         std::cout << "\tEPlacer: Creating thread " << (*ID) << ".\n";
        //     pthread_create(&Workers[i], NULL, move, (void*)ID);
        // }
        // pthread_mutex_init(&Mutex, NULL);
        // pthread_mutex_init(&PrintMutex, NULL);
    }

    std::pair<uint32_t, uint32_t> getGridSize()
    {
        return std::make_pair(Grid.size(), Grid[0].size());
    }

    uint32_t getGridElement(uint32_t X, uint32_t Y)
    {
        return Grid[X][Y];
    }

    std::vector<uint32_t> getConnectionsSize()
    {
        std::vector<uint32_t> Sizes;
        for(uint32_t i = 0; i < Blocks.size(); i++)
        {
            Sizes.push_back(Blocks[i].size());
        }
        return Sizes;
    }

    uint32_t getConnection(uint32_t X, uint32_t Y)
    {
        return Blocks[X][Y];
    }

    std::pair<uint32_t, uint32_t> getLocation(uint32_t X)
    {
        return Locations[X];
    }

    // Simulated Annealing
    // TODO: make this faster by reducing the random search space instead of
    // "retrying"!!
    // void pickMoveCandidates(uint32_t Temperature)
    std::tuple<uint32_t, uint32_t, uint32_t> pickMoveCandidates(uint32_t Temperature)
    {
        // Whether sparse or not, select move candidates. Keep selecting until
        // you end up with NumThreads candidates that are "unrelated".
        // Unrelated means they are not the same and are not connected.
        std::vector<uint32_t> Candidates;
        LocationsType CandidateLocations;

        uint32_t Choice;
        while (Candidates.size() < NumThreads)
        {
            // Pick random element
            // Again, a standard way because I'm too lazy.
            std::random_device Seed;
            std::mt19937 Engine(Seed());
            std::uniform_int_distribution<uint32_t> Choose(0, Blocks.size()-1);
            Choice = Choose(Engine);

            // If this block is already picked, look again.
            if (std::find(Candidates.begin(), Candidates.end(), Choice) != Candidates.end())
                continue;

            // If the block is connected to another picked, look again.
            bool LookAgain = false;
            for (uint32_t i = 0; i < Candidates.size(); i++)
            {
                if (std::find(Blocks[Candidates[i]].begin(), Blocks[Candidates[i]].end(), Choice) != Blocks[Candidates[i]].end())
                {
                    LookAgain = true;
                    break;
                }
                if (std::find(Blocks[Choice].begin(), Blocks[Choice].end(), Candidates[i]) != Blocks[Choice].end())
                {
                    LookAgain = true;
                    break;
                }
            }
            if (LookAgain)
            {
                continue;
            }

            // When we reach here, then the choice doesn't conflict with anything
            // we can now add it to the queue.
            Candidates.push_back(Choice);
            CandidateLocations.push_back(Locations[Choice]);
        }

        if(Verbose)
        {
            // pthread_mutex_lock(&PrintMutex);
            std::cout << "EPlacer: Picked move candidates.\n";
            // pthread_mutex_unlock(&PrintMutex);
        }
        std::vector<uint32_t> Targets;
        LocationsType TargetLocations;
        std::pair<uint32_t, uint32_t> TargetChoice;
        if (Sparse)
        {
            while (TargetLocations.size() < NumThreads)
            {
                // Just pick a free location to move too, make sure it's not selected
                // by anyone else.
                // Pick random element
                // Again, a standard way because I'm too lazy.
                std::random_device Seed;
                std::mt19937 Engine(Seed());
                std::uniform_int_distribution<uint32_t> Choose(0, EmptyLocations.size()-1);
                Choice = Choose(Engine);
                TargetChoice = EmptyLocations[Choice];

                // If this block is already picked, look again.
                if (std::find(TargetLocations.begin(), TargetLocations.end(), TargetChoice) != TargetLocations.end())
                    continue;

                // When we reach here, then the choice doesn't conflict with anything
                // we can now add it to the queue.
                Targets.push_back(Choice);
                TargetLocations.push_back(TargetChoice);
            }
        }
        else
        {
            while (Targets.size() < NumThreads)
            {
                // Pick random element
                // Again, a standard way because I'm too lazy.
                std::random_device Seed;
                std::mt19937 Engine(Seed());
                std::uniform_int_distribution<uint32_t> Choose(0, Blocks.size()-1);
                Choice = Choose(Engine);

                // If this block is already picked, look again.
                if (std::find(Candidates.begin(), Candidates.end(), Choice) != Candidates.end())
                    continue;
                if (std::find(Targets.begin(), Targets.end(), Choice) != Targets.end())
                    continue;

                // If the block is connected to another picked, look again.
                bool LookAgain = false;
                for (uint32_t i = 0; i < Candidates.size(); i++)
                {
                    if (std::find(Blocks[Candidates[i]].begin(), Blocks[Candidates[i]].end(), Choice) != Blocks[Candidates[i]].end())
                    {
                        LookAgain = true;
                        break;
                    }
                }
                if (LookAgain)
                {
                    continue;
                }
                for (uint32_t i = 0; i < Targets.size(); i++)
                {
                    if (std::find(Blocks[Targets[i]].begin(), Blocks[Targets[i]].end(), Choice) != Blocks[Targets[i]].end())
                    {
                        LookAgain = true;
                        break;
                    }
                }
                if (LookAgain)
                {
                    continue;
                }

                // When we reach here, then the choice doesn't conflict with anything
                // we can now add it to the queue.
                Targets.push_back(Choice);
                TargetLocations.push_back(Locations[Choice]);
            }
        }

        if(Verbose)
        {
            // pthread_mutex_lock(&PrintMutex);
            std::cout << "EPlacer: Picked victim candidates.\n";
            // pthread_mutex_unlock(&PrintMutex);
        }

        // WorkQueue->push(std::make_tuple(Candidates, Targets, Temperature));

        if(Verbose)
        {
            // pthread_mutex_lock(&PrintMutex);
            std::cout << "EPlacer: Pushed move to queue.\n";
            std::cout << "EPlacer: Picked Block " << Candidates[0] << ", Empty Target " << Targets[0] << "\n";
            // std::cout << "EPlacer: Size 1: " << Candidates.size() << ", Size 2: "
            //     << Targets.size() << "\n";
            // pthread_mutex_unlock(&PrintMutex);
        }
        return std::make_tuple(Candidates[0], Targets[0], Temperature);
    }

    // void *move(void* ThreadID)
    void move(std::tuple<uint32_t, uint32_t, uint32_t> PossibleMove)
    {
        uint32_t TID;
        // TID = *(uint32_t *)ThreadID;
        TID = 0;

        if (Verbose)
        {
            // pthread_mutex_lock(&PrintMutex);
            std::cout << "Worker thread " << TID << " created.\n";
            // pthread_mutex_unlock(&PrintMutex);
        }

        // while (true)
        // {
            // See if the queue has a new move for us!
            // if (Verbose)
            // {
            //     // pthread_mutex_lock(&PrintMutex);
            //     std::cout << "Worker thread " << TID << " trying to peak.\n";
            //     // pthread_mutex_unlock(&PrintMutex);
            // }
            // std::tuple<std::vector<uint32_t>,std::vector<uint32_t>,uint32_t> PossibleMove = WorkQueue->peak(TID);
            // if (Verbose)
            // {
            //     // pthread_mutex_lock(&PrintMutex);
            //     std::cout << "Worker thread " << TID << " received move.\n";
            //     // pthread_mutex_unlock(&PrintMutex);
            // }

            // Now that we have a potential move, we should try and check the cost.
            uint32_t CurrentCost = 0;
            uint32_t MoveCost = 0;
            // pthread_mutex_lock(&PrintMutex);
            // std::cout << "Size 1: " << std::get<0>(PossibleMove).size() << ", Size 2: "
            //     << std::get<1>(PossibleMove).size() << ", ID = " << TID << "\n";
            // pthread_mutex_unlock(&PrintMutex);

            for (uint32_t i = 0; i < Blocks[std::get<0>(PossibleMove)].size(); i++)
            {
                CurrentCost += distanceCostFunction(std::get<0>(PossibleMove),
                    Blocks[std::get<0>(PossibleMove)][i]);
            }
            if (!Sparse)
            {
                for (uint32_t i = 0; i < Blocks[std::get<1>(PossibleMove)].size(); i++)
                {
                    CurrentCost += distanceCostFunction(std::get<1>(PossibleMove),
                        Blocks[std::get<1>(PossibleMove)][i]);
                }
            }

            for (uint32_t i = 0; i < Blocks[std::get<0>(PossibleMove)].size(); i++)
            {
                MoveCost += distanceCostFunction(std::get<1>(PossibleMove),
                    Blocks[std::get<0>(PossibleMove)][i]);
            }
            if (!Sparse)
            {
                for (uint32_t i = 0; i < Blocks[std::get<1>(PossibleMove)].size(); i++)
                {
                    MoveCost += distanceCostFunction(std::get<0>(PossibleMove),
                        Blocks[std::get<1>(PossibleMove)][i]);
                }
            }

            if (MoveCost < CurrentCost)
            {
                // Move is accepted.
                uint32_t Temperature = std::get<2>(PossibleMove);
                // Pick random number, see if it is less than the limit.
                std::random_device Seed;
                std::mt19937 Engine(Seed());
                std::uniform_real_distribution<double> Choose(0, 1);
                if (Choose(Engine) < std::exp(-std::abs(MoveCost - CurrentCost)/Temperature))
                {
                    // Take move! If the placement si sparse, we move the block's
                    // location to the EmptyLocations array, and we move the blank
                    // location to the Locations array. Otherwise we swap the two
                    // locations in the Locations array.
                    // Don't forget to change the Grid too, for thr GUI.
                    if (Sparse)
                    {
                        // pthread_mutex_lock(&Mutex);
                        std::pair<uint32_t, uint32_t> TempLoc;
                        TempLoc = Locations[std::get<0>(PossibleMove)];
                        // std::cout << "Moving block " << std::get<0>(PossibleMove) <<
                        // " from location " << std::get<0>(Locations[std::get<0>(PossibleMove)]) << ", " << std::get<1>(Locations[std::get<0>(PossibleMove)]) <<
                        // " to location " << std::get<0>(EmptyLocations[std::get<1>(PossibleMove)]) << ", " << std::get<1>(EmptyLocations[std::get<1>(PossibleMove)]) << "\n";
                        Locations[std::get<0>(PossibleMove)] = EmptyLocations[std::get<1>(PossibleMove)];
                        EmptyLocations[std::get<1>(PossibleMove)] = TempLoc;
                        Grid[std::get<0>(TempLoc)][std::get<1>(TempLoc)] = 0;
                        Grid[std::get<0>(Locations[std::get<0>(PossibleMove)])]
                            [std::get<1>(Locations[std::get<0>(PossibleMove)])]
                            = std::get<0>(PossibleMove)+1;
                        if (MainWindow)
                        {
                            MainWindow->update();
                            usleep(2000);
                            // std::cin.ignore();
                        }
                        // pthread_mutex_unlock(&Mutex);
                    }
                    else
                    {
                        // pthread_mutex_lock(&Mutex);
                        std::pair<uint32_t, uint32_t> TempLoc;
                        TempLoc = Locations[std::get<0>(PossibleMove)];
                        Locations[std::get<0>(PossibleMove)] = Locations[std::get<1>(PossibleMove)];
                        Locations[std::get<1>(PossibleMove)] = TempLoc;
                        Grid[std::get<0>(TempLoc)][std::get<1>(TempLoc)] = std::get<1>(PossibleMove)+1;
                        Grid[std::get<0>(Locations[std::get<0>(PossibleMove)])]
                            [std::get<1>(Locations[std::get<0>(PossibleMove)])]
                            = std::get<0>(PossibleMove)+1;
                        if (MainWindow)
                        {
                            MainWindow->update();
                            usleep(2000);
                            // std::cin.ignore();
                        }
                        // pthread_mutex_unlock(&Mutex);
                    }
                }
            }
        // }
        // return NULL;                // disable stupid gcc warning
    }

    void simulateAnnealing()
    {
        // Worker threads are already launched by now.
        if (Verbose)
            std::cout << "Starting Annealing.\n";
        for (uint32_t Temperature = 1000; Temperature > 1; Temperature *= 0.9)
        {
            for (uint32_t NumIters = 0; NumIters < 10*std::pow(Blocks.size(),4/3); NumIters++)
            {
                if(Verbose)
                {
                    // pthread_mutex_lock(&PrintMutex);
                    std::cout << "EPlacer: Temperature " << Temperature << ", Iter " << NumIters << "\n";
                    // pthread_mutex_unlock(&PrintMutex);
                }
                move(pickMoveCandidates(Temperature));
            }
        }

        // for(uint32_t i = 0; i < NumThreads; i++)
        // {
        //     if(Verbose)
        //         std::cout << "\tEPlacer: exiting thread " << i << ".\n";
        //     pthread_join(Workers[i], NULL);
        // }
    }

    uint32_t distanceCostFunction(uint32_t First, uint32_t Second)
    {
        uint32_t XDistance = std::abs(std::get<0>(Locations[First]) - std::get<0>(Locations[Second]));
        uint32_t YDistance = std::abs(std::get<1>(Locations[First]) - std::get<1>(Locations[Second]));
        return (XDistance + YDistance);
    }
}
