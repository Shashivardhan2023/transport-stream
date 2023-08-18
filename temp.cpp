#include <iostream>
#include <fstream>

using namespace std;

struct tspacket
{
    uint8_t syncbyte;
    uint8_t t_err_indicator:1;
    uint8_t payload_unit_start_indicator:1;
    uint8_t transport_priority:1;
    uint16_t pid:13;
    uint8_t transport_scrambling_control:2;
    uint8_t adaptation_field_control:2;
    uint8_t continuity_count:4;
    uint8_t content[184];
};

int main(int argc, char **argv) {
    vector<string> args(argv, argv+argc);

    vector<tspacket> stream;

    ifstream ifile(args[1], ios::binary);
    ofstream ofile(args[2]);
    
    if (!ifile.is_open()) {
        cerr << "Error opening file-1: " << args[0] << endl;
        return 1;
    }
    if (!ofile.is_open()) {
        cerr << "Error opening file-2: " << args[1] << endl;
        return 1;
    }
    char ch_packet[188];
    tspacket *packet = new tspacket();
    while (ifile.read(ch_packet, 188)) {
        packet->syncbyte = (uint8_t)ch_packet[0];
        packet->t_err_indicator = ch_packet[1]>>7 & 0x1;
        packet->payload_unit_start_indicator = ch_packet[1]>>6 & 0x1;
        packet->transport_priority = ch_packet[1]>>5 & 0x1;
        packet->pid = ( (ch_packet[1] & 0x1f)<<8 ) & ch_packet[2];
        packet->transport_scrambling_control = ch_packet[3]>>6 & 0x3;
        packet->adaptation_field_control = ch_packet[3]>>4 & 0x3;
        packet->continuity_count = ch_packet[3] & 0xf;
        *packet->content = ch_packet[4];

        if(packet->syncbyte != 0x47){
            cout<<"sync byte error in packet number "<<(stream.size()+1)<<endl;
        }
        stream.push_back(*packet);
    }

    cout<<stream.size()<<endl;

    ifile.close();
    ofile.close();
    return 0;
}