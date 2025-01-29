#ifndef NETWORK_HELPERS_H
#define NETWORK_HELPERS_H

#include "enet.h"

// ENet variables
extern ENetHost *enetHost; // Global host pointer
extern ENetPeer *enetPeer; // Global peer pointer

int InitEnet();
ENetHost *CreateEnetHost(uint16_t port, size_t maxClients, size_t maxChannels);
ENetPeer *ConnectToEnetPeer(ENetHost *host, const char *ip, uint16_t port, size_t maxChannels);
void CleanupEnetHost(ENetHost *host);

#endif // NETWORK_HELPERS_H