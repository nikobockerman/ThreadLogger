/*
Author:
Niko Böckerman
*/
#include "threadlogger.h"

#include "compiler_check.h"

#include <QSettings>
#include <QStringList>
#include <QReadWriteLock>
#include <QWriteLocker>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QString>

#include <thread>
#include <unordered_map>
#include <mutex>
#include <iostream>

#if GCC_VERSION > 40700
using threadid = std::thread::id;
#else
typedef std::thread::id threadid;
#endif

namespace threadlogger {

QString toString(const LogLevel &logLevel)
{
    switch(logLevel) {
    case LogLevel::DEBUG:
        return "DEBUG";
        break;
    case LogLevel::VERBOSE:
        return "VERBOSE";
        break;
    case LogLevel::INFO:
        return "INFO";
        break;
    case LogLevel::MANDATORY:
        return "MANDATORY";
        break;
    case LogLevel::ERROR:
        return "ERROR";
        break;
    case LogLevel::PLAINTEXT:
        return "PLAINTEXT";
    }
    return 0;
}




// Functions
constexpr int toInt(const LogLevel &verbosity)
{
    return static_cast<int>(verbosity);
}


threadid getThreadId()
{
    return std::this_thread::get_id();
}



// Classes

class LogHolder
{
public:
    bool contains(const threadid &id);
    bool insert(const threadid &id, const Log &log);
    /**
     * @brief Get Log with the given id.
     * @param id
     * @return Matching Log or first inserted log if not found.
     */
    Log value(const threadid &id);

#if GCC_VERSION > 40700
    std::vector<Log> storage {};
#else
    std::vector<Log> storage;
#endif
    QReadWriteLock lock;
};


class DuplicateLogError : public std::runtime_error
{};


class LogPrivate
{
public:
    LogPrivate();
    LogPrivate(const QString threadName);
    ~LogPrivate();
    LogPrivate(const LogPrivate&) = delete;
    LogPrivate &operator =(const LogPrivate&) = delete;

    void init(const QString &folder, const QString &filename, const LogLevel &coutLevel, const LogLevel &logFileLevel);
    LogInstance instance(const QString &callerName, const LogLevel &verbosity);

    static bool addLog(Log &log);
    static Log log();

#if GCC_VERSION > 40700
    QAtomicInt ref = 0;
    bool m_valid = false;
    QString m_threadName = QString();
#else
    QAtomicInt ref;
    bool m_valid;
    QString m_threadName;
#endif

    threadid m_threadId;
    QFile m_logFile;

    LogLevel m_logMsgLevel; // Min verbosity level for log file
    LogLevel m_coutMsgLevel; // Min verbosity level for cout
};


class LogInstancePrivate
{
public:
    /**
     * @brief LogInstancePrivate No output
     */
    LogInstancePrivate();

    /**
     * @brief LogInstancePrivate
     * @param callerName
     * @param verbosity
     * @param threadName
     * @param logFile
     * @param logFileLevel
     * @param coutLevel
     */
    LogInstancePrivate(const QString &callerName, const LogLevel &verbosity,
                       const QString &threadName, QFile &logFile,
                       const LogLevel &logFileLevel, const LogLevel &coutLevel);
    ~LogInstancePrivate();
    LogInstancePrivate(const LogInstancePrivate&) = delete;
    LogInstancePrivate &operator =(const LogInstancePrivate&) = delete;

    template <class T>
    LogInstancePrivate &operator <<(const T &t);

    LogInstancePrivate &operator <<(std::ostream &endl(std::ostream &os));

    QString dynamicLogFilePrefix();
    QString dynamicCoutPrefix();
    QString logFilePrefix();
    QString coutPrefix();
    QString commonPrefix();

#if GCC_VERSION > 40700
    QAtomicInt ref = 0;
    bool m_valid = false;
    bool m_startedLogFile = false;
    bool m_startedCout = false;
    bool m_outputFile = false;
    bool m_outputCout = false;
#else
    bool m_valid;
    QAtomicInt ref;
    bool m_startedLogFile;
    bool m_startedCout;
    bool m_outputFile;
    bool m_outputCout;
#endif

    QString m_callerName;
    LogLevel m_verbosity;
    QString m_threadName;

