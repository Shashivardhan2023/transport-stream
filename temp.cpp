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

    string serialize(){
        string stream;
        stream.append((char *)this, 188);

        return stream;
    }
};

struct pespacket
{
    uint32_t start_prefix:24;
    uint8_t stream_id;
    uint16_t packet_length;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t header_length;
    uint8_t *optional, *payload;

    string serialize(){
        string stream;
        stream.insert(0, *(string *)this, 9);
        stream.insert(9, *(string *)this->optional, header_length);
        stream.insert(9 + header_length, *(string *)this->payload, packet_length - header_length - 9);

        return stream;
    }
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
            cout<<"sync byte error"<<endl;
            continue;
        }

        uint8_t char_packet[188];
        char_packet[0] = sync_byte;
        ifile.read((char *)(char_packet+1), 187);

        *packet = *(tspacket *)char_packet;

        tsstream.push_back(*packet);

        ofile << packet->serialize().substr(4);
    }

    unordered_map<uint16_t, vector<uint8_t *> > ts_pack_contents;
    for(auto p: tsstream){
        ts_pack_contents[p.pid].push_back(p.content);
    }

    unordered_map<uint16_t, string> dmux_pes_stream;

    for(auto stream: ts_pack_contents){
        int rem_length = -1;
        string p;
        for(auto _184_bytes_ptr: stream.second){
            if(rem_length == -1){
                uint32_t prefix_code = (*(uint32_t *)_184_bytes_ptr) & 0xffffff;
                if(prefix_code != 0x1){
                    cout<<"error PES packet prefix code is not 0x1"<<endl;
                    continue;
                }else{
                    rem_length = *((uint8_t *)_184_bytes_ptr + 8);
                }
            }
            int curr_req = (rem_length>184)?184:rem_length;
            string temp((char *)_184_bytes_ptr, 184);
            p.append(temp, 184 - curr_req, curr_req);
        }
        dmux_pes_stream[stream.first] = p;
    }

    ifile.close();
    ofile.close();
    return 0;
}