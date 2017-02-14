#include <iostream>
#include <string>
#include <tuple>

#include <boost/program_options.hpp>

#include <QtWidgets/QApplication>

#include "Layout.hpp"
#include "InFileReader.hpp"

// LAYOUT::LayoutWidget *MainWindow;

int main(int argc, char** argv)
{
    bool UseGUI = false;
    bool UseGPU = false;
    bool Verbose = false;
    bool ShowNets = false;
    uint32_t ThreadCount;
    std::string FileName;

    boost::program_options::options_description Description("Options");
    Description.add_options()
        ("help,h", "Print this help message.")
        ("threads,t", boost::program_options::value<uint32_t>(&ThreadCount)->default_value(4), "Specify number of threads to use if using the CPU.")
        ("infile,i", boost::program_options::value<std::string>(&FileName)->required(), "Specify the input file.")
        ("verbose,v", boost::program_options::bool_switch(&Verbose), "Enable Verbose Output.")
        ("GUI", boost::program_options::bool_switch(&UseGUI), "Enable GUI Interface.")
        ("GPU", boost::program_options::bool_switch(&UseGPU), "Run on a CUDA enabled GPU.")
        ("gui-show-nets", boost::program_options::bool_switch(&ShowNets), "Show nets on the GUI.");
    boost::program_options::variables_map VariableMap;

    // Try reading the program options, print help message if help option specified,
    // print errors (and help again) for erroneous use of options.
    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, Description), VariableMap);

        std::cout << "##**************************************************************************##\n"
        "##                                  EPlacer                                 ##\n"
        "##                                                                          ##\n"
        "## Copyright (C) Mohammad Ewais - All Rights Reserved                       ##\n"
        "## Unauthorized copying of this file, via any medium is strictly prohibited ##\n"
        "## Proprietary and confidential                                             ##\n"
        "## Written by:                                                              ##\n"
        "##      Mohammad Ewais \"mohammad.a.ewais@gmail.com\"                         ##\n"
        "##**************************************************************************##\n";

        if (VariableMap.count("help"))
        {
            std::cout << Description << std::endl;
            exit(0);
        }
        boost::program_options::notify(VariableMap);
    }
    catch (boost::program_options::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << Description << std::endl;
        exit(2);
    }

    if (Verbose)
        std::cout << "EPlacer: Start reading file.\n";
    INFILE::readInFile(FileName, Verbose);
    if (Verbose)
        std::cout << "EPlacer: File read successfully.\n";

    if (UseGUI)
    {
        // When using the GUI, we need to have a separate thread for the GUI and
        // The router. we need to "initialize" the GUI first before swapning
        // the routing thread, so that the router can safely update GUI.
        // Instead of using barriers or so, we will have the GUI init function
        // spawn the router thread!!!
        // If we do not use threading, one of both (either QT or the Routing)
        // will finish before the other can start!
        // NOTE: those two threads are independent of the number of threads
        // entered by the user.
        // Create blank arguments and pass them to the QT App
        char *gui_argv[] = {(char *)"EPlacer", NULL};
        int gui_argc = sizeof(gui_argv) / sizeof(char*) - 1;
        QApplication *App = new QApplication(gui_argc, gui_argv);
        // The following will call the Placer
        LAYOUT::LayoutWidget *MainWindow = new LAYOUT::LayoutWidget(ThreadCount, ShowNets, Verbose);
        App->exec();
    }
    else
    {
        if (Verbose)
            std::cout << "EPlacer: Initializing placement.\n";
        PLACER::initPlacement(ThreadCount, NULL, Verbose);
        PLACER::simulateAnnealing();
    }
}
