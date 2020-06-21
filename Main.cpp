#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

#include "cipher_tool/cipher.h"
#include "frequential_analysis/kasiski.h"
#include "frequential_analysis/keylength.h"

#define MIN_ARGC 4
#define MAX_ARGC 5

static int requireValidFileDescriptor(const char *path, int flags)
{
    int fd = open(path, flags);
    if (fd == -1)
    {
        perror(strerror(errno));
        exit(1);
    }
    return fd;
}

// Converts the given number to the letter of the latin alphabet corresponding.
static char toLetter(unsigned index)
{
    char letters[26] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q',
        'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
    return letters[index];
}

// Prints the given vector of keys as a string.
static void printKey(std::vector<unsigned> keys)
{
    std::cout << "The key is \"";
    for (const auto &key : keys)
        std::cout << toLetter(key);
    std::cout << "\"" << std::endl;
}

/* Compilation line
 * g++ Main.cpp frequential_analysis/kasiski.cpp frequential_analysis/keylength.cpp cipher_tool/cipher.c
-o vigenere
 */

/* argv contains (in the following order):
 * - argv[1]: 
 *  - "cipher" to cipher the content of the source file with a key.
 *  - "unipher" to uncipher the content of the source file with a key.
 *  - "attack" to uncipher the content of the source file without a key.
 * - argv[2] contains the path to the file where to store the result.
 * - argv[3] contains the key when needed.
 */
int main(int argc, const char *argv[])
{
    if (argc < MIN_ARGC)
    {
        std::cout << "usage: ./sec <cipher | uncipher | attack> <source> <destination> [key]\n";
        exit(3);
    }

    const char *action_type = argv[1];
//    const char *cipher_type = argv[2];
    const char *source_path = argv[2];
    const char *destination_path = argv[3];
    const char *key = argv[4];
    const int key_len = strlen(key);

    int src = requireValidFileDescriptor(source_path, O_RDWR);
    int dest = open(destination_path, O_WRONLY | O_CREAT, 0666);

    if (strcmp("attack", action_type) == 0)
    {
        std::cout << "Attacking " << source_path << "...\n";
        // Vigenere attack
        std::vector<unsigned> keys = findKey(src, dest);
        printKey(keys);
        unsigned *k = &keys[0];
        uncipher(src, dest, k, keys.size());
        std::cout << destination_path << " contains the unciphered text.\n";
    }
    else if (strcmp("cipher", action_type) == 0 || strcmp("uncipher", action_type) == 0)
    {
        unsigned keys[key_len];
        keyToValues(key, keys);
        if (strcmp("cipher", action_type) == 0)
        {
            cipher(src, dest, keys, key_len);
            std::cout << "Ciphered text of " << source_path << " is in " << destination_path << ".\n";
        }
        else if (strcmp("uncipher", action_type) == 0)
        {
            uncipher(src, dest, keys, key_len);
            std::cout << "Unciphered text of " << source_path << " is in " << destination_path << ".\n";
        }
    }
    else
    {
        std::cout << "Unkown type of action! Here are your options: cipher";
        std::cout << " uncipher or attack.";
        exit(1);
    }
    close(src);
    close(dest);
    exit(0);
}
