// encrypted_chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "crypto.hpp"
#include "net.hpp"
#include <wx/wx.h>
#include <wx/socket.h>

class User : public CryptographicUser {
public: 
    char name[16];
};

User me;
bool is_server;

struct {
    bool received_data = false;
    unsigned char pubkey[crypto_kx_PUBLICKEYBYTES];
    char name[16];
} Peer;

class EncryptedChatApp : public wxApp {
public: 
    EncryptedChatApp();
    virtual ~EncryptedChatApp();
    virtual bool OnInit() override;
};

EncryptedChatApp::EncryptedChatApp()
{
}

EncryptedChatApp::~EncryptedChatApp()
{
}

class AppFrame : public wxFrame
{
public:
    AppFrame(wxString hostname);

    wxTextCtrl* text_output;
    wxTextCtrl* text_input;

    wxSocketServer* server_sock;
    wxSocketClient* client_sock;
    wxSocketBase* peer_sock;
    wxDECLARE_EVENT_TABLE();

private:
    void OnExit(wxCommandEvent& event);
    void UsernamePrompt(wxCommandEvent& event);
    void OnSend(wxCommandEvent& event);
    void OnServerEvent(wxSocketEvent& event);
    void OnSocketEvent(wxSocketEvent& event);

};

enum {
    ID_Output = 1,
    ID_Input = 2,
    ID_Socket = 3,
    ID_Server = 4,
};

enum {
    ENCRYPTED_CHAT_USER_DATA = 0xA0,
    ENCRYPTED_CHAT_MESSAGE = 0xA1
};

void AppFrame::OnSend(wxCommandEvent& event) {
    std::string input = text_input->GetValue().utf8_string();
    text_output->WriteText(wxNow() << ' ' << me.name << ": " << input << '\n');

    TransportCipher cipher = me.encrypt_message(input);

    if (peer_sock) {
        if (peer_sock->IsConnected()) {
            unsigned char buf[2];

            buf[0] = ENCRYPTED_CHAT_MESSAGE;
            buf[1] = sizeof(TransportCipher);

            peer_sock->Write(buf, 2);
            peer_sock->Write(&cipher, sizeof(TransportCipher));
        }
    }
    else if (client_sock) {
        if (client_sock->IsConnected()) {
            unsigned char buf[2];

            buf[0] = ENCRYPTED_CHAT_MESSAGE;
            buf[1] = sizeof(TransportCipher);

            client_sock->Write(buf, 2);
            client_sock->Write(&cipher, sizeof(TransportCipher));
        }
    }
    text_input->Clear();
}

void AppFrame::OnServerEvent(wxSocketEvent& event) {
    switch (event.GetSocketEvent()) {
    case wxSOCKET_CONNECTION:
        peer_sock = server_sock->Accept(false);
        if (peer_sock) {

            wxIPV4address addr;

            if (peer_sock->GetPeer(addr)) {
                text_output->WriteText(wxNow() << " New connection from " << addr.IPAddress() << '\n');
            }

            unsigned char buf[2 + crypto_kx_PUBLICKEYBYTES + 16];

            buf[0] = ENCRYPTED_CHAT_USER_DATA;
            buf[1] = crypto_kx_PUBLICKEYBYTES + 16;

            memcpy(buf + 2, me.keypair.pubkey, crypto_kx_PUBLICKEYBYTES);
            memcpy(buf + crypto_kx_PUBLICKEYBYTES + 2, me.name, 16);

            peer_sock->Write(buf, sizeof(buf));

        }

        peer_sock->SetEventHandler(*this, ID_Socket);
        peer_sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
        peer_sock->Notify(true);
        break;
    }
}

