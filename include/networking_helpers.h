#ifndef NETWORK_HELPERS_H
#define NETWORK_HELPERS_H

#include "enet.h"
#include <string>

// ENet variables
extern ENetHost *enetHost; // Global host pointer
extern ENetPeer *enetPeer; // Global peer pointer

int InitEnet();
ENetHost *CreateEnetHost(uint16_t port, size_t maxClients, size_t maxChannels);
ENetPeer *ConnectToEnetPeer(ENetHost *host, const char *ip, uint16_t port, size_t maxChannels);
void CleanupEnetHost(ENetHost *host);
bool SendENetMessage(ENetPeer *peer, const std::string &message, enet_uint8 channelID = 0, ENetPacketFlag flags = ENET_PACKET_FLAG_RELIABLE);

#endif // NETWORK_HELPERS_H