/*
    kryptonite - A simple cURL client, made for fun in a few hours :)
    Copyright (C) 2023  Unified

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <getopt.h>

// ANSI escape codes for text color
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Callback function to write data received from the HTTP server to a file and/or the console.
size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
    FILE *output_file = (FILE *)userdata;
    size_t written = fwrite(ptr, size, nmemb, output_file);
    size_t written_to_console = fwrite(ptr, size, nmemb, stdout);
    return written; // Return the number of bytes written to the file
}

// Callback function to display download progress.
int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    if (dltotal > 0) {
        double progress = (double)dlnow / (double)dltotal * 100.0;
        int bar_width = 50;
        int progress_width = (int)(progress * bar_width / 100.0);

        printf("\r[");
        for (int i = 0; i < bar_width; i++) {
            if (i < progress_width) {
                printf(ANSI_COLOR_GREEN "#");
            } else {
                printf(ANSI_COLOR_RED "-");
            }
        }
        printf(ANSI_COLOR_RESET "] %.2f%%", progress);
        fflush(stdout);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char *url = NULL;
    const char *output_file_path = "output.txt";
    const char *http_method = "GET";
    const char *post_data = NULL;
    struct curl_slist *headers = NULL;
    const char *certificate_path = NULL;

    int option;
    while ((option = getopt(argc, argv, "u:f:m:d:H:c:hv")) != -1) {
        switch (option) {
            case 'u':
                url = optarg;
                break;
            case 'f':
                output_file_path = optarg;
                break;
            case 'm':
                http_method = optarg;
                break;
            case 'd':
                post_data = optarg;
                break;
            case 'H':
                headers = curl_slist_append(headers, optarg);
                break;
            case 'c':
                certificate_path = optarg;
                break;
            case 'v':
                printf("Running kryptonite v1.0 by unium @ unified\n"); // Display version information
                break;
            case 'h':
                // Display help menu
                printf("\n------------------------------------------------------------------------------------\n");
                printf("kryptonite v1.0 by unium @ unified\n");
                printf("Description : A simple cURL client, made for fun in a few hours :)\n");
                printf("Usage: kryptonite -u <URL> [options]\\n");
                printf("Options:\n");
                printf("-h                Displays this menu.\n");
                printf("-v                Displays the version of this command.\n");
                printf("-u                URL to be cURL'd.\n");
                printf("-f                Output file path.\n");
                printf("-m                HTTP method.\n");
                printf("-d                POST data.\n");
                printf("-H                Header(s).\n");
                printf("-c                Certificate path.\n");
                printf("------------------------------------------------------------------------------------\n\n");
                break;
            default:
                fprintf(stderr, ANSI_COLOR_RED "Usage: %s -u <URL> [-h <help_menu>] [-v <kryptonite_version>] [-f <output_file_path>] -m <HTTP_method> [-d <POST_data>] [-H <Header>] [-c <certificate_path>]\n" ANSI_COLOR_RESET, argv[0]);
                return 1;
        }
    }

    if (!url) {
        fprintf(stderr, ANSI_COLOR_RED "Please specify a URL using the -u option.\n" ANSI_COLOR_RESET);
        return 1;
    }

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        FILE *output_file = fopen(output_file_path, "wb");
        if (!output_file) {
            fprintf(stderr, ANSI_COLOR_RED "Failed to open the output file.\n" ANSI_COLOR_RESET);
            return 1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, output_file);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, http_method);

        if (post_data) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // SSL/TLS options
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Enable peer certificate verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // Check the certificate's CN and verify hostname
        if (certificate_path) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, certificate_path); // Path to user-specified CA certificate bundle
        }

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            printf("\nOutput saved to: %s\n", output_file_path);
        } else {
            fprintf(stderr, ANSI_COLOR_RED "\ncurl_easy_perform() failed: %s\n" ANSI_COLOR_RESET, curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        fclose(output_file);
    }

    return 0;
}
