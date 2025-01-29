#include <stdio.h>
#define ENET_IMPLEMENTATION
#include "enet.h"
#include "networking_helpers.h"

// Global variables
ENetHost *enetHost = NULL;
ENetPeer *enetPeer = NULL;

// Function to initialize ENet
int InitEnet()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "ENet initialized successfully.\n");

    // Automatically call enet_deinitialize() at program exit
    atexit(enet_deinitialize);
    return EXIT_SUCCESS;
}

// Function to create an ENet host
ENetHost *CreateEnetHost(uint16_t port, size_t maxClients, size_t maxChannels)
{
    ENetAddress address;
    address.host = ENET_HOST_ANY; // Bind to any available IP
    address.port = port;          // Use the provided port

    ENetHost *serverHost = enet_host_create(&address, maxClients, maxChannels, 0, 0);
    if (serverHost == NULL)
    {
        fprintf(stderr, "Failed to create an ENet host.\n");
        return NULL;
    }

    fprintf(stderr, "ENet host created successfully on port %u.\n", port);
    return serverHost;
}

// Function to connect to a peer
ENetPeer *ConnectToEnetPeer(ENetHost *host, const char *ip, uint16_t port, size_t maxChannels)
{
    ENetAddress peerAddress;
    if (enet_address_set_host(&peerAddress, ip) != 0)
    {
        fprintf(stderr, "Invalid peer IP address: %s\n", ip);
        return NULL;
    }
    peerAddress.port = port;

    ENetPeer *peer = enet_host_connect(host, &peerAddress, maxChannels, 0);
    if (peer == NULL)
    {
        fprintf(stderr, "No available peers for initiating an ENet connection.\n");
        return NULL;
    }

    fprintf(stderr, "Connecting to %s:%u...\n", ip, port);
    return peer;
}

// Cleanup function for the host
void CleanupEnetHost(ENetHost *host)
{
    if (host != NULL)
    {
        enet_host_destroy(host);
        fprintf(stderr, "ENet host destroyed.\n");
    }
}

bool SendENetMessage(ENetPeer *peer, const std::string &message, enet_uint8 channelID, ENetPacketFlag flags)
{
    if (peer == nullptr)
    {
        printf("Error: Peer is null.\n");
        return false;
    }

    // Create a packet with the message data
    ENetPacket *packet = enet_packet_create(message.c_str(), message.size() + 1, flags);

    if (packet == nullptr)
    {
        printf("Error: Failed to create packet.\n");
        return false;
    }

    // Send the packet to the peer on the specified channel
    if (enet_peer_send(peer, channelID, packet) < 0)
    {
        printf("Error: Failed to send packet.\n");
        enet_packet_destroy(packet); // Clean up the packet if sending fails
        return false;
    }

    return true;
}