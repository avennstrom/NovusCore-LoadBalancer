#include "ConnectionSystems.h"
#include <entt.hpp>
#include <Networking/MessageHandler.h>
#include <Networking/NetworkServer.h>
#include "../../Components/Network/ConnectionSingleton.h"
#include "../../Components/Network/AuthenticationSingleton.h"
#include "../../../Utils/ServiceLocator.h"
#include <tracy/Tracy.hpp>

void ConnectionUpdateSystem::Update(entt::registry& registry)
{
    ZoneScopedNC("ConnectionUpdateSystem::Update", tracy::Color::Blue)
    ConnectionSingleton& connectionSingleton = registry.ctx<ConnectionSingleton>();

    if (connectionSingleton.networkClient)
    {
        std::shared_ptr<NetworkPacket> packet = nullptr;

        MessageHandler* networkMessageHandler = ServiceLocator::GetNetworkMessageHandler();
        while (connectionSingleton.packetQueue.try_dequeue(packet))
        {
#ifdef NC_Debug
            NC_LOG_SUCCESS("[Network/Socket]: CMD: %u, Size: %u", packet->header.opcode, packet->header.size);
#endif // NC_Debug

            if (!networkMessageHandler->CallHandler(connectionSingleton.networkClient, packet))
            {
                connectionSingleton.networkClient->Close(asio::error::shut_down);
                break;
            }
        }
    }
}

void ConnectionUpdateSystem::HandleConnect(BaseSocket* socket, bool connected)
{
    if (connected)
    {
#ifdef NC_Debug
        NC_LOG_SUCCESS("[Network/Socket]: Successfully connected to (%s, %u)", socket->socket()->remote_endpoint().address().to_string().c_str(), socket->socket()->remote_endpoint().port());
#endif // NC_Debug

        entt::registry* registry = ServiceLocator::GetRegistry();
        AuthenticationSingleton& authentication = registry->ctx<AuthenticationSingleton>();
        
        /* Send Initial Packet */
        std::shared_ptr<Bytebuffer> buffer = Bytebuffer::Borrow<512>();

        authentication.srp.username = "loadbalancer";
        authentication.srp.password = "password";

        // If StartAuthentication fails, it means A failed to generate and thus we cannot connect
        if (!authentication.srp.StartAuthentication())
            return;

        buffer->Put(Opcode::CMSG_LOGON_CHALLENGE);
        buffer->SkipWrite(sizeof(u16));

        u16 size = static_cast<u16>(buffer->writtenData);
        buffer->PutString(authentication.srp.username);
        buffer->PutBytes(authentication.srp.aBuffer->GetDataPointer(), authentication.srp.aBuffer->size);

        u16 writtenData = static_cast<u16>(buffer->writtenData) - size;

        buffer->Put<u16>(writtenData, 2);
        socket->Send(buffer);

        NetworkClient* networkClient = static_cast<NetworkClient*>(socket);
        networkClient->SetStatus(ConnectionStatus::AUTH_CHALLENGE);
        socket->AsyncRead();
    }
    else
    {
#ifdef NC_Debug
        NC_LOG_WARNING("[Network/Socket]: Failed connecting to (%s, %u)", socket->socket()->remote_endpoint().address().to_string().c_str(), socket->socket()->remote_endpoint().port());
#endif // NC_Debug
    }
}
void ConnectionUpdateSystem::HandleRead(BaseSocket* socket)
{
    entt::registry* registry = ServiceLocator::GetRegistry();
    ConnectionSingleton& connectionSingleton = registry->ctx<ConnectionSingleton>();

    NetworkClient* client = static_cast<NetworkClient*>(socket);
    std::shared_ptr<Bytebuffer> buffer = client->GetReceiveBuffer();

    while (buffer->GetActiveSize())
    {
        Opcode opcode = Opcode::INVALID;
        u16 size = 0;

        buffer->Get(opcode);
        buffer->GetU16(size);

        if (size > NETWORK_BUFFER_SIZE)
        {
            client->Close(asio::error::shut_down);
            return;
        }
        
        std::shared_ptr<NetworkPacket> packet = NetworkPacket::Borrow();
        {
            // Header
            {
                packet->header.opcode = opcode;
                packet->header.size = size;
            }

            // Payload
            {
                if (size)
                {
                    packet->payload = Bytebuffer::Borrow<NETWORK_BUFFER_SIZE>();
                    packet->payload->size = size;
                    packet->payload->writtenData = size;
                    std::memcpy(packet->payload->GetDataPointer(), buffer->GetReadPointer(), size);
                }
            }

            connectionSingleton.packetQueue.enqueue(packet);
        }

        buffer->readData += size;
    }

    client->Listen();
}
void ConnectionUpdateSystem::HandleDisconnect(BaseSocket* socket)
{
#ifdef NC_Debug
    NC_LOG_WARNING("[Network/Socket]: Disconnected from (%s, %u)", socket->socket()->remote_endpoint().address().to_string().c_str(), socket->socket()->remote_endpoint().port());
#endif // NC_Debug
}