    QTextStream m_logFileStream;
    QTextStream m_coutStream;
};




// Static variables
static LogHolder dict;




// LogHolder

bool LogHolder::contains(const threadid &id)
{
    QReadLocker locker(&lock);
    for (const Log &i : storage) {
        if (i.d->m_threadId == id)
            return true;
    }
    return false;
}

bool LogHolder::insert(const threadid &id, const Log &log)
{
    if (not contains(id)) {
        QWriteLocker locker(&lock);
        storage.push_back(log);
        log.d->m_threadId = id;
    } else {
        return false;
    }
    return true;
}

Log LogHolder::value(const threadid &id)
{
    QReadLocker locker(&lock);
    for (const Log &i : storage) {
        if (i.d->m_threadId == id)
            return i;
    }
    if (not storage.empty())
        return storage.front();
    return Log();
}



// LogPrivate

bool LogPrivate::addLog(Log &log)
{
    threadid threadId = getThreadId();
    if (not dict.contains(threadId))
        dict.insert(threadId, log);
    else
        return false;
    return true;
}


Log LogPrivate::log()
{
    threadid threadId = getThreadId();
    return dict.value(threadId);
}


void LogPrivate::init(const QString &folder, const QString &filename, const LogLevel &coutLevel, const LogLevel &logFileLevel)
{

    QDir dir {folder};
    if (dir.isAbsolute() and not dir.exists())
    {
        // TODO Check if successful
        dir.mkpath(folder);
    } else if (dir.isRelative() and not dir.exists()) {
        // TODO Check if successful
        QDir curDir {QDir::current()};
        curDir.mkpath(folder);
    }

    QFileInfo info {folder, filename};
    m_logFile.setFileName(info.absoluteFilePath());
    if (not m_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
        std::cerr << "Failed to open log file" << std::endl;
    }


    m_logMsgLevel = logFileLevel;
    m_coutMsgLevel = coutLevel;
}


LogInstance LogPrivate::instance(const QString &callerName, const LogLevel &verbosity)
{
    if (not m_valid)
        return LogInstance();
    return LogInstance(callerName, verbosity, m_threadName, m_logFile, m_logMsgLevel,
                       m_coutMsgLevel);
}

QAtomicInt ref = 0;
bool m_valid = false;
QString m_threadName = QString();

LogPrivate::LogPrivate()
#if GCC_VERSION < 40700
    : ref{0}, m_valid{false}
#endif
{}


LogPrivate::LogPrivate(const QString threadName)
    :
#if GCC_VERSION < 40700
      ref{0},
#endif
      m_valid{true}, m_threadName{threadName}
{}


LogPrivate::~LogPrivate()
{
    if (m_logFile.isOpen())
        m_logFile.close();
}



// LogInstancePrivate

LogInstancePrivate::LogInstancePrivate()
#if GCC_VERSION < 40700
    : m_valid{false}, ref{0}, m_startedLogFile{false}, m_startedCout{false}, m_outputFile{false},
      m_outputCout{false}
#endif
{}


LogInstancePrivate::LogInstancePrivate(const QString &callerName, const LogLevel &verbosity,
                                       const QString &threadName, QFile &logFile,
                                       const LogLevel &logFileLevel,
                                       const LogLevel &coutLevel)
    : m_valid{true},
#if GCC_VERSION < 40700
      ref{0}, m_startedLogFile{false}, m_startedCout{false},
#endif
      m_outputFile{toInt(verbosity) >= toInt(logFileLevel)
                   or verbosity == LogLevel::PLAINTEXT},
      m_outputCout{toInt(verbosity) >= toInt(coutLevel)
                   or verbosity == LogLevel::PLAINTEXT},
      m_callerName{callerName},
      m_verbosity{verbosity},
      m_threadName{(m_outputFile ? threadName : "")},
      m_coutStream{((verbosity == LogLevel::ERROR) ? stderr : stdout)}
{
    if (m_outputFile)
        m_logFileStream.setDevice(&logFile);
}


LogInstancePrivate::~LogInstancePrivate()
{
    if (m_valid) {
        if (m_startedLogFile and m_outputFile) {
            m_logFileStream << endl;
        }
        if (m_startedCout and m_outputCout) {
            m_coutStream << endl;
        }
    }
}


template <class T>
LogInstancePrivate &LogInstancePrivate::operator <<(const T &t)
{
    if (m_valid) {
        if (m_outputFile) {
            m_logFileStream << dynamicLogFilePrefix() << t;
            m_startedLogFile = true;
        }
        if (m_outputCout) {
            m_coutStream << dynamicCoutPrefix() << t;
            m_startedCout = true;
        }
    }
    return *this;
}


LogInstancePrivate &LogInstancePrivate::operator <<(std::ostream &endline(std::ostream &os))
{
    Q_UNUSED(endline);
    if (m_valid) {
        if (m_outputFile) {
            m_logFileStream << dynamicLogFilePrefix() << endl;
            m_startedLogFile = false;
        }
        if (m_outputCout) {
            m_coutStream << dynamicCoutPrefix() << endl;
            m_startedCout = false;
        }
    }
    return *this;
}


QString LogInstancePrivate::dynamicCoutPrefix()
{
    if (m_startedCout or m_verbosity == LogLevel::PLAINTEXT)
        return "";
    return coutPrefix();
}


QString LogInstancePrivate::dynamicLogFilePrefix()
{
    if (m_startedLogFile
            or m_verbosity == LogLevel::PLAINTEXT)
        return "";
    return logFilePrefix();
}


QString LogInstancePrivate::coutPrefix()
{
    QString prefix;
    if (not m_threadName.isEmpty())
        prefix += m_threadName + ": ";
    prefix += commonPrefix() + ": ";
    return prefix;
}


QString LogInstancePrivate::logFilePrefix()
{
    return commonPrefix() + ": ";
}


QString LogInstancePrivate::commonPrefix()
{
    QDateTime currentTimeUtc {QDateTime::currentDateTimeUtc()};
    QString prefix;

    prefix += currentTimeUtc.toString("dd.MM.yyyy hh:mm:ss") + " ";
    prefix += toString(m_verbosity);
    if (m_threadName != m_callerName)
        prefix += " " + m_callerName;
    return prefix;
}




// LogInstance

LogInstance::LogInstance() : d{new LogInstancePrivate()} {}
LogInstance::LogInstance(const LogInstance &other) : d{other.d} {}
LogInstance::~LogInstance() {}


LogInstance::LogInstance(const QString &callerName, const LogLevel &verbosity,
                         const QString &threadName, QFile &logFile,
                         const LogLevel &logFileLevel, const LogLevel &coutLevel)
    : d{new LogInstancePrivate(callerName, verbosity, threadName, logFile, logFileLevel,
                               coutLevel)}
{}


LogInstance &LogInstance::operator =(const LogInstance &other)
{
    d = other.d;
    return *this;
}


LogInstance &LogInstance::operator <<(const QString &t)
{
    *d << t;
    return *this;
}


LogInstance &LogInstance::operator <<(const int &t)
{
    *d << t;
    return *this;
}


LogInstance &LogInstance::operator <<(const unsigned int &t)
{
    *d << t;
    return *this;
}


LogInstance &LogInstance::operator <<(std::ostream &endl(std::ostream &os))
{
    *d << endl;
    return *this;
}




// Log

Log Log::addLog()
{
    return Log::addLog(QString(""));
}


Log Log::addLog(const QString &threadName)
{
    Log log {threadName};
    if (not LogPrivate::addLog(log))
        return Log();
    return log;
}


LogInstance Log::instance(const QString &callerName, const LogLevel &verbosity)
{
    return d->instance(callerName, verbosity);
}


Log Log::log()
{
    return LogPrivate::log();
}


Log::Log() : d{new LogPrivate()}
{}


Log::Log(const Log &other) : d{other.d}
{}


Log::~Log()
{}


Log &Log::operator =(const Log &other)
{
    d = other.d;
    return *this;
}


Log::Log(const QString &threadName) : d{new LogPrivate(threadName)}
{}


void Log::init(const QString &folder, const QString &filename, const LogLevel &coutLevel, const LogLevel &logFileLevel)
{
    d->init(folder, filename, coutLevel, logFileLevel);
}




// lDebug

LogInstance lDebug()
{
    return lDebug(QString());
}


LogInstance lDebug(const QString &callerName)
{
    return Log::log().instance(callerName, LogLevel::DEBUG);
}



// lVerbose

LogInstance lVerbose()
{
    return lVerbose(QString());
}


LogInstance lVerbose(const QString &callerName)
{
    return Log::log().instance(callerName, LogLevel::VERBOSE);
}



// lInfo

LogInstance lInfo()
{
    return lInfo(QString());
}


LogInstance lInfo(const QString &callerName)
{
    return Log::log().instance(callerName, LogLevel::INFO);
}



// lMandatory

LogInstance lMandatory()
{
    return lMandatory(QString());
}


LogInstance lMandatory(const QString &callerName)
{
    return Log::log().instance(callerName, LogLevel::MANDATORY);
}



// lError

LogInstance lError()
{
    return lError(QString());
}


LogInstance lError(const QString &callerName)
{
    return Log::log().instance(callerName, LogLevel::ERROR);
}



// lPlaintext

LogInstance lPlaintext()
{
    return Log::log().instance(QString(), LogLevel::PLAINTEXT);
}



}
