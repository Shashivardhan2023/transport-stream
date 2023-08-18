#include <iostream>
#include <fstream>

using namespace std;

struct tspacket
{
    char syncbyte;
    char t_err_indicator:1;
    char payload_unit_start_indicator:1;
    char transport_priority:1;
    char16_t pid:13;
    char transport_scrambling_control:2;
    char adaptation_field_control:2;
    char continuity_count:4;
    char content[184];
};

int main(int argc, char **argv) {
    vector<string> args(argv, argv+argc);

    vector<tspacket *> stream;

    ifstream ifile(args[1], ios::binary);
    ofstream ofile(args[2]);
    
    if (!ifile.is_open()) {
        cerr << "Error opening " << args[0] << endl;
        return 1;
    }
    if (!ofile.is_open()) {
        cerr << "Error opening " << args[1] << endl;
        return 1;
    }
    char byte;
    int packet_bytes = 188;
    while (ifile.read(&byte, 1)) {
        // for (int i = 1; i >= 0; --i) {
        //     ofile << hex << ((byte >> i*4) & 15);
        // }
        // ofile << " ";
        if(packet_bytes == 188 && byte!=0x47){

        }
    }

    ifile.close();
    ofile.close();
    return 0;
}