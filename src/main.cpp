#include "../include/gnss_parser.h"
#include <optional>


int main(){

    GnssParser parser;

    std::ifstream data_file("../data/text.txt");
    std::string file_line_str;
    std::vector<std::string> msg_lines;
    std::string header;
    std::stringstream ss;

    while(getline(data_file, file_line_str)){
        msg_lines.push_back(file_line_str);
    }
    for (int i = 0; i < msg_lines.size(); i++){
        header = parser.parseHeader(msg_lines.at(i));
        parser.parseMsg(header, msg_lines.at(i));
    }

    return 0;
}