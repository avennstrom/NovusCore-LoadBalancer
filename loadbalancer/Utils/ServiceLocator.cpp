#include "ServiceLocator.h"
#include <Networking/MessageHandler.h>

entt::registry* ServiceLocator::_gameRegistry = nullptr;
MessageHandler* ServiceLocator::_networkMessageHandler = nullptr;

void ServiceLocator::SetRegistry(entt::registry* registry)
{
    assert(_gameRegistry == nullptr);
    _gameRegistry = registry;
}
void ServiceLocator::SetNetworkMessageHandler(MessageHandler* networkMessageHandler)
{
    assert(_networkMessageHandler == nullptr);
    _networkMessageHandler = networkMessageHandler;
}