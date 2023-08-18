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

struct pespacket
{
    uint32_t start_prefix:24;
    uint8_t stream_id;
    uint16_t packet_length;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t header_length;
    uint8_t *pts, *dts, *optional, *payload;
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
        for(int i=0;i<188;i++){
            for(int j=7;j>=0;j--){
                ofile << ((ch_packet[i]>>j) & 1);
            }
            ofile << endl;
        }

        packet->syncbyte = ch_packet[0];
        packet->t_err_indicator = ch_packet[1]>>7 & 0x1;
        packet->payload_unit_start_indicator = ch_packet[1]>>6 & 0x1;
        packet->transport_priority = ch_packet[1]>>5 & 0x1;
        packet->pid = ( (ch_packet[1] & 0x1f)<<8 ) | ch_packet[2];
        packet->transport_scrambling_control = ch_packet[3]>>6 & 0x3;
        packet->adaptation_field_control = ch_packet[3]>>4 & 0x3;
        packet->continuity_count = ch_packet[3] & 0xf;
        *packet->content = ch_packet[4];

        if(packet->syncbyte != 0x47){
            cout<<"sync byte error detected"<<endl;
        }else{
            stream.push_back(*packet);
        }
    }

    unordered_map<uint16_t, vector<uint8_t *> > ts_pack_contents; 
    for(int i=0;i<stream.size();i++){
        tspacket p = stream[i];
        ts_pack_contents[p.pid].push_back(p.content);
    }

    ifile.close();
    ofile.close();
    return 0;
}