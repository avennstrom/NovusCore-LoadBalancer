#include "GeneralHandlers.h"
#include <Networking/MessageHandler.h>
#include <Networking/NetworkPacket.h>
#include <Networking/NetworkClient.h>
#include <Networking/AddressType.h>
#include <Networking/PacketUtils.h>
#include "../../Utils/ServiceLocator.h"
#include "../../ECS/Components/Network/LoadBalanceSingleton.h"

void InternalSocket::GeneralHandlers::Setup(MessageHandler* messageHandler)
{
    messageHandler->SetMessageHandler(Opcode::SMSG_CONNECTED, InternalSocket::GeneralHandlers::SMSG_CONNECTED);
    messageHandler->SetMessageHandler(Opcode::MSG_REQUEST_ADDRESS, InternalSocket::GeneralHandlers::MSG_REQUEST_ADDRESS);
    messageHandler->SetMessageHandler(Opcode::SMSG_SEND_FULL_INTERNAL_SERVER_INFO, InternalSocket::GeneralHandlers::SMSG_SEND_FULL_INTERNAL_SERVER_INFO);
    messageHandler->SetMessageHandler(Opcode::SMSG_SEND_ADD_INTERNAL_SERVER_INFO, InternalSocket::GeneralHandlers::SMSG_SEND_ADD_INTERNAL_SERVER_INFO);
    messageHandler->SetMessageHandler(Opcode::SMSG_SEND_REMOVE_INTERNAL_SERVER_INFO, InternalSocket::GeneralHandlers::SMSG_SEND_REMOVE_INTERNAL_SERVER_INFO);
}

