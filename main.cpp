#include <string>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <intrin.h>
#include <filesystem>

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

static void updatePal(fstream* stream);

int main() {
    for(const auto & file : recursive_directory_iterator("b2w2_rom")) {
        string pstr = file.path().string();
        string command = "tools\\ndstool.exe -x \"" + pstr + "\" -9 temp\\arm9.bin -7 temp\\arm7.bin -y9 temp\\y9.bin -y7 temp\\y7.bin -d data -y temp\\overlay -t temp\\banner.bin -h temp\\header.bin";
        cout << command << endl;
        system(command.c_str());
        break;
    }
    cout << "Preparing the ROM! (this may take awhile)" << endl;
    cout << "Replacing battle files..." << endl;
    system("tools\\knarc.exe -u .\\data\\a\\0\\0\\4 -d mon_data\\extracted");
    int c = 0;
    while(c < 14980) {
        int zeros;
        string cmd = "copy files\\";
        switch(c % 20) {
            case 0:
            case 1:
                cmd += "1_substitute_dex.bin ";
                break;
            case 2:
            case 3:
                cmd += "2_sprite.bin ";
                break;
            case 4:
                cmd += "4_recn.bin ";
                break;
            case 5:
                cmd += "5_cells.bin ";
                break;
            case 6:
                cmd += "6_rcmn.bin ";
                break;
            case 7:
                cmd += "7_ramn.bin ";
                break;
            case 8:
                cmd += "8_oam.bin ";
                break;
            case 11:
            case 12:
                cmd += "11_sprite_back.bin ";
                break;
            case 13:
                cmd += "13_recn.bin ";
                break;
            case 14:
                cmd += "14_cells.bin ";
                break;
            case 15:
                cmd += "15_rcmn.bin ";
                break;
            case 16:
                cmd += "16_nmar.bin ";
                break;
            case 17:
                cmd += "17_oam_back.bin ";
                break;
            case 18:
            case 19:
            {
                fstream bin;
                string path = ".\\mon_data\\extracted\\4_";
                zeros = 7 - floor(log10(c));
                for(int z = 0; z < zeros; z++) {
                    path += "0";
                }
                path += to_string(c);
                path += ".bin";
                bin.open(path, ios::binary|ios::in|ios::out|ios::ate);
                //cout << "Modifying file: " << path << endl;
                updatePal(&bin);
                bin.close();
                c++;
                continue;
            }
            default:
                c++;
                continue;
        }
        cmd += ".\\mon_data\\extracted\\4_";
        zeros = 7 - floor(log10(c));
        for(int z = 0; z < zeros; z++) {
            cmd += "0";
        }
        cmd += to_string(c);
        cmd += ".bin >nul 2>&1 ";
        system(cmd.c_str());
        c++;
    }
    system("del /f mon_data\\extracted\\4_0.bin >nul 2>&1"); //no idea how this file gets here but it messes things up LOL
    system("tools\\knarc.exe -p .\\data\\a\\0\\0\\4 -d mon_data\\extracted");
    cout << "Battle files updated!" << endl;
    
    cout << "Replacing Pokemon icons..." << endl;
    system("tools\\knarc.exe -u .\\data\\a\\0\\0\\7 -d .\\mon_data\\icons");
    c = 8;
    while(c < 1510) {
        
        int zeros;
        string cmd = "copy files\\plush_icon_rgcn.bin ";
        cmd += ".\\mon_data\\icons\\7_";
        zeros = 7 - floor(log10(c));
        for(int z = 0; z < zeros; z++) {
            cmd += "0";
        }
        cmd += to_string(c);
        cmd += ".bin >nul 2>&1 ";
        system(cmd.c_str());
        c++;
    }
    system("tools\\knarc.exe -p .\\data\\a\\0\\0\\7 -d .\\mon_data\\icons");
    cout << "Pokemon icons updated!" << endl;
    
    cout << "Removing type icons..." << endl;
    system("tools\\knarc.exe -u .\\data\\a\\0\\8\\2 -d .\\mon_data\\type_icons");
    c = 34;
    while(c < 56) {
        int zeros;
        string cmd = "copy files\\type_null.bin ";
        cmd += ".\\mon_data\\type_icons\\2_";
        zeros = 7 - floor(log10(c));
        for(int z = 0; z < zeros; z++) {
            cmd += "0";
        }
        cmd += to_string(c);
        cmd += ".bin >nul 2>&1 ";
        system(cmd.c_str());
        c++;
    }
    system("tools\\knarc.exe -p .\\data\\a\\0\\8\\2 -d .\\mon_data\\type_icons");
    cout << "Type icons removed!" << endl;
    
    cout << "Replacing text files..." << endl;
    system("tools\\knarc.exe -u .\\data\\a\\0\\0\\2 -d .\\mon_data\\text");
    system("copy .\\mon_data\\text_modified .\\mon_data\\text");
    system("tools\\knarc.exe -p .\\data\\a\\0\\0\\2 -d .\\mon_data\\text");
    cout << "Text files replaced!" << endl;
    
    string command = "tools\\ndstool.exe -c \"Pokemon_Plush_Mode.nds\" -9 temp\\arm9.bin -7 temp\\arm7.bin -y9 temp\\y9.bin -y7 temp\\y7.bin -d data -y temp\\overlay -t temp\\banner.bin -h temp\\header.bin";
    system(command.c_str());
    
}

