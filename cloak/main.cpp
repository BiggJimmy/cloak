#include <cstdint>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

/*
* static uint8_t magic[4] = { 'A', 'L', 'R', 'M' };
*/

std::vector<uint8_t> read_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
    {
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> content(size);
    if (!file.read(reinterpret_cast<char*>(content.data()), size))
    {
        return {};
    }

    return content;
}
bool write_file(const std::string& path, const std::vector<uint8_t>& content)
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
    {
        return false;
    }

    file.write(reinterpret_cast<const char*>(content.data()), content.size());

    return file.good();
}

uint32_t crc32(const std::vector<uint8_t>& data)
{
    static const uint32_t polynomial = 0xEDB88320;

    uint32_t table[256];

    for (uint32_t i = 0; i < 256; ++i)
    {
        uint32_t c = i;

        for (uint32_t j = 0; j < 8; ++j)
        {
            c = (c & 1) ? (polynomial ^ (c >> 1)) : (c >> 1);
        }

        table[i] = c;
    }

    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < data.size(); ++i)
    {
        crc = (crc >> 8) ^ table[(crc ^ data[i]) & 0xFF];
    }

    return crc ^ 0xFFFFFFFF;
}
std::vector<uint8_t> xor_cipher(std::vector<uint8_t> data, uint8_t key)
{
    for (auto& byte : data)
    {
        byte ^= key;
    }

    return data;
}

int craft_cloak64_v2(const std::string& input_path, const std::string& output_path, uint32_t sig, uint8_t key)
{
    std::vector<uint8_t> header(0x200, 0x00);
    std::vector<uint8_t> content = read_file(input_path);

    if (content.empty())
    {
        std::cerr << "Failed to read input file: " << input_path << std::endl;
        return -1;
    }

    uint32_t content_size = static_cast<uint32_t>(content.size());
    uint32_t header_size = static_cast<uint32_t>(header.size());
    uint32_t checksum = crc32(content);

    *reinterpret_cast<uint32_t*>(&header[0]) = sig;
    *reinterpret_cast<uint32_t*>(&header[4]) = content_size;
    *reinterpret_cast<uint32_t*>(&header[8]) = header_size;
    *reinterpret_cast<uint32_t*>(&header[12]) = checksum;

    content = xor_cipher(content, key);

    std::vector<uint8_t> result = header;
    result.insert(result.end(), content.begin(), content.end());

    if (!write_file(output_path, result))
    {
        std::cerr << "Failed to write output file: " << output_path << std::endl;
        return -2;
    }

    std::cout << "Craft complete. Output written to " << output_path << std::endl;
    return 0;
}
int uncraft_cloak64_v2(const std::string& input_path, const std::string& output_path, uint32_t sig, uint8_t key)
{
    std::vector<uint8_t> content = read_file(input_path);

    if (content.empty())
    {
        std::cerr << "Failed to read input file: " << input_path << std::endl;
        return -1;
    }

    if (content.size() <= 0x200)
    {
        std::cerr << "File too small to uncraft (must be > 0x200 bytes)." << std::endl;
        return -2;
    }

    if (*reinterpret_cast<uint32_t*>(content.data()) != sig)
    {
        std::cerr << "Invalid file format: signature not found." << std::endl;
        return -3;
    }

    std::vector<uint8_t> result(content.begin() + 0x200, content.end());
    result = xor_cipher(result, key);

    if (!write_file(output_path, result))
    {
        std::cerr << "Failed to write output file: " << output_path << std::endl;
        return -4;
    }

    std::cout << "Uncraft complete. Output written to " << output_path << std::endl;
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::cout
            << "Usage:\n"
            << "  " << argv[0] << " craft <input_file> <output_file> [sig] [key]\n"
            << "  " << argv[0] << " uncraft <input_file> <output_file> [sig] [key]\n"
            << "\n"
            << "Parameters:\n"
            << "  <input_file>   Path to the input file\n"
            << "  <output_file>  Path to the output file\n"
            << "  [sig]          Optional file signature (default: 0x4D524C41)\n"
            << "  [key]          Optional XOR encryption key (default: 0xB3)\n"
            << "\n"
            << "Examples:\n"
            << "  " << argv[0] << " craft payload.efi cloak64.dat\n"
            << "  " << argv[0] << " craft payload.efi cloak64.dat 0x4D524C41 0xB3\n"
            << "  " << argv[0] << " uncraft cloak64.dat restored.efi\n"
            << "  " << argv[0] << " uncraft cloak64.dat restored.efi 0x4D524C41 0xB3\n";

        return 1;
    }

    std::string mode = argv[1];
    std::string input_path = argv[2];
    std::string output_path = argv[3];

    /* Configuration Options */
    uint32_t sig = argc > 4 ? static_cast<uint32_t>(std::stoul(argv[4], nullptr, 0)) : 0x4D524C41;
    uint8_t key = argc > 5 ? static_cast<uint8_t>(std::stoul(argv[5], nullptr, 0)) : 0xB3;

    if (mode == "craft")
    {
        return craft_cloak64_v2(input_path, output_path, sig, key);
    }
    else if (mode == "uncraft")
    {
        return uncraft_cloak64_v2(input_path, output_path, sig, key);
    }
    else
    {
        std::cerr
            << "Unknown mode: " << mode << "\n"
            << "Valid modes: craft, uncraft\n";

        return -600;
    }

    return 0;
}