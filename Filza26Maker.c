#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Pretty colors on the screen, pretty colors everywhere.
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_BLUE    "\033[38;5;117m"
#define COLOR_ORANGE  "\033[1;33m"
#define COLOR_PINK "\033[1;35m"

int chkARAvailableAtPath() {
    return system("which ar > /dev/null 2>&1") == 0;
}

void rollieRollieRollieRollieWithADabOfRanch() {
    const char spinner[] = {'/', '-', '\\', '|'};
    int spinner_index = 0;

    system("xcode-select --install &");

    while (!chkARAvailableAtPath()) {
        printf("\r" COLOR_BLUE "[ i ] Waiting for Command Line Tools to finish installing... " COLOR_PINK "%c" COLOR_RESET,
               spinner[spinner_index]);
        fflush(stdout);
        spinner_index = (spinner_index + 1) % 4;
        usleep(150000);
    }

    printf("\r" COLOR_GREEN "[ + ] Toolchain installed successfully!                             \n" COLOR_RESET);
}

static int pbarCallBack(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    if (dltotal == 0) return 0;

    int bar_width = 40;
    float progress = (float)dlnow / (float)dltotal;
    int pos = (int)(bar_width * progress);

    float mb_now = (float)dlnow / (1024 * 1024);
    float mb_total = (float)dltotal / (1024 * 1024);

    printf("\r" COLOR_PINK "[");
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %3.0f%%  (%.1f MB / %.1f MB)" COLOR_RESET, progress * 100, mb_now, mb_total);
    fflush(stdout);

    return 0;
}

int tweakFetchUtils() {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "[-] Failed to initialize libcurl.\n");
        return 0;
    }

    FILE *fp = fopen("filza.deb", "wb");
    if (!fp) {
        fprintf(stderr, "[-] Failed to open file for writing.\n");
        curl_easy_cleanup(curl);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://tigisoftware.com/cydia/com.tigisoftware.filza_4.0.1-2_iphoneos-arm.deb");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, pbarCallBack);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Filza26Builder/1.0");

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);
    printf("\n");

    return res == CURLE_OK;
}

int extractTweakDEB() {
    return system("ar -x filza.deb") == 0;
}

int extractDATAFromDebTweak() {
    return system("tar -xzf data.tar.gz") == 0;
}

int moveFilzaToPayload() {
    system("mkdir -p Payload");

    if (system("cp -R Applications/Filza.app Payload/") == 0)
        return 1;
    
    return system("find . -type d -name Filza.app -exec cp -R {} Payload/ \\;") == 0;
}

int buildFinalFilzaIPA() {
    return system("zip -r ../Filza-Jailed-iOS26-GeoSn0w.ipa Payload > /dev/null 2>&1") == 0;
}

void nukeTempFiles() {
    chdir("..");
    system("rm -rf _GEO_TEMP");
}

int main() {
    printf("\033[8;36;96t");
    printf("\033[H\033[J");
    printf(COLOR_ORANGE);
    // I love ascii art ^_^
    printf("***************************************************\n");
    printf("*                Filza26 Builder                  *\n");
    printf("*              by GeoSn0w (@FCE365)               *\n");
    printf("* ----------------------------------------------- *\n");
    printf("*           https://idevicecentral.com            *\n");
    printf("* Thanks for watching iDevice Central on YouTube! *\n");
    printf("*                                                 *\n");
    printf("***************************************************\n\n");
    printf(COLOR_RESET);
    printf(COLOR_BLUE "[i] Checking toolchain availability...\n" COLOR_RESET);

    if (!chkARAvailableAtPath()) {
        printf(COLOR_RED "[!] Toolchain not installed. Downloading Command Line Tools...\n" COLOR_RESET);
        rollieRollieRollieRollieWithADabOfRanch();
        return 1;
    }

    printf(COLOR_BLUE "[i] Creating working directory...\n" COLOR_RESET);
    mkdir("_GEO_TEMP", 0755);
    if (chdir("_GEO_TEMP") != 0) {
        printf(COLOR_RED "[ - ] Failed to enter working directory.\n" COLOR_RESET);
        return 1;
    }

    printf(COLOR_BLUE "[i] Downloading Filza tweak DEB file...\n" COLOR_RESET);
    if (!tweakFetchUtils()) {
        printf(COLOR_RED "[-] Failed to download Filza DEB tweak.\n" COLOR_RESET);
        return 1;
    }
    printf(COLOR_GREEN "[+] Filza DEB download succeeded.\n" COLOR_RESET);

    printf(COLOR_GREEN "[+] Unpacking tweak data...\n" COLOR_RESET);
    if (!extractTweakDEB()) {
        printf(COLOR_RED "[-] Failed to extract DEB using ar.\n" COLOR_RESET);
        return 1;
    }

    if (!extractDATAFromDebTweak()) {
        printf(COLOR_RED "[-] Failed to extract data.tar.gz.\n" COLOR_RESET);
        return 1;
    }

    printf(COLOR_GREEN "[+] Building iOS 26-ready Filza Jailed...\n" COLOR_RESET);
    if (!moveFilzaToPayload()) {
        printf(COLOR_RED "[-] Failed to move Filza.app to Payload folder.\n" COLOR_RESET);
        return 1;
    }

    if (!buildFinalFilzaIPA()) {
        printf(COLOR_RED "[-] Failed to package IPA file.\n" COLOR_RESET);
        return 1;
    }
    printf(COLOR_BLUE "[i] Nuking any garbage / temporary files used...\n" COLOR_RESET);
    nukeTempFiles();
    
    printf(COLOR_GREEN "[+] Filza Jailed iOS 26 IPA created successfully.\n" COLOR_RESET);
    printf(COLOR_GREEN "[i] Please sign it with your favorite tool like Sideloadly, AltStore, etc. Enjoy!\n" COLOR_RESET);
    return 0;
}

