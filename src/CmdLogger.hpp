/*******************************************************************************
 * Project:  NebulaLogger
 * @file     CmdLogger.hpp
 * @brief    
 * @author   Bwar
 * @date:    2018-6-7
 * @note
 * Modify history:
 ******************************************************************************/
#ifndef SRC_CMDLOGGER_HPP_
#define SRC_CMDLOGGER_HPP_

#include <string>
#include <fstream>
#include <actor/cmd/Cmd.hpp>

namespace logger
{

class CmdLogger: public neb::Cmd, public neb::DynamicCreator<CmdLogger, int32>
{
public:
    CmdLogger(int32 iCmd);
    virtual ~CmdLogger();

    virtual bool Init();
    virtual bool AnyMessage(
                    std::shared_ptr<neb::SocketChannel> pChannel, 
                    const MsgHead& oMsgHead,
                    const MsgBody& oMsgBody);

protected:
    bool OpenDataFile();

private:
    int32 m_iLogMaxFileSize;
    std::string m_strLogDataPath;
    std::string m_strLogFileName;
    std::string m_strLogDataFile;
    std::ofstream m_ofs;
};

}

#endif /* SRC_CMDLOGGER_HPP_ */
