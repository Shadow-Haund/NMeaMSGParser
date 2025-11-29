#include "gnss_parser.hpp"
#include <optional>


int main(int argc, char* argv[]){

    GnssParser parser;
    std::string pathToInput;

    if (argc > 1) pathToInput = argv[1];
    else pathToInput = "data/text.txt";
    
    std::ifstream dataFile(pathToInput);
    std::string fileLineStr;
    std::vector<std::string> msgLines;
    std::string header;
    std::stringstream ss;

    while(getline(dataFile, fileLineStr)){
        msgLines.push_back(fileLineStr);
    }
    for (int i = 0; i < msgLines.size(); i++){
        header = parser.parseHeader(msgLines.at(i));
        parser.processMsg(header, msgLines.at(i));
    }

    return 0;
}

