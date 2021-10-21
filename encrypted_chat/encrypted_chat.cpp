// encrypted_chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <array>
#include <sodium.h>

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

    Keypair() {
        std::cout << "Generating keypair!" << "\n";
        crypto_box_keypair(pubkey, seckey);
    };

    void print_key(unsigned char * array, unsigned int len) {
        for (unsigned int i = 0; i < len; i++)
            std::cout << std::hex << (unsigned int)array[i] << " ";
        std::cout << std::endl;
    };
    void print_keypair() {
        print_key(pubkey, crypto_kx_PUBLICKEYBYTES);
        print_key(seckey, crypto_kx_SECRETKEYBYTES);
    };
};

class CryptographicUser {
public:
    Keypair keypair; 
    unsigned char rx[crypto_kx_SESSIONKEYBYTES];
    unsigned char tx[crypto_kx_SESSIONKEYBYTES];
    bool secrets_shared = false; 

    TransportCipher encrypt_message(std::string message) {
        TransportCipher cipher; 
        randombytes_buf(cipher.nonce, crypto_secretbox_NONCEBYTES);

        if (message.length() < MESSAGE_LEN)
            crypto_secretbox_easy(cipher.data, (const unsigned char*)message.c_str(), MESSAGE_LEN, cipher.nonce, tx);
        return cipher;
    }

    unsigned char * decrypt_message(TransportCipher cipher) {
        unsigned char decrypted[MESSAGE_LEN];
        if (crypto_secretbox_open_easy(decrypted, cipher.data, CIPHER_LEN, cipher.nonce, rx) == 0) {
            return decrypted; 
        }
    }
};

class User : public CryptographicUser {
public: 
    std::string name; 

    User(std::string str) {
        name = str; 
    }
};

bool key_exchange(CryptographicUser client, CryptographicUser server) {
    if ((crypto_kx_client_session_keys(client.rx, client.tx, client.keypair.pubkey, client.keypair.seckey, server.keypair.pubkey) != 0) ||
        (crypto_kx_server_session_keys(server.rx, server.tx, server.keypair.pubkey, server.keypair.seckey, client.keypair.pubkey) != 0))
        return false;
    else
        return true;
}

int main()
{
    sodium_init();

    User alice("Alice");
    User bob("Bob");

    alice.keypair.print_keypair();
    bob.keypair.print_keypair();

    bool status = key_exchange(alice, bob);
    if (status == true)
        std::cout << "Successful key exchange!" << std::endl;
    else 
        std::cout << "Failed to exchange keys!" << std::endl;

    TransportCipher cipher = alice.encrypt_message("Test!");
    std::cout << bob.decrypt_message(cipher) << std::endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
