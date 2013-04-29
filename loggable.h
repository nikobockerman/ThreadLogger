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

namespace threadlogger {

class Loggable
{
public:
    virtual std::string className() const = 0;
};

}
#endif // LOGGABLE_H
