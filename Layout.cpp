#include "Layout.hpp"

namespace LAYOUT
{
    void *initPlacement(void *PlaceHolder)
    {
        // This is a stupid intermediate function needed for pthreads.
        uint32_t NumThreads = std::get<0>(*(std::tuple<uint32_t, LAYOUT::LayoutWidget *, bool>*)PlaceHolder);
        LAYOUT::LayoutWidget *GUI = std::get<1>(*(std::tuple<uint32_t, LAYOUT::LayoutWidget *, bool>*)PlaceHolder);
        bool Verbose = std::get<2>(*(std::tuple<uint32_t, LAYOUT::LayoutWidget *, bool>*)PlaceHolder);
        if (Verbose)
            std::cout << "EPlacer: Initializing placement.\n";
        PLACER::initPlacement(NumThreads, GUI, Verbose);
        PLACER::simulateAnnealing();
        return NULL;                // disable stupid gcc warning
    }

    LayoutWidget::LayoutWidget(uint32_t ThreadCount, bool ShowNets, bool Verbose) : Verbose(Verbose), ShowNets(ShowNets)
    {
        // Show the window and adjust the size to the size of layout
        // The height depends on the number of rows and channel spaces,
        // The width depends on the number of cols only.
        if (Verbose)
            std::cout << "EPlacer: Starting GUI.\n";
        this->resize(std::get<0>(PLACER::getGridSize())*PixelsPerGridBlock + PixelsPerChannelWidth,
                    std::get<1>(PLACER::getGridSize())*(PixelsPerGridBlock + PixelsPerChannelWidth));
        this->show();

        // Now we have initialized the GUI, we can now start the router
        pthread_t PlaceThread;
        std::tuple<uint32_t, LAYOUT::LayoutWidget *, bool>* Params;
        Params = new std::tuple<uint32_t, LAYOUT::LayoutWidget *, bool>;
        *Params = std::make_tuple(ThreadCount, this, Verbose);
        pthread_create(&PlaceThread, NULL, initPlacement, (void *)(Params));
    }

    void LayoutWidget::paintEvent(QPaintEvent *event)
    {
        // Create a Paint Area and Painter
        QPainter GridPainter(this);
        GridPainter.setPen(Qt::black);

        // draw vertical and horizontal lines that resemble the rows, cols
        // and channels.
        for (uint32_t i = 0; i < std::get<0>(PLACER::getGridSize())*(PixelsPerGridBlock +
        PixelsPerChannelWidth); i+=(PixelsPerGridBlock + PixelsPerChannelWidth))
        {
            // Those two will draw a row and leave a channel router.
            GridPainter.drawLine(0, i, std::get<1>(PLACER::getGridSize())*PixelsPerGridBlock, i);
            GridPainter.drawLine(0, i+PixelsPerGridBlock, std::get<1>(PLACER::getGridSize())*PixelsPerGridBlock, i+PixelsPerGridBlock);

            // This one here will draw the blocks withing the rows
            for (uint32_t j = 0; j <= std::get<1>(PLACER::getGridSize())*PixelsPerGridBlock; j+=PixelsPerGridBlock)
            {
                GridPainter.drawLine(j, i, j, i+PixelsPerGridBlock);
            }
        }

        // Now give each block a number, Nothing means empty, anything else is an
        // actual block's ID.
        for (uint32_t i = 0; i < std::get<0>(PLACER::getGridSize()); i++)
        {
            for (uint32_t j = 0; j < std::get<1>(PLACER::getGridSize()); j++)
            {
                if(PLACER::getGridElement(i, j))
                {
                    // The offsets I got by trial and error.
                    // Do NOT change.
                    GridPainter.drawText((j+0.3)*PixelsPerGridBlock, (i+0.45)*(PixelsPerGridBlock+PixelsPerChannelWidth), std::to_string(PLACER::getGridElement(i, j)).c_str());
                }
            }
        }

        // Now the only thing left is to show Nets too.
        for (uint32_t i = 0; i < PLACER::getConnectionsSize().size(); i++)
        {
            for (uint32_t j = 0; j < PLACER::getConnectionsSize()[i]; j++)
            {
                // We should be looking for the one we're connected to,
                // but searching for it everytime is stupid, so we use
                // a location array instead.
                std::pair<uint32_t, uint32_t> FirstLocation, SecondLocation;

                FirstLocation = PLACER::getLocation(i);
                SecondLocation = PLACER::getLocation(PLACER::getConnection(i,j));

                // if(Verbose)
                // {
                //     std::cout << "EPlacer: Connecting nodes: " << i+1 << ", " <<
                //     PLACER::getConnection(i,j)+1 << "\n";
                //     std::cout << "\tLocations = " << std::get<0>(FirstLocation)
                //     << ", " << std::get<1>(FirstLocation) << ", " <<
                //     std::get<0>(SecondLocation) << ", " << std::get<1>(SecondLocation)
                //     << "\n";
                //     std::cout << "\t\tLocations = " << std::get<0>(FirstLocation)*(PixelsPerGridBlock+PixelsPerChannelWidth)
                //     << ", " << std::get<1>(FirstLocation)*PixelsPerGridBlock << ", " <<
                //     std::get<0>(SecondLocation)*(PixelsPerGridBlock+PixelsPerChannelWidth) << ", " <<
                //     std::get<1>(SecondLocation)*PixelsPerGridBlock << "\n";
                // }

                // we will just connect blocks by their origins.
                GridPainter.drawLine(std::get<1>(FirstLocation)*PixelsPerGridBlock,
                    std::get<0>(FirstLocation)*(PixelsPerGridBlock+PixelsPerChannelWidth),
                    std::get<1>(SecondLocation)*PixelsPerGridBlock,
                    std::get<0>(SecondLocation)*(PixelsPerGridBlock+PixelsPerChannelWidth));
            }
        }
    }
}
