#pragma once
#include <asio.hpp>
#include <entity/fwd.hpp>
#include <Utils/ConcurrentQueue.h>

class NetworkServer;
class BaseSocket;
namespace moddycamel
{
    class ConcurrentQueue;
}
class ConnectionUpdateSystem
{
public:
    static void Update(entt::registry& registry);

    // Handlers for Network Client
    static void HandleRead(BaseSocket* socket);
    static void HandleConnect(BaseSocket* socket, bool connected);
    static void HandleDisconnect(BaseSocket* socket);
};