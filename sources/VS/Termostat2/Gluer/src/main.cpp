#include <string>
#include <fstream>
#include <vector>


using namespace std;


typedef unsigned char uint8;


static void AppendFile(vector<uint8_t> &bytes, const string &file_name, unsigned int address);


int main()
{
    static string path("c:/msys32/home/Sasha/esp/Termostat2/build/");         // Путь к каталогу файлов
    static string name0x00000("bootloader/bootloader.bin");
    static string name0x08000("partitions_singleapp.bin");
    static string name0x10000("Termostat2.bin");

    vector<uint8_t> bytes;

    string *names[3] = { &name0x00000, &name0x08000, &name0x10000 };
    unsigned int addresses[3] = { 0x00000, 0x08000, 0x10000 };

    for (int i = 0; i < 3; i++)
    {
        AppendFile(bytes, path + *names[i], addresses[i]);
    }

    ofstream file("Termostat_full.bin", ios::out | ios::binary);

    file.write((char *)bytes.data(), bytes.size());

    file.close();

    return 0;
}


static void AppendFile(vector<uint8_t> &bytes, const string &file_name, unsigned int address)
{
    while (bytes.size() < address)
    {
        bytes.push_back(0xFF);
    }

    ifstream file;

    file.open(file_name.c_str(), ios::in | ios::binary);

    if (file.is_open())
    {
        int counter = 0;

        while (!file.eof())
        {
            counter++;
            char byte = 0;
            file.read(&byte, 1);
            bytes.push_back((uint8)byte);
        }
    }
}