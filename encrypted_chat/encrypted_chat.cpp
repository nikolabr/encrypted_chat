// encrypted_chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <array>
#include <sodium.h>

class Keypair {
private: 
    std::array<unsigned char, crypto_box_PUBLICKEYBYTES> pubkey; 
    std::array<unsigned char, crypto_box_SECRETKEYBYTES> seckey;

public: 
    Keypair() {
        std::cout << "Generating keypair!" << "\n";
        crypto_box_keypair(pubkey.data(), seckey.data());
    };
    template <size_t N>
    void print_key(std::array<unsigned char, N> array) {
        for (const auto& byte : array)
            std::cout << std::hex << (unsigned int)byte << " ";
    };
    void print_keypair() {
        print_key(pubkey);
    };
};

int main()
{
    sodium_init();
    Keypair keypair;
    keypair.print_keypair();
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
