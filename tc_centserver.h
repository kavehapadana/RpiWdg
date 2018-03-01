#ifndef TC_CENTSERVER_H
#define TC_CENTSERVER_H
#include <Tcp_Client.h>
class TC_CentServer : public Tcp_Client
{

public:
    TC_CentServer();
private:
public slots:
    void Parse_Byte(u_int8_t DATA) override;
    void initClient() override;
};

#endif // TC_CENTSERVER_H
