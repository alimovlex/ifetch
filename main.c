#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <wchar.h>

#define MAX_DATA_LENGTH 64
#define INTERFACE_SIZE 32
#define MAX_IPVX_NUM 8
#define MAX_IP_LENGTH 128

#define MAX_PATH_LENGTH 4096
#define MAX_FILENAME_LENGTH 256
#define MAX_INTERFACE_LENGTH 64

#define INTERFACES_PATH "/sys/class/net"

enum transmission_type {TX, RX};

static double double_from_file(double *dest, char *path) {
    FILE *fs = fopen(path, "r");
    if(fs == NULL)
        return -1;

    char number_str[32];
    number_str[0] = '\0';
    char *status = fgets(number_str, 32, fs);

    if(fgets(number_str, 32, fs))
        return -1;
    if(!strcmp(number_str, "\n") || !strcmp(number_str, "")) {
        // If the file is empty, the function failed
        fclose(fs);
        return -1;
    }
    fclose(fs);

    (*dest) = strtod(number_str, NULL);
    return *dest;
}

double get_bytes(double *dest, char *interface, enum transmission_type t) {
    char file_path[MAX_PATH_LENGTH];
    if(t == RX)
        sprintf(file_path, "%s/%s/%s", INTERFACES_PATH, interface, "/statistics/rx_bytes");
    else if(t == TX)
        sprintf(file_path, "%s/%s/%s", INTERFACES_PATH, interface, "/statistics/tx_bytes");

    double bytes = double_from_file(dest, file_path);
    return bytes;
}

void to_formatted_bytes(double bytes) {
    double approx_bytes = bytes;
    unsigned int divisions = 0;
    const char *suffixes[7] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};

    while(approx_bytes > 1e3 && divisions <= 7) {
        approx_bytes /= 1024;
        divisions++;
    }

    printf("%.2f %s\n", approx_bytes, suffixes[divisions]);
    //return approx_bytes;
}

int get_max_interface() {
    int res = 0;

    DIR *interfaces = opendir(INTERFACES_PATH);
    struct dirent *entry;

    char cur_opstate_path[MAX_PATH_LENGTH];
    char cur_opstate[8];

    double cur_rx_bytes;
    double cur_tx_bytes;

    while((entry = readdir(interfaces)) != NULL) {

        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            res++;
            continue;
        }
        sprintf(cur_opstate_path, "%s/%s/%s", INTERFACES_PATH, entry->d_name, "operstate");

        FILE *opstate_file = fopen(cur_opstate_path, "r");
        if(opstate_file == NULL) // If the operstate file is missing, continue
            continue;

        if(!fgets(cur_opstate, 8, opstate_file))
            continue;
        fclose(opstate_file);

        if(strcmp(cur_opstate, "up\n") != 0) // If interface is not up, continue
            continue;

        get_bytes(&cur_rx_bytes, entry->d_name, RX);
        get_bytes(&cur_tx_bytes, entry->d_name, TX);

        printf("INTERFACE: %s\n", entry->d_name);
        printf("RX: ");
        to_formatted_bytes(cur_rx_bytes);
        printf("TX: ");
        to_formatted_bytes(cur_tx_bytes);

    }
    closedir(interfaces);

    return res;
}

int main() {
    if(!(getenv("LANG") && setlocale(LC_ALL, getenv("LANG"))) &&
       !(getenv("LC_ALL") && setlocale(LC_ALL, getenv("LC_ALL")))) {
        // If no relevant locale environment variable is set, fall back to English
        setlocale(LC_ALL, "en_US.UTF-8");
    }

    //char *home_dir = getpwuid(getuid())->pw_dir;

    char interface[MAX_INTERFACE_LENGTH];
    interface[0] = '\0';

        int interface_counter = get_max_interface();
        //printf("Available interfaces: %d\n", interface_counter);
        if(interface_counter <= 0) {
            printf("No interface available\n");
            return -1;
        }

    /*
    get_mac(data[MAC_INDEX].data, interface);
    get_ip(&(data[IP4_INDEX].data), interface, IPv4);
    get_ip(&(data[IP6_INDEX].data), interface, IPv6);

    if(data[RX_INDEX].show && data[RX_INDEX].instances) to_formatted_bytes(data[RX_INDEX].data, MAX_DATA_LENGTH, rx);
    if(data[TX_INDEX].show && data[TX_INDEX].instances) to_formatted_bytes(data[TX_INDEX].data, MAX_DATA_LENGTH, tx);
    if(data[MAC_INDEX].show) data[MAC_INDEX].instances = get_mac(data[MAC_INDEX].data, interface);
    if(data[IP4_INDEX].show) data[IP4_INDEX].instances = get_ip(&(data[IP4_INDEX].data), interface, IPv4);
    if(data[IP6_INDEX].show) data[IP6_INDEX].instances = get_ip(&(data[IP6_INDEX].data), interface, IPv6);
    */
    return 0;
}
