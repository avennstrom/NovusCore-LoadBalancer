#pragma once
#include <memory>

class MessageHandler;
class NetworkClient;
struct NetworkPacket;
namespace InternalSocket
{
    class GeneralHandlers
    {
    public:
        static void Setup(MessageHandler*);
        static bool SMSG_CONNECTED(std::shared_ptr<NetworkClient>, NetworkPacket*);
        static bool MSG_REQUEST_ADDRESS(std::shared_ptr<NetworkClient>, NetworkPacket*);
        static bool SMSG_SEND_FULL_INTERNAL_SERVER_INFO(std::shared_ptr<NetworkClient>, NetworkPacket*);
        static bool SMSG_SEND_ADD_INTERNAL_SERVER_INFO(std::shared_ptr<NetworkClient>, NetworkPacket*);
        static bool SMSG_SEND_REMOVE_INTERNAL_SERVER_INFO(std::shared_ptr<NetworkClient>, NetworkPacket*);
    };
}