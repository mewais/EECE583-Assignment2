#ifndef LAYOUT_HPP
#define LAYOUT_HPP

#include <QtCore/QSize>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtGui/QPainter>

#include <iostream>
#include <utility>
#include <tuple>
#include <algorithm>

#include <pthread.h>

#include "Placer.hpp"

#define PixelsPerGridBlock 40
#define PixelsPerChannelWidth 5

namespace LAYOUT
{
    void *initPlacement(void *PlaceHolder);

    // some of QT capabilities can only be inherited, I'm forced to use a class
    // here although nothing really requires an OOP structure.
    class LayoutWidget : public QWidget
    {
        protected:
            bool Verbose;
            bool ShowNets;

            void paintEvent(QPaintEvent *event);

        public:
            LayoutWidget(uint32_t ThreadCount, bool ShowNets, bool Verbose);
    };
}

#endif // LAYOUT_HPP
