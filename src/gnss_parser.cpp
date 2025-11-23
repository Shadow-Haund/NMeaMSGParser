#include "../include/gnss_parser.h"
#include <optional>

// helper functions

std::string GnssParser::formatUtcConverter(const std::string& utc_str) {

    std::string hh = utc_str.substr(0, 2);
    std::string mm = utc_str.substr(2, 2);
    size_t dot_pos = utc_str.find('.');
    std::string ss = utc_str.substr(4, dot_pos == std::string::npos ? 2 : std::string::npos);

    return hh + ":" + mm + ":" + ss + "\n";
}

std::string GnssParser::latLotConverter(const std::string& nmea) {
    
    if (nmea.empty()) {
        return "Field is empty";
    }

    // Найдём позицию десятичной точки
    size_t dot = nmea.find('.');
    size_t min_start = (dot == std::string::npos) ? nmea.length() : dot;
    // Градусы = всё, кроме последних 2 цифр до точки
    int deg_digits = static_cast<int>(min_start) - 2;
    // Извлекаем градусы и минуты
    std::string deg_str = nmea.substr(0, deg_digits);
    std::string min_str = nmea.substr(deg_digits);
    double total_degrees = std::stod(deg_str);
    double total_minutes = std::stod(min_str);
    int minutes = static_cast<int>(total_minutes);
    double seconds = (total_minutes - minutes) * 60.0;
    int degrees = static_cast<int>(total_degrees);

    std::ostringstream oss;
    oss << degrees << " Degrees, "
        << minutes << " Minutes, "
        << std::fixed << std::setprecision(5) << seconds << " Seconds ";

    return oss.str();
}

std::string GnssParser::dateConverter(const std::string& date) {

    std::string dd = date.substr(0, 2);
    std::string mm = date.substr(2, 2);
    std::string yy = date.substr(4, 2);

    int year = std::stoi(yy);
    int full_year = (year >= 80) ? 1900 + year : 2000 + year;

    return dd + "." + mm + "." + std::to_string(full_year);

}

// main class methods

std::vector<std::string> GnssParser::parseMsg(std::string header, std::string& msg){
    if (header.empty()) {
        std::cout << "Data is invalid. Header mistake.\n";
        return {};
    }
    else if (!std::regex_match(msg, gaa_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, gll_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, vtg_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, gst_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, zda_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, dtm_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, gsa_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, gns_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, gsv_regex)) {
        std::cout << "Data is valid.\n";
    }
    else if (!std::regex_match(msg, rmc_regex)) {
        std::cout << "Data is valid.\n";
    }
    else {
        std::cout << "Data is invalid.\n";
        return {};
    }

    return splitNmeaMessage(msg);
}

std::string GnssParser::parseHeader(std::string& msg){
    int h_end_idx = msg.find_first_of(',');
    std::string msg_type_parsed = msg.substr(0, h_end_idx);
    // std::string msg_data = msg.substr(h_end_idx+1, msg.length());
    return std::regex_match(msg_type_parsed, header_regex) ? msg_type_parsed : "";
}

std::vector<std::string> GnssParser::splitNmeaMessage(const std::string& msg) {
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
    fOutput(result);
    return result;
}

