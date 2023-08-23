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

    vector<tspacket> tsstream;

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

    uint8_t sync_byte;
    tspacket *packet = new tspacket();

    while (ifile.read((char *)&sync_byte, 1)) {

        if (sync_byte != 0x47){
            cout<<"sync byte error detected"<<endl;
            continue;
        }

        uint8_t char_packet[188];
        char_packet[0] = sync_byte;
        ifile.read((char *)(char_packet+1), 187);

        *packet = *(tspacket *)char_packet;

        tsstream.push_back(*packet);

        ofile.write((char *)packet, 188);
    }

    unordered_map<uint16_t, vector<uint8_t *> > ts_pack_contents;
    for(auto p: tsstream){
        ts_pack_contents[p.pid].push_back(p.content);
    }

    unordered_map<uint8_t, vector<pespacket> > pes_packets;

    for(auto tsstream: ts_pack_contents){
        vector<uint8_t *> contents_184 = tsstream.second;
        int rem_payload_bytes = -1;
        for(auto content_ptr: contents_184){
            for(int i=0;i<184;i++){
                uint8_t *content_byte_ptr = content_ptr + i;
                if(*(uint32_t *)content_byte_ptr>>8 == 1){
                    pespacket pack;
                    pack.start_prefix = *(uint32_t *)content_byte_ptr >> 8;
                    pack.stream_id = *(uint32_t *)content_byte_ptr & 255;
                    pack.packet_length = ((uint16_t *)content_byte_ptr)[2] & 0xffff;
                    pack.flags1 = content_byte_ptr[6];
                    pack.flags2 = content_byte_ptr[7];
                    pack.header_length = content_byte_ptr[8];

                    // payload start byte = i + 9 + pack.flags2>>7 + pack.flags2
                }
            }
        }
    }

    ifile.close();
    ofile.close();
    return 0;
}