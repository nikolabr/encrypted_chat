#include "crypto.hpp"

Keypair::Keypair() {
    success = crypto_box_keypair(pubkey, seckey);
};

void Keypair::print_key(unsigned char* array, unsigned int len) {
    for (unsigned int i = 0; i < len; i++)
        std::cout << std::hex << (unsigned int)array[i] << " ";
    std::cout << std::endl;
};

void Keypair::print_keypair() {
    print_key(pubkey, crypto_kx_PUBLICKEYBYTES);
    print_key(seckey, crypto_kx_SECRETKEYBYTES);
};

TransportCipher CryptographicUser::encrypt_message(std::string message) {
    TransportCipher cipher;
    randombytes_buf(cipher.nonce, crypto_secretbox_NONCEBYTES);

    if (message.length() < MESSAGE_LEN)
        crypto_secretbox_easy(cipher.data, (const unsigned char*)message.c_str(), MESSAGE_LEN, cipher.nonce, tx);
    return cipher;
}

void CryptographicUser::decrypt_message(TransportCipher &cipher) {
    unsigned char decrypted[MESSAGE_LEN];
    if (crypto_secretbox_open_easy(decrypted, cipher.data, CIPHER_LEN, cipher.nonce, rx) == 0) {
        std::memset(cipher.data, 0, CIPHER_LEN);
        std::memcpy(cipher.data, decrypted, MESSAGE_LEN);
    }
}

bool key_exchange(CryptographicUser client, CryptographicUser server) {
    if ((crypto_kx_client_session_keys(client.rx, client.tx, client.keypair.pubkey, client.keypair.seckey, server.keypair.pubkey) != 0) ||
        (crypto_kx_server_session_keys(server.rx, server.tx, server.keypair.pubkey, server.keypair.seckey, client.keypair.pubkey) != 0))
        return false;
    else
        return true;
}