void GnssParser::fOutput(std::vector<std::string>& v_msg){
    std::ofstream output;
    std::string file_output = "../data/output.txt";
    output.open(file_output, std::ios::app);
    std::string msg_header_type = v_msg[0].substr(3,6);
    std::string msg_header_device = v_msg[0].substr(1,2);
    if (msg_type_map.count(msg_header_type)) {
        const auto& field_map = msg_type_map.at(msg_header_type);

        for (const auto& [index, name] : field_map) {
            bool dev_idx = index == 1;
            bool qual_stat_pos_nav_flag = name == "quality" || name == "status" || name == "pos_mode" || name == "nav_mode";
            bool op_mode_flag = name == "op_mode";
            bool datum_flag = name == "datum";
            bool data_available = !v_msg[index].empty();
            
            if (data_available){
                if (index == 0) output << "Message header type is " << msg_header_type << "\n";
                else if (msg_header_device == "GP" && dev_idx) output << "Message header device is GPS, SBAS or QZSS\n";
                else if (msg_header_device == "GL" && dev_idx) output << "Message header device is GLONASS\n";
                else if (msg_header_device == "GA" && dev_idx) output << "Message header device is Galileo\n";
                else if (msg_header_device == "GB" && dev_idx) output << "Message header device is BeiDou\n";
                else if (msg_header_device == "GN" && dev_idx) output << "Message header device is any combination of GNSS\n";
                else if (name == "time") output << "UTC time stamp for this message is : " << formatUtcConverter(v_msg[index]);
                else if (name == "lat" || name == "lon") output << latLotConverter(v_msg[index]);
                else if (name == "ns" || name == "ew") output << "Direction " << v_msg[index] << " direction\n";
                else if (qual_stat_pos_nav_flag && (msg_header_type == "GLL" || msg_header_type == "RMC" || msg_header_type == "VTG")){
                    if (v_msg[index] == "V") output << "Data is invalid\n";
                    else if (v_msg[index] == "A") output << "Data is valid\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "E" ) output << "Estimated/Dead reckoning\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "A" ) output << "Autonomous GNSS fix\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "D" ) output << "Differential GNSS fix\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "F" ) output << "RTK float\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "R" ) output << "RTK fixed\n";
                    else output << "Field is empty\n";
                    
                }
                else if (qual_stat_pos_nav_flag && msg_header_type == "GGA") {
                    if (v_msg[index] == "1") output << "Autonomous GNSS fix\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "2") output << "Differential GNSS fix\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "4") output << "RTK fixed\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "5") output << "RTK float\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "6") output << "Estimated/Dead reckoning fix\n";
                    else output << "Field is empty\n";
                }
                else if (qual_stat_pos_nav_flag && msg_header_type == "GSA") {
                    if (qual_stat_pos_nav_flag && v_msg[index] == "2") output << "2D fix\n";
                    else if (qual_stat_pos_nav_flag && v_msg[index] == "3") output << "3D fix\n";
                    else output << "Field is empty\n";
                }
                else if (qual_stat_pos_nav_flag && (v_msg[index] == "0" || v_msg[index] == "N" || v_msg[index] == "1")) output << "No fix\n";
                else if (name == "num_sv") output << "Number of used satellites : " << v_msg[index] << "\n";
                else if (name == "hdop") output << "Horizontal Dilution of Precision is : " << v_msg[index] << "\n";
                else if (name == "alt") output << "Altitude above mean sea level : " << v_msg[index];
                else if (name == "alt_unit" || name =="sep_unit") output << " Unit of altitude and geoid sepparation is meter.\n";
                else if (name == "sep") output << "Difference between ellipsoid and mean sea level : " << v_msg[index];
                else if (name == "diff_age") output << "Age of differential corrections is " << v_msg[index] << "if 0 then when DGPS is not used\n";
                else if (name == "diff_station") output << "ID of station providing differential corrections is " << v_msg[index]<< "if 0 then when DGPS is not used\n";
                else if (name == "cs") output << "Checksum is " << v_msg[index] << "\n\n";
                else if (op_mode_flag) output << (v_msg[index] == "M") ? "Manually set to operate in 2D or 3D mode \n" : "Automatically switching between 2D or 3D mode \n";
                else if (name == "svid") output << "Number of satelites is " << v_msg[index] << "\n";
                else if (name == "pdop") output << "Position dilution of precision is " << v_msg[index] << "\n";
                else if (name == "vdop") output << "Vertical dilution of precision is " << v_msg[index] << "\n";
                else if (name == "system_id") output << "NMEA-defined GNSS system ID " << v_msg[index] << "\n";
                else if (name == "num_msg") output << "Total number of GSV messages being output is  " << v_msg[index] << "\n";
                else if (name == "num_sv_v") output << "Number of known satellites in view regarding both the talker ID and the signalId is " << v_msg[index] << "\n";
                else if (name == "elv") output << "Elevation is " << v_msg[index] << "\n";
                else if (name == "az") output << "Azimuth " << v_msg[index] << "\n";
                else if (name == "cno") output << "Signal strength " << v_msg[index] << " if null when not tracking\n";
                else if (name == "signal_id") output << "NMEA-defined GNSS signal ID " << v_msg[index] << "\n";
                else if (name == "spd") output << "Speed over ground in knots " << v_msg[index] << "\n";
                else if (name == "cog") output << "Course over ground in degree " << v_msg[index] << "\n";
                else if (name == "date") output << "Date in day, month, year format " << dateConverter(v_msg[index]) << "\n";
                else if (name == "mv") output << "Magnetic variation value is " << v_msg[index] << "\n";
                else if (name == "mv_ew") output << "Magnetic variation E/W indicator is " << v_msg[index] << "\n";
                else if (name == "nav_status") output << "Navigational status indicator: Equipment is not providing navigational status information" << "\n";
                else if (name == "cog_t") output << "Course over ground " << v_msg[index];
                else if (name == "cog_t_unit" || name == "cog_m_unit") output << "Unit of cource over ground is degree \n";
                else if (name == "cog_m") output << "Course over ground (magnetic) " << v_msg[index] << "\n";
                else if (name == "sog_n") output << "Speed over ground in knots " << v_msg[index];
                // else if (name == "sog_n_unit") output << "fixed in knots\n";
                else if (name == "sog_k") output << "Speed over ground in kilometers per hour" << v_msg[index] << "\n";
                // else if (name == "sog_k_unit") output << "fixed in kilometers per hour\n";
                else if (name == "range_rms") output << "RMS value of the standard deviation of the ranges " << v_msg[index] << "\n";//
                else if (name == "std_major") output << "Standard deviation of semi-major axis " << v_msg[index] << "\n";
                else if (name == "std_minor") output << "Standard deviation of semi-minor axis " << v_msg[index] << "\n";
                else if (name == "orient") output << "Orientation of semi-major axis " << v_msg[index] << "\n";
                else if (name == "std_lat") output << "Standard deviation of latitude error " << v_msg[index] << "\n";
                else if (name == "std_long") output << "Standard deviation of longitude error " << v_msg[index] << "\n";
                else if (name == "std_alt") output << "Standard deviation of altitude error " << v_msg[index] << "\n";
                else if (name == "day") output << "UTC day " << v_msg[index] << "\n";
                else if (name == "month") output << "UTC month " << v_msg[index] << "\n";
                else if (name == "year") output << "UTC year " << v_msg[index] << "\n";
                else if (name == "ltzh") output << "Local time zone hours, always 00 " << v_msg[index] << "\n";
                else if (name == "ltzn") output << "Local time zone minutes, always 00 " << v_msg[index] << "\n";
                else if (datum_flag) output << "Local datum code is "  << (v_msg[index] == "W84") ? "WGS84\n" :
                (v_msg[index] == "P90") ? "PZ90\n" : "user defined\n";
                else if (name == "sub_datum") output << "A null field "<< "\n";
                else if (name == "off_lat") output << "Offset in Latitude is " << v_msg[index] << "\n";
                else if (name == "off_lon") output << "Offset in Longitude is " << v_msg[index] << "\n";
                else if (name == "off_alt") output << "Offset in altitude is " << v_msg[index] << "\n";
                else if (name == "ref_datum") output << "Reference datum code " << v_msg[index] << "\n";
                else if (name == "pos_mode_4a"){ 
                    output << "Positioning mode, in the following order for GPS, GLONASS, Galileo and BeiDou " << v_msg[index] << "\n";
                    for (int i = 0; i < v_msg[index].size(); i++){
                        if (v_msg[index] == "N" ) output << "No fix";
                        else if (v_msg[index] == "E" ) output << "Estimated/Dead reckoning";
                        else if (v_msg[index] == "A" ) output << "Autonomous GNSS fix";
                        else if (v_msg[index] == "D" ) output << "Differential GNSS fix";
                        else if (v_msg[index] == "F" ) output << "RTK float";
                        else if (v_msg[index] == "R" ) output << "RTK fixed";
                    }
                }
            }

        }
    } else {
        std::cout << "Message type '" << msg_header_type << "' is not supported.\n";
    }
    output.close();
}

