#include "DataBlk.h"


DataBlk::DataBlk(const DataBlk& other)
{
    //copy ctor
}

DataBlk& DataBlk::operator=(const DataBlk& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