void AppFrame::OnSocketEvent(wxSocketEvent& event) {
    wxSocketBase* sockBase = event.GetSocket();
    wxUint32 lenRd;

    switch (event.GetSocketEvent()) {
    case wxSOCKET_INPUT:
        sockBase->SetNotify(wxSOCKET_LOST_FLAG);

        sockBase->SetFlags(wxSOCKET_WAITALL);

        unsigned char rx_buf[256];
        lenRd = sockBase->Read(rx_buf, 2).LastCount();

        if (lenRd) {
            switch (rx_buf[0]) {
            case ENCRYPTED_CHAT_USER_DATA:  
                if (!Peer.received_data) {
                    text_output->WriteText(wxNow() << " Received user data!" << '\n');

                    lenRd = sockBase->Read(rx_buf + 2, crypto_kx_PUBLICKEYBYTES + 16).LastCount();
                    memcpy(Peer.name, rx_buf + crypto_kx_PUBLICKEYBYTES + 2, 16);
                    text_output->WriteText(wxNow() << " New user: " << Peer.name << '\n');

                    bool status = is_server ? key_exchange_server(me, rx_buf + 2) : key_exchange_client(me, rx_buf + 2);
                    if (status == true) {
                        text_output->WriteText(wxNow() << " Successful key exchange! " << '\n');
                    }
                    else {
                        text_output->WriteText(wxNow() << " Failed to exchange keys! " << '\n');
                    }

                    unsigned char tx_buf[2 + crypto_kx_PUBLICKEYBYTES + 16];

                    tx_buf[0] = ENCRYPTED_CHAT_USER_DATA;
                    tx_buf[1] = crypto_kx_PUBLICKEYBYTES + 16;

                    memcpy(tx_buf + 2, me.keypair.pubkey, crypto_kx_PUBLICKEYBYTES);
                    memcpy(tx_buf + crypto_kx_PUBLICKEYBYTES + 2, me.name, 16);

                    sockBase->Write(tx_buf, sizeof(tx_buf));

                    Peer.received_data = true;

                    wxMessageBox("My public key: " + get_hex_key(me.keypair.pubkey, crypto_kx_PUBLICKEYBYTES) + "\nPeer's public key: " + get_hex_key(rx_buf + 2, crypto_kx_PUBLICKEYBYTES) + 
                        "\nTX key: " + get_hex_key(me.tx, crypto_kx_SESSIONKEYBYTES) + "\nRX key : " + get_hex_key(me.rx, crypto_kx_SESSIONKEYBYTES), "Key exchange info", wxOK | wxICON_INFORMATION);
                }
                break;
            case ENCRYPTED_CHAT_MESSAGE:
                TransportCipher rx_cipher;

                lenRd = sockBase->Read(&rx_cipher, sizeof(TransportCipher)).LastCount();

                me.decrypt_message(rx_cipher);
                
                text_output->WriteText(wxNow() << ' ' << Peer.name << ": " << rx_cipher.data << '\n');
                break;

            }
        }

        sockBase->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
        break;
    case wxSOCKET_LOST:
        sockBase->Destroy();
        break;
    }
}

AppFrame::AppFrame(wxString hostname) : wxFrame(nullptr, wxID_ANY, L"Encrypted Chat") {

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    wxPanel* panel = new wxPanel(this, wxID_ANY);

    text_output = new wxTextCtrl(panel, ID_Output, wxT(""),
        wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_WORDWRAP);
    vbox->Add(text_output, 4, wxEXPAND | wxTOP);

    text_input = new wxTextCtrl(panel, ID_Input, wxT(""),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    vbox->Add(text_input, 1, wxEXPAND | wxBOTTOM);

    panel->SetSizer(vbox);

    CreateStatusBar();

    SetStatusText(hostname);

    wxIPV4address addr;
    peer_sock = client_sock = NULL;

    addr.AnyAddress();
    addr.Service(28015);

    if (hostname != "localhost") {
        is_server = false;

        addr.Hostname(hostname);

        client_sock = new wxSocketClient();

        client_sock->SetEventHandler(*this, ID_Socket);
        client_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
            wxSOCKET_INPUT_FLAG |
            wxSOCKET_LOST_FLAG);
        client_sock->Notify(true);

        client_sock->Connect(addr, false);

    }
    else {
        is_server = true;

        server_sock = new wxSocketServer(addr);

        server_sock->SetEventHandler(*this, ID_Server);
        server_sock->SetNotify(wxSOCKET_CONNECTION_FLAG);
        server_sock->Notify(true);
    }

    Bind(wxEVT_TEXT_ENTER, &AppFrame::OnSend, this, ID_Input);
    Center();
}

void AppFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

bool EncryptedChatApp::OnInit()
{
    sodium_init();
    me = User();
    strcpy(me.name, "debug");

    wxString addr;
    if (wxApp::argc > 1) {
        addr = wxApp::argv[1];
    }
    else {
        addr = "localhost";
    }
    // addr = "172.20.128.178";
    AppFrame* mainFrame = new AppFrame(addr);

    mainFrame->Show(true);

    return true;
}

wxBEGIN_EVENT_TABLE(AppFrame, wxFrame)
EVT_SOCKET(ID_Server, AppFrame::OnServerEvent)
EVT_SOCKET(ID_Socket, AppFrame::OnSocketEvent)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(EncryptedChatApp);

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
