#include "gnss_parser.hpp"

#include <optional>

// helper functions

std::string GnssParser::convertFormatUtc(const std::string& utc_str) {

    std::string hh = utc_str.substr(0, 2);
    std::string mm = utc_str.substr(2, 2);
    size_t dotPos = utc_str.find('.');
    std::string ss = utc_str.substr(4, dotPos == std::string::npos ? 2 : std::string::npos);

    return hh + ":" + mm + ":" + ss + "\n";
}

std::string GnssParser::convertLatLot(const std::string& latLong) {
    
    size_t dot = latLong.find('.');
    size_t minStart = (dot == std::string::npos) ? latLong.length() : dot;
    int degDigits = static_cast<int>(minStart) - 2;
    std::string degStr = latLong.substr(0, degDigits);
    std::string minStr = latLong.substr(degDigits);
    double totalDegrees = std::stod(degStr);
    double totalMinutes = std::stod(minStr);
    int minutes = static_cast<int>(totalMinutes);
    double seconds = (totalMinutes - minutes) * 60.0;
    int degrees = static_cast<int>(totalDegrees);

    std::ostringstream oss;
    oss << degrees << " Degrees, "
        << minutes << " Minutes, "
        << std::fixed << std::setprecision(5) << seconds << " Seconds ";

    return oss.str();
}

std::string GnssParser::convertDate(const std::string& date) {

    std::string dd = date.substr(0, 2);
    std::string mm = date.substr(2, 2);
    std::string yy = date.substr(4, 2);

    int year = std::stoi(yy);
    int fullYear = (year >= 80) ? 1900 + year : 2000 + year;

    return dd + "." + mm + "." + std::to_string(fullYear);

}

void GnssParser::correctGsvMsg(const std::string& msg, std::vector<std::string>& delimMsg){

    const auto& fieldMapGsv = MSG_TYPE_MAP_.at("GSV");

    int test_size = fieldMapGsv.size() - (delimMsg.size() + NO_CR_CL_IDX_);

    if (fieldMapGsv.size() != delimMsg.size() + NO_CR_CL_IDX_){
        for (int i = 0; i != test_size; i++){
            delimMsg.insert(delimMsg.end() - STABLE_DATA_FRROM_END_GSV_, "");
        }
    }
}
// main class methods

void GnssParser::processMsg(std::string header, std::string& msg){
    if (header.empty()) {
        std::cout << "Data is invalid. Header mistake.\n";
        return;
    }
    else if (std::regex_match(msg, gaaRegex_)) {
        std::cout << "Data is valid. GAA\n";
    }
    else if (std::regex_match(msg, gllRegex_)) {
        std::cout << "Data is valid. Gll\n";
    }
    else if (std::regex_match(msg, vtgRegex_)) {
        std::cout << "Data is valid. VTG\n";
    }
    else if (std::regex_match(msg, gstRegex_)) {
        std::cout << "Data is valid. GST\n";
    }
    else if (std::regex_match(msg, zdaRegex_)) {
        std::cout << "Data is valid. ZDA\n";
    }
    else if (std::regex_match(msg, dtmRegex_)) {
        std::cout << "Data is valid. DTM\n";
    }
    else if (std::regex_match(msg, gsaRegex_)) {
        std::cout << "Data is valid. GSA\n";
    }
    else if (std::regex_match(msg, gnsRegex_)) {
        std::cout << "Data is valid. GNS \n";
    }
    else if (std::regex_match(msg, gsvRegex_)) {
        std::cout << "Data is valid. GSV \n";
    }
    else if (std::regex_match(msg, rmcRegex_)) {
        std::cout << "Data is valid. RMC \n";
    }
    else {
        std::cout << "Data is invalid.\n";
        return;
    }

    splitNmeaMessage(msg);
}

std::string GnssParser::parseHeader(std::string& msg){
    int hEndIdx = msg.find_first_of(',');
    std::string msgTypeParsed = msg.substr(0, hEndIdx);
    return std::regex_match(msgTypeParsed, headerRegex_) ? msgTypeParsed : "";
}

void GnssParser::splitNmeaMessage(const std::string& msg) {
    std::vector<std::string> result;
    std::stringstream ss(msg);
    std::string field;

    while (std::getline(ss, field, ',')) {
        result.push_back(field);
    }   
    if (!result.empty()) {
        std::string& last = result.back();
        size_t pos = last.find('*');
        if (pos != std::string::npos) {
            std::string checksum = last.substr(pos);
            last = last.substr(0, pos);
            result.push_back(checksum);
        }
    }
    formatOutput(msg, result);
}