bool InternalSocket::GeneralHandlers::SMSG_CONNECTED(std::shared_ptr<NetworkClient> networkClient, NetworkPacket* packet)
{
    if (networkClient->GetStatus() != ConnectionStatus::AUTH_SUCCESS)
        return false;

    networkClient->SetStatus(ConnectionStatus::CONNECTED);
    return true;
}
bool InternalSocket::GeneralHandlers::MSG_REQUEST_ADDRESS(std::shared_ptr<NetworkClient> networkClient, NetworkPacket* packet)
{
    if (networkClient->GetStatus() != ConnectionStatus::CONNECTED)
        return false;

    // Validate that we did get an AddressType and that it is valid
    AddressType requestType;
    if (!packet->payload->Get(requestType) ||
        (requestType < AddressType::AUTH || requestType >= AddressType::COUNT))
    {
        return false;
    }

    entt::registry* registry = ServiceLocator::GetRegistry();
    auto& loadBalanceSingleton = registry->ctx<LoadBalanceSingleton>();

    ServerInformation serverInformation;
    if (requestType == AddressType::AUTH)
    {
        loadBalanceSingleton.Get<AddressType::AUTH>(serverInformation);
    }
    else if (requestType == AddressType::REALM)
    {
        loadBalanceSingleton.Get<AddressType::REALM>(serverInformation);
    }
    else if (requestType == AddressType::WORLD)
    {
        loadBalanceSingleton.Get<AddressType::WORLD>(serverInformation);
    }
    else if (requestType == AddressType::INSTANCE)
    {
        loadBalanceSingleton.Get<AddressType::INSTANCE>(serverInformation);
    }
    else if (requestType == AddressType::CHAT)
    {
        loadBalanceSingleton.Get<AddressType::CHAT>(serverInformation);
    }
    else if (requestType == AddressType::LOADBALANCE)
    {
        loadBalanceSingleton.Get<AddressType::LOADBALANCE>(serverInformation);
    }
    else if (requestType == AddressType::REGION)
    {
        loadBalanceSingleton.Get<AddressType::REGION>(serverInformation);
    }

    std::shared_ptr<Bytebuffer> buffer = Bytebuffer::Borrow<128>();
    u8 status = 1;

    // If the load balancer couldn't find a valid server, we send status 0 back
    if (serverInformation.type == AddressType::INVALID)
    {
        status = 0;
    }

    if (!PacketUtils::Write_SMSG_SEND_ADDRESS(buffer, status, serverInformation.address, serverInformation.port, packet->payload->GetReadPointer(), packet->payload->GetReadSpace()))
        return false;

    networkClient->Send(buffer.get());
    return true;
}
bool InternalSocket::GeneralHandlers::SMSG_SEND_FULL_INTERNAL_SERVER_INFO(std::shared_ptr<NetworkClient> networkClient, NetworkPacket* packet)
{
    if (networkClient->GetStatus() != ConnectionStatus::CONNECTED)
        return false;

    entt::registry* registry = ServiceLocator::GetRegistry();
    LoadBalanceSingleton& loadBalanceSingleton = registry->ctx<LoadBalanceSingleton>();

    // Clear Cache
    loadBalanceSingleton.Clear();

    ServerInformation serverInformation;
    while (packet->payload->GetReadSpace())
    {
        if (!packet->payload->Get(serverInformation.entity))
            return false;

        if (!packet->payload->Get(serverInformation.type) ||
            (serverInformation.type < AddressType::AUTH || serverInformation.type >= AddressType::COUNT))
        {
            return false;
        }

        if (!packet->payload->GetU32(serverInformation.address))
            return false;

        if (!packet->payload->GetU16(serverInformation.port))
            return false;

        if (serverInformation.type == AddressType::AUTH)
        {
            loadBalanceSingleton.Add<AddressType::AUTH>(serverInformation);
        }
        else if (serverInformation.type == AddressType::REALM)
        {
            loadBalanceSingleton.Add<AddressType::REALM>(serverInformation);
        }
        else if (serverInformation.type == AddressType::WORLD)
        {
            loadBalanceSingleton.Add<AddressType::WORLD>(serverInformation);
        }
        else if (serverInformation.type == AddressType::INSTANCE)
        {
            loadBalanceSingleton.Add<AddressType::INSTANCE>(serverInformation);
        }
        else if (serverInformation.type == AddressType::CHAT)
        {
            loadBalanceSingleton.Add<AddressType::CHAT>(serverInformation);
        }
        else if (serverInformation.type == AddressType::LOADBALANCE)
        {
            loadBalanceSingleton.Add<AddressType::LOADBALANCE>(serverInformation);
        }
        else if (serverInformation.type == AddressType::REGION)
        {
            loadBalanceSingleton.Add<AddressType::REGION>(serverInformation);
        }
    }

    return true;
}
bool InternalSocket::GeneralHandlers::SMSG_SEND_ADD_INTERNAL_SERVER_INFO(std::shared_ptr<NetworkClient> networkClient, NetworkPacket* packet)
{
    if (networkClient->GetStatus() != ConnectionStatus::CONNECTED)
        return false;

    entt::registry* registry = ServiceLocator::GetRegistry();
    LoadBalanceSingleton& loadBalanceSingleton = registry->ctx<LoadBalanceSingleton>();

    ServerInformation serverInformation;

    if (!packet->payload->Get(serverInformation.entity))
        return false;

    if (!packet->payload->Get(serverInformation.type) ||
        (serverInformation.type < AddressType::AUTH || serverInformation.type >= AddressType::COUNT))
    {
        return false;
    }

    if (!packet->payload->GetU32(serverInformation.address))
        return false;

    if (!packet->payload->GetU16(serverInformation.port))
        return false;

    if (serverInformation.type == AddressType::AUTH)
    {
        loadBalanceSingleton.Add<AddressType::AUTH>(serverInformation);
    }
    else if (serverInformation.type == AddressType::REALM)
    {
        loadBalanceSingleton.Add<AddressType::REALM>(serverInformation);
    }
    else if (serverInformation.type == AddressType::WORLD)
    {
        loadBalanceSingleton.Add<AddressType::WORLD>(serverInformation);
    }
    else if (serverInformation.type == AddressType::INSTANCE)
    {
        loadBalanceSingleton.Add<AddressType::INSTANCE>(serverInformation);
    }
    else if (serverInformation.type == AddressType::CHAT)
    {
        loadBalanceSingleton.Add<AddressType::CHAT>(serverInformation);
    }
    else if (serverInformation.type == AddressType::LOADBALANCE)
    {
        loadBalanceSingleton.Add<AddressType::LOADBALANCE>(serverInformation);
    }
    else if (serverInformation.type == AddressType::REGION)
    {
        loadBalanceSingleton.Add<AddressType::REGION>(serverInformation);
    }

    return true;
}
bool InternalSocket::GeneralHandlers::SMSG_SEND_REMOVE_INTERNAL_SERVER_INFO(std::shared_ptr<NetworkClient> networkClient, NetworkPacket* packet)
{
    if (networkClient->GetStatus() != ConnectionStatus::CONNECTED)
        return false;

    entt::registry* registry = ServiceLocator::GetRegistry();
    LoadBalanceSingleton& loadBalanceSingleton = registry->ctx<LoadBalanceSingleton>();

    entt::entity entity = entt::null;
    AddressType type = AddressType::INVALID;

    if (!packet->payload->Get(entity))
        return false;

    if (!packet->payload->Get(type) ||
        (type < AddressType::AUTH || type >= AddressType::COUNT))
    {
        return false;
    }

    loadBalanceSingleton.Remove(type, entity);

    return true;
}