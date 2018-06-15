/*******************************************************************************
 * Project:  NebulaLogger
 * @file     CmdLogger.cpp
 * @brief    
 * @author   Bwar
 * @date:    2018-6-7
 * @note
 * Modify history:
 ******************************************************************************/

#include <sys/stat.h>       //need int mkdir(const char *pathname, mode_t mode)
#include <sys/types.h>      //need int mkdir(const char *pathname, mode_t mode)
#include <chrono>
#include <iomanip>
#include <sstream>
#include <util/json/CJsonObject.hpp>
#include <pb/neb_sys.pb.h>
#include "CmdLogger.hpp"

namespace logger
{

CmdLogger::CmdLogger(int32 iCmd)
    : neb::Cmd(iCmd), m_iLogMaxFileSize(20480000)
{
}

CmdLogger::~CmdLogger()
{
    if (m_ofs.is_open())
    {
        m_ofs.flush();
        m_ofs.close();
    }
}

bool CmdLogger::Init()
{
    neb::CJsonObject oJson = GetCustomConf();
    oJson["log_data"].Get("data", m_strLogDataPath);
    oJson["log_data"].Get("log_data", m_strLogFileName);
    oJson["log_data"].Get("data_file_size", m_iLogMaxFileSize);
    if (m_strLogDataPath[0] != '/')
    {
        m_strLogDataPath = GetWorkPath() + std::string("/") + m_strLogDataPath;
    }
    if (access(m_strLogDataPath.c_str(), W_OK) != 0)
    {
        if (mkdir(m_strLogDataPath.c_str(), S_IRWXU) != 0)
        {
            LOG4_ERROR("create dir %s error!", m_strLogDataPath.c_str());
            return(false);
        }
    }

    return(OpenDataFile());
}

bool CmdLogger::AnyMessage(
                std::shared_ptr<neb::SocketChannel> pChannel, 
                const MsgHead& oMsgHead, const MsgBody& oMsgBody)
{
    if (!OpenDataFile())
    {
        return(false);
    }
    
    neb::TraceLog oTraceLog;
    if (!oTraceLog.ParseFromString(oMsgBody.data()))
    {
        LOG4_ERROR("log trace format error!");
        return(false);
    }
    auto time_now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(time_now);
    if (oTraceLog.node_identify() == oMsgBody.req_target().route())     // no trace_id
    {
        m_ofs << oTraceLog.node_identify() << "|" << std::put_time(std::localtime(&t), "%Y%m%d%H%M%S") << "|"
            << oTraceLog.code_file_name() << ":" << oTraceLog.code_file_line << "|"
            << oTraceLog.code_function() << "|" << oTraceLog.log_content() << "\n";
    }
    else
    {
        m_ofs << oTraceLog.node_identify() << "|" << std::put_time(std::localtime(&t), "%Y%m%d%H%M%S") << "|"
            << oTraceLog.code_file_name() << ":" << oTraceLog.code_file_line << "|"
            << oTraceLog.code_function() << "|" << oTraceLog.log_content() << "|"
            << oMsgBody.req_target().route() << "\n";
    }
    return(true);
}

bool CmdLogger::OpenDataFile()
{
    if (m_ofs.is_open())
    {
        if (m_ofs.tellp() < m_iLogMaxFileSize)
        {
            return(true);
        }
        m_ofs.flush();
        m_ofs.close();
    }
    auto time_now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(time_now);
    std::ostringstream osDataFile;
    osDataFile << m_strLogDataPath << "/" << m_strLogFileName << "." << std::put_time(std::localtime(&t), "%Y%m%d%H%M%S");
    m_strLogDataFile = osDataFile.str();
    m_ofs.open(m_strLogDataFile, std::ios::app);
    if (!m_ofs.good())
    {
        LOG4_ERROR("failed to open file \"%s\"", m_strLogDataFile.c_str());
        return(false);
    }
    return(true);
}

}
