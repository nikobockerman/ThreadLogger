#ifndef LOGGABLE_H
#define LOGGABLE_H

namespace threadlogger {

class Loggable
{
public:
    virtual std::string className() const = 0;
};

}
#endif // LOGGABLE_H
