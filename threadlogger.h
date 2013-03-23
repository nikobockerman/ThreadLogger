#ifndef THREADLOGGER_HH
#define THREADLOGGER_HH

#include <QExplicitlySharedDataPointer>
#include <QString>

#include <sstream>

class QTextStream;
class QFile;

namespace threadlogger {

class LogInstancePrivate;
class LogPrivate;
    
enum class LogLevel {
    DEBUG,
    VERBOSE,
    INFO,
    MANDATORY,
    ERROR,
    PLAINTEXT
};

class LogInstance
{
public:
    /**
     * @brief Create invalid LogInstance
     * Nothing can be logged through invalid LogInstance object
     */
    LogInstance();


    /**
     * @brief Create valid LogInstance
     * @param callerName Name of the calling function to be appended to the message prefix
     * @param verbosity Message verbosity level
     * @param threadName Thread name to be appended to the message prefix to standard output
     * @param logFile QFile object to use for logging. The file needs to be open.
     * @param logFileLevel Minimum logging level for logging to file
     * @param coutLevel Minimum logging level for logging to file
     */
    LogInstance(const QString &callerName, const LogLevel &verbosity,
                const QString &threadName, QFile &logFile,
                const LogLevel &logFileLevel, const LogLevel &coutLevel);

    LogInstance(const LogInstance &other);
    ~LogInstance();
    LogInstance &operator =(const LogInstance &other);

    /*
     * Stream functions for logging different types of objects.
     */
    LogInstance &operator <<(const QString &t);
    LogInstance &operator <<(const unsigned int &t);
    LogInstance &operator <<(const int &t);

    /**
     * @brief operator << for logging std::endl
     * @param endl
     * @return
     */
    LogInstance &operator <<(std::ostream &endl(std::ostream &os));

    /**
     * @brief Template output stream operator function for everything that support output to std::stringstream.
     */
    template <class T>
    LogInstance &operator <<(const T &t)
    {
        std::stringstream stream;
        stream << t;
        return (*this) << QString::fromStdString(stream.str());

    }

private:
    friend class LogInstancePrivate;
    QExplicitlySharedDataPointer<LogInstancePrivate> d;
};


class Log
{
public:
    /**
     * @brief Create invalid Log.
     */
    Log();

    Log(const Log &other);
    ~Log();
    Log &operator =(const Log &other);

    void init(const QString &folder, const QString &filename, const LogLevel &coutLevel, const LogLevel &logFileLevel);

    /**
     * @brief Create a Logging instance for this Log
     * @param callerName Caller name to be placed to the log message prefix
     * @param verbosity Message verbosity level to use for the created LogInstance
     * @return LogInstance which performs the actual logging.
     */
    LogInstance instance(const QString &callerName, const LogLevel &verbosity);

    /**
     * @brief Create Log.
     * Only one logger is allowed per thread. If trying to add second Log for the same thread
     * returns invalid Log.
     * @return Created Log object for initialization.
     */
    static Log addLog();

    /**
     * @brief Create new Log.
     * @param threadName Appended to the message prefix when logging to standard output.
     * @return Created Log object for initialization.
     */
    static Log addLog(const QString &threadName);

    /**
     * @brief Get the Log created for the current thread.
     * @return
     */
    static Log log();

protected:
    /**
     * @brief Create valid Log with a thread name.
     * @param threadName Appended to the message prefix when logging to standard output.
     */
    explicit Log(const QString &threadName);

private:
    friend class LogPrivate;
    friend class LogHolder;
    QExplicitlySharedDataPointer<LogPrivate> d;
};


/**
 * @brief Get LogInstance object for logging with verbosity level Debug
 *
 * Get LogInstance from thread specific Log object.
 * Caller name is empty and verbosity level is DEBUG.
 * @return LogInstance object for logging
 */
LogInstance lDebug();

/**
 * @brief Get LogInstance object for logging with verbosity level Debug
 *
 * Same as lDebug() with caller name specified.
 * @param callerName Caller name passed to LogInstance
 * @return LogInstance object for logging
 */
LogInstance lDebug(const QString &callerName);

/**
 * @brief Same as lDebug() with verbosity level Verbose.
 * @return
 */
LogInstance lVerbose();

/**
 * @brief Same as lDebug(const QString &callerName) with verbosity level Verbose.
 * @param callerName
 * @return
 */
LogInstance lVerbose(const QString &callerName);


/**
 * @brief Same as lDebug() with verbosity level Info.
 * @return
 */
LogInstance lInfo();

/**
 * @brief Same as lDebug(const QString &callerName) with verbosity level Info.
 * @param callerName
 * @return
 */
LogInstance lInfo(const QString &callerName);

/**
 * @brief Same as lDebug() with verbosity level Mandatory.
 * @return
 */
LogInstance lMandatory();

/**
 * @brief Same as lDebug(const QString &callerName) with verbosity level Mandatory.
 * @param callerName
 * @return
 */
LogInstance lMandatory(const QString &callerName);

/**
 * @brief Same as lDebug() with verbosity level Error.
 * @return
 */
LogInstance lError();

/**
 * @brief Same as lDebug(const QString &callerName) with verbosity level Error.
 * @param callerName
 * @return
 */
LogInstance lError(const QString &callerName);

/**
 * @brief Get LogInstance for logging with verbosity level Plaintext.
 *
 * With Plaintext level no prefix is generated and therefore callerName can't be given.
 * @sa lDebug()
 * @return
 */
LogInstance lPlaintext();

}

#endif // THREADLOGGER_HH