typedef struct colour {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Colour;

inline unsigned char readChar(vector<unsigned char> binData, int c) {
    return binData[c];
}

int getDarkestIndexAfter(Colour cols[], int firstIndex) {
    int darkest = firstIndex;
    int darkness = cols[firstIndex].r + cols[firstIndex].b + cols[firstIndex].g;
    for(int c = firstIndex+1; c < 16; c++) {
        if(cols[c].r == 31 && cols[c].b == 31 && cols[c].g == 0) continue;
        int curDarkness = cols[c].r + cols[c].b + cols[c].g;
        if(curDarkness < darkness) {
            darkness = curDarkness;
            darkest = c;
        }
    }
    return darkest;
}

static void updatePal(fstream* pal) {
    pal->seekg(0, ios::end);
        
    streampos size = pal->tellg();
    
    pal->seekp(0);
    
    vector<unsigned char> binData(size);
    pal->read((char*) &binData[0], size);
    
    Colour cols[16];
    
    for(int c = 0; c < 16; c++) {
        //	Format is BGR555 (XBBBBBGGGGGRRRRR).
        unsigned char left = binData[0x28 + c*2];
        unsigned char right = binData[0x28 + c*2 + 1];
        //cout << +left << " " << +right << endl;
        unsigned short combined = (right << 8) | left;
        cols[c].b = (combined & 0x7C00) >> 10;
        cols[c].g = (combined & 0x3E0) >> 5;
        cols[c].r = (combined & 0x1F);
        //cout << combined << endl;
        //cout << (int) cols[c].r << " " << (int) cols[c].g << " " << (int) cols[c].b << endl;
    }
    
    int temp = getDarkestIndexAfter(cols, 1);
      
    swap(cols[1], cols[getDarkestIndexAfter(cols, 1)]);
    swap(cols[2], cols[getDarkestIndexAfter(cols, 2)]);
    swap(cols[5], cols[getDarkestIndexAfter(cols, 5)]);
    swap(cols[6], cols[getDarkestIndexAfter(cols, 6)]);
    
    int pos = 0x28;
    
    pal->seekp(pos);
    
    for(int c = 0; c < 16; c++) {
        unsigned short combined = (cols[c].b << 10) | (cols[c].g << 5) | cols[c].r;
        //combined = _byteswap_ushort(combined);     
        pal->seekp(pos);
        pal->write((char*) &combined, 2);
        pos+=2;
    }
}