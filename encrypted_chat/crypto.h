#pragma once
#include <sodium.h>
#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>

#define MESSAGE_LEN 200
#define CIPHER_LEN ( crypto_secretbox_MACBYTES + MESSAGE_LEN ) 

typedef struct {
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char data[CIPHER_LEN];
} TransportCipher;

class Keypair {
public:
    unsigned char pubkey[crypto_kx_PUBLICKEYBYTES];
    unsigned char seckey[crypto_kx_SECRETKEYBYTES];

    Keypair();
    int success = false;

    void print_key(unsigned char* array, unsigned int len);
    void print_keypair();
};

class CryptographicUser {
public:
    Keypair keypair;
    unsigned char rx[crypto_kx_SESSIONKEYBYTES];
    unsigned char tx[crypto_kx_SESSIONKEYBYTES];
    bool secrets_shared = false;

    TransportCipher encrypt_message(std::string message);

    unsigned char* decrypt_message(TransportCipher cipher);
};

bool key_exchange(CryptographicUser client, CryptographicUser server);