void GnssParser::formatOutput(const std::string& msg, std::vector<std::string>& vMsg){
    std::filesystem::path pathToOutput = "./data/output.txt";

    if (!std::filesystem::exists(pathToOutput.parent_path())) std::filesystem::create_directories(pathToOutput.parent_path());
    
    std::ofstream output;
    output.open(pathToOutput, std::ios::app);
    std::string msgHeaderType = vMsg[0].substr(3,6);
    std::string msgHeaderDevice = vMsg[0].substr(1,2);
    if (MSG_TYPE_MAP_.count(msgHeaderType)) {
        const auto& fieldMap = MSG_TYPE_MAP_.at(msgHeaderType);

        if (msgHeaderType == "GSV"){
            correctGsvMsg(msgHeaderType, vMsg);
        }

        output << "Message itself is " << msg << "\n";
        
        for (const auto& [index, name] : fieldMap) {
            bool isQualStatPosNavFlag = name == "quality" || name == "status" || name == "pos_mode" || name == "nav_mode";
            bool isFieldDataAvailable = !vMsg[index].empty();
            
            if (isFieldDataAvailable){
                if (index == 0){ 
                output << "Message header type is " << msgHeaderType << "\n";
                    if (msgHeaderDevice == "GP") output << "Message header device is GPS, SBAS or QZSS\n";
                    else if (msgHeaderDevice == "GL") output << "Message header device is GLONASS\n";
                    else if (msgHeaderDevice == "GA") output << "Message header device is Galileo\n";
                    else if (msgHeaderDevice == "GB") output << "Message header device is BeiDou\n";
                    else if (msgHeaderDevice == "GN") output << "Message header device is any combination of GNSS\n";
                }
                else if (name == "time") output << "UTC time stamp for this message is : " << convertFormatUtc(vMsg[index]);
                else if (name == "lat" || name == "lon") output << convertLatLot(vMsg[index]);
                else if (name == "ns" || name == "ew") output << "Direction " << vMsg[index] << " direction\n";
                else if (isQualStatPosNavFlag && (msgHeaderType == "GLL" || msgHeaderType == "RMC" || msgHeaderType == "VTG")){
                    if (vMsg[index] == "V") output << "Data is invalid\n";
                    else if (vMsg[index] == "A") output << "Data is valid\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "E" ) output << "Estimated/Dead reckoning\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "A" ) output << "Autonomous GNSS fix\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "D" ) output << "Differential GNSS fix\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "F" ) output << "RTK float\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "R" ) output << "RTK fixed\n";
                    else output << "Field is empty\n";
                    
                }
                else if (isQualStatPosNavFlag && msgHeaderType == "GGA") {
                    if (vMsg[index] == "1") output << "Autonomous GNSS fix\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "2") output << "Differential GNSS fix\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "4") output << "RTK fixed\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "5") output << "RTK float\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "6") output << "Estimated/Dead reckoning fix\n";
                    else output << "Field is empty\n";
                }
                else if (isQualStatPosNavFlag && msgHeaderType == "GSA") {
                    if (isQualStatPosNavFlag && vMsg[index] == "2") output << "2D fix\n";
                    else if (isQualStatPosNavFlag && vMsg[index] == "3") output << "3D fix\n";
                    else output << "Field is empty\n";
                }
                else if (isQualStatPosNavFlag && (vMsg[index] == "0" || vMsg[index] == "N" || vMsg[index] == "1")) output << "No fix\n";
                else if (name == "num_sv") output << "Number of used satellites : " << vMsg[index] << "\n";
                else if (name == "hdop") output << "Horizontal Dilution of Precision is : " << vMsg[index] << "\n";
                else if (name == "alt") output << "Altitude above mean sea level : " << vMsg[index] << " Unit of altitude is meter.\n";
                else if (name == "sep") output << "Difference between ellipsoid and mean sea level : " << vMsg[index] << " Unit sepparation is meter \n";
                else if (name == "diff_age") output << "Age of differential corrections is " << vMsg[index] << "if 0 then when DGPS is not used\n";
                else if (name == "diff_station") output << "ID of station providing differential corrections is " << vMsg[index]<< "if 0 then when DGPS is not used\n";
                else if (name == "cs") output << "Checksum is " << vMsg[index] << "\n\n";
                else if (name == "op_mode") output << (vMsg[index] == "M") ? "Manually set to operate in 2D or 3D mode \n" : "Automatically switching between 2D or 3D mode \n";
                else if (name == "svid") output << "Number of satelites is " << vMsg[index] << "\n";
                else if (name == "pdop") output << "Position dilution of precision is " << vMsg[index] << "\n";
                else if (name == "vdop") output << "Vertical dilution of precision is " << vMsg[index] << "\n";
                else if (name == "system_id") output << "NMEA-defined GNSS system ID " << vMsg[index] << "\n";
                else if (name == "num_msg") output << "Total number of GSV messages being output is  " << vMsg[index] << "\n";
                else if (name == "msg_num") output << "Number of this message specific message is  " << vMsg[index] << "\n";
                else if (name == "num_sv_v") output << "Number of known satellites in view regarding both the talker ID and the signalId is " << vMsg[index] << "\n";
                else if (name == "elv") output << "Elevation is " << vMsg[index] << "\n";
                else if (name == "az") output << "Azimuth " << vMsg[index] << "\n";
                else if (name == "cno") output << "Signal strength " << vMsg[index] << " if null when not tracking\n";
                else if (name == "signal_id") output << "NMEA-defined GNSS signal ID " << vMsg[index] << "\n";
                else if (name == "spd" || name == "sog_n") output << "Speed over ground in knots " << vMsg[index] << "\n";
                else if (name == "cog") output << "Course over ground in degree " << vMsg[index] << "\n";
                else if (name == "date") output << "Date in day, month, year format " << convertDate(vMsg[index]) << "\n";
                else if (name == "mv") output << "Magnetic variation value is " << vMsg[index] << "\n";
                else if (name == "mv_ew") output << "Magnetic variation E/W indicator is " << vMsg[index] << "\n";
                else if (name == "nav_status") output << "Navigational status indicator: Equipment is not providing navigational status information" << "\n";
                else if (name == "cog_t") output << "Course over ground " << vMsg[index];
                else if (name == "cog_t_unit" || name == "cog_m_unit") output << "Unit of cource over ground is degree \n";
                else if (name == "cog_m") output << "Course over ground (magnetic) " << vMsg[index] << "\n";
                else if (name == "sog_k") output << "Speed over ground in kilometers per hour " << vMsg[index] << "\n";
                else if (name == "range_rms") output << "RMS value of the standard deviation of the ranges " << vMsg[index] << "\n";//
                else if (name == "std_major") output << "Standard deviation of semi-major axis " << vMsg[index] << "\n";
                else if (name == "std_minor") output << "Standard deviation of semi-minor axis " << vMsg[index] << "\n";
                else if (name == "orient") output << "Orientation of semi-major axis " << vMsg[index] << "\n";
                else if (name == "std_lat") output << "Standard deviation of latitude error " << vMsg[index] << "\n";
                else if (name == "std_long") output << "Standard deviation of longitude error " << vMsg[index] << "\n";
                else if (name == "std_alt") output << "Standard deviation of altitude error " << vMsg[index] << "\n";
                else if (name == "day") output << "UTC day " << vMsg[index] << "\n";
                else if (name == "month") output << "UTC month " << vMsg[index] << "\n";
                else if (name == "year") output << "UTC year " << vMsg[index] << "\n";
                else if (name == "ltzh") output << "Local time zone hours, always 00 " << vMsg[index] << "\n";
                else if (name == "ltzn") output << "Local time zone minutes, always 00 " << vMsg[index] << "\n";
                else if (name == "datum") output << "Local datum code is "  << (vMsg[index] == "W84") ? "WGS84\n" :
                (vMsg[index] == "P90") ? "PZ90\n" : "user defined\n";
                else if (name == "sub_datum") output << "A null field "<< "\n";
                else if (name == "off_lat") output << "Offset in Latitude is " << vMsg[index] << "\n";
                else if (name == "off_lon") output << "Offset in Longitude is " << vMsg[index] << "\n";
                else if (name == "off_alt") output << "Offset in altitude is " << vMsg[index] << "\n";
                else if (name == "ref_datum") output << "Reference datum code " << vMsg[index] << "\n";
                else if (name == "pos_mode_4a"){ 
                    output << "Positioning mode, in the following order for GPS, GLONASS, Galileo and BeiDou " << vMsg[index] << "\n";
                    for (int i = 0; i < vMsg[index].size(); i++){
                        if (vMsg[index] == "N" ) output << "No fix";
                        else if (vMsg[index] == "E" ) output << "Estimated/Dead reckoning";
                        else if (vMsg[index] == "A" ) output << "Autonomous GNSS fix";
                        else if (vMsg[index] == "D" ) output << "Differential GNSS fix";
                        else if (vMsg[index] == "F" ) output << "RTK float";
                        else if (vMsg[index] == "R" ) output << "RTK fixed";
                    }
                }
            }

        }
    } else {
        std::cout << "Message type '" << msgHeaderType << "' is not supported.\n";
    }
    output.close();
}

