//
// Created by Michal Kowalczyk on 09/10/2017.
//

#ifndef PUSHUPPAL_TIMESERIE_H
#define PUSHUPPAL_TIMESERIE_H

#include <algorithm>
#include <array>

template <typename T, int N>
class TimeSerie
{
public:
    TimeSerie() : allCount_(0)
    {
        // do nothing
    }

    void addValue(const T& val)
    {
        values_[allCount_ % N] = val;
        ++allCount_;
    }

    T min() const
    {
        return *std::min_element(values_.begin(), values_.end());
    }

    T max() const
    {
        return *std::max_element(values_.begin(), values_.end());
    }

private:
    unsigned int allCount_;
    std::array<T, N> values_;
};

#endif //PUSHUPPAL_TIMESERIE_H
