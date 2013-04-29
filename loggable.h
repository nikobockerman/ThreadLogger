#ifndef LOGGABLE_H
#define LOGGABLE_H

#include <string>

template<typename T> class loggable {
    virtual void toOutput(std::ostream& out) const = 0;
    friend std::ostream& ::operator<<(std::ostream& out, const T& a)
    {
        a.toOutput(out);
        return out;
    }
};

#endif // LOGGABLE_H
