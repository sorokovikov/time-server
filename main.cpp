#include <iostream>
#include <string>

#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "boost/date_time/local_time/local_time.hpp"

#define BACKLOG 10

using namespace boost::local_time;
using namespace boost::posix_time;


const int PORT = 8000;
const int BUFFER_SIZE = 128;


std::string removeEscapeCharacters(std::string str) { 
    str.erase(remove(str.begin(), str.end(), '\n'), str.end()); 
    str.erase(remove(str.begin(), str.end(), '\r'), str.end()); 
    return str; 
}


std::string formatTime(local_date_time current_time) {
    std::stringstream ss;
    local_time_facet* output_facet = new local_time_facet();
    ss.imbue(std::locale(std::locale::classic(), output_facet));

    output_facet->format("%a %b %d %H:%M:%S %z %Y\n");
    ss.str();
    ss << current_time;
    return ss.str();
}


time_zone_ptr getTimeZonePtr(tz_database tz_db, std::string abbreviation) {
    time_zone_ptr tz;

    for (std::string &region : tz_db.region_list()) {
        tz = tz_db.time_zone_from_region(region);
        if (tz->std_zone_abbrev() == abbreviation || tz->dst_zone_abbrev() == abbreviation) {
            return tz;
        }
    }

    return NULL;
}


int main(int argc, const char** argv) {
    tz_database tz_db;
    tz_db.load_from_file("./date_time_zonespec.csv");
    
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_socket == -1) {
        printf("Error creating socket.\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &addr, sizeof(addr)) == -1) {
        printf("Couldn't set address to reuse.\n");
    }

    if (bind(listen_socket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        printf("Error binding port.\n");
        return 1;
    }

    if (listen(listen_socket, BACKLOG) == -1) {
        printf("Error setting socket to listen.\n");
        return 1;
    }

    while(true) {
        printf("Waiting for connection.\n");

        int accept_socket = accept(listen_socket, NULL, NULL);
        if (accept_socket == -1) {
            printf("Error accepting request.\n");
            return 1;
        }

        char buffer[BUFFER_SIZE];
        recv(accept_socket, buffer, BUFFER_SIZE, 0);

        std::string abbreviation(buffer);
        abbreviation = removeEscapeCharacters(abbreviation);

        time_zone_ptr tz = getTimeZonePtr(tz_db, abbreviation);
        
        if (tz == NULL) {
            std::string data = "Abbreviation is not found.\n";
            send(accept_socket, data.c_str(), data.size(), 0);
        }
        else {
            abbreviation = tz->std_zone_abbrev();
            ptime now = second_clock::universal_time();
            local_date_time current_time(now, tz);

            std::string data = formatTime(current_time);
            send(accept_socket, data.c_str(), data.size(), 0);
        }
        printf("Send response.\n");

        close(accept_socket);
    }

    return 0;
}
