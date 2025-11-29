#include <fstream>
#include <vector>
#include <iostream>
#include <regex>
#include <map>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <filesystem>

class GnssParser {

    public:
        // helper functions
        /**
        * \brief Converting string to utc time format
        * \param [in] utc_str String line to convert
        * \return Converted string
        */
        std::string convertFormatUtc(const std::string& utc_str);

        /**
        * \brief Converting string to latitude or longitude
        * \param [in] latLong String line to convert
        * \return Converted string 
        */
        std::string convertLatLot(const std::string& latLong);

        /**
        * \brief Converting string to date
        * \param [in] date String line to convert
        * \return Converted string 
        */
        std::string convertDate(const std::string& date);

        /**
        * \brief Size correction for GSV message 
        * \param [in] header Message type string 
        * \param [in] header Vector to correct size upon
        * \return Converted string 
        */
        void correctGsvMsg(const std::string& header, std::vector<std::string>& vMsg);
        

        // main functions
        /**
        * \brief Checking if message header is valid  
        * \param [in] msg Message from which check header
        * \return String with header if it is valid, empty string otherwise 
        */
        std::string parseHeader(std::string& msg);

        /**
        * \brief Checking if message itself is valid and executes message spliting
        * \param [in] header Header to check if empty
        * \param [in] msg Message to check with regex
        */
        void processMsg(std::string header, std::string& msg);

        /**
        * \brief Splits message on individual data fields via comma (,) and star symbol (*)
        * \param [in] msg Message to split
        */
        void splitNmeaMessage(const std::string& msg);

        /**
        * \brief Outputs formatted data in file
        * \param [in] msg Original message. Used to make output a little more readable
        * \param [in] vMsg Splited data to formate and output in file
        */
        void formatOutput(const std::string& msg, std::vector<std::string>& vMsg);

    private:
        using ProtocolMap = std::map<int, std::string>;
        const std::unordered_map<std::string, ProtocolMap> MSG_TYPE_MAP_ = {
            {"GGA", {
                {0, "header"},
                {1, "time"},
                {2, "lat"},
                {3, "ns"},
                {4, "lon"},
                {5, "ew"},
                {6, "quality"},
                {7, "num_sv"},
                {8, "hdop"},
                {9, "alt"},
                {10, "alt_unit"},
                {11, "sep"},
                {12, "sep_unit"},
                {13, "diff_age"},
                {14, "diff_station"},
                {15, "cs"},
                {16, "cr_lf"}
            }},
            {"GLL", {
                {0, "header"},
                {1, "lat"},
                {2, "ns"},
                {3, "lon"},
                {4, "ew"},
                {5, "time"},
                {6, "status"},
                {7, "pos_mode"},
                {8, "cs"},
                {9, "cr_lf"}
            }},
            {"GSA", {
                {0, "header"},
                {1, "op_mode"},
                {2, "nav_mode"},
                {3, "svid"},
                {4, "svid"},
                {5, "svid"},
                {6, "svid"},
                {7, "svid"},
                {8, "svid"},
                {9, "svid"},
                {10, "svid"},
                {11, "svid"},
                {12, "svid"},
                {13, "svid"},
                {14, "svid"},
                {15, "pdop"},
                {16, "hdop"},
                {17, "vdop"},
                {18, "system_id"},
                {19, "cs"},
                {20, "cr_lf"}
            }},
            {"GSV", {
                {0, "header"},
                {1, "num_msg"},
                {2, "msg_num"},
                {3, "num_sv_v"},
                {4, "svid"},
                {5, "elv"},
                {6, "az"},
                {7, "cno"},
                {8, "svid"},
                {9, "elv"},
                {10, "az"},
                {11, "cno"},
                {12, "svid"},
                {13, "elv"},
                {14, "az"},
                {15, "cno"},
                {16, "svid"},
                {17, "elv"},
                {18, "az"},
                {19, "cno"},
                {20, "signal_id"},
                {21, "cs"},
                {22, "cr_lf"}
            }},
            {"RMC", {
                {0, "header"},
                {1, "time"},
                {2, "status"},
                {3, "lat"},
                {4, "ns"},
                {5, "lon"},
                {6, "ew"},
                {7, "spd"},
                {8, "cog"},
                {9, "date"},
                {10, "mv"},
                {11, "mv_ew"},
                {12, "pos_mode"},
                {13, "nav_status"},
                {14, "cs"},
                {15, "cr_lf"}
            }},
            {"VTG", {
                {0, "header"},
                {1, "cog_t"},
                {2, "cog_t_unit"},
                {3, "cog_m"},
                {4, "cog_m_unit"},
                {5, "sog_n"},
                {6, "sog_n_unit"},
                {7, "sog_k"},
                {8, "sog_k_unit"},
                {9, "pos_mode"},
                {10, "cs"},
                {11, "cr_lf"}
            }},
            {"GST", {
                {0, "header"},
                {1, "time"},
                {2, "range_rms"},
                {3, "std_major"},
                {4, "std_minor"},
                {5, "orient"},
                {6, "std_lat"},
                {7, "std_long"},
                {8, "std_alt"},
                {9, "cs"},
                {10, "cr_lf"}
            }},
            {"ZDA", {
                {0, "header"},
                {1, "time"},
                {2, "day"},
                {3, "month"},
                {4, "year"},
                {5, "ltzh"},
                {6, "ltzn"},
                {7, "cs"},
                {8, "cr_lf"},

            }},
            {"DTM", {
                {0, "header"},
                {1, "datum"},
                {2, "sub_datum"},
                {3, "off_lat"},
                {4, "ns"},
                {5, "off_lon"},
                {6, "ew"},
                {7, "alt"},
                {8, "ref_datum"},
                {9, "cs"},
                {10, "cr_lf"}

            }},
            {"GNS", {
                {0, "header"},
                {1, "time"},
                {2, "lat"},
                {3, "ns"},
                {4, "lon"},
                {5, "ew"},
                {6, "pos_mode_4a"},
                {7, "num_sv"},
                {8, "hdop"},
                {9, "alt"},
                {10, "sep"},
                {11, "diff_age"},
                {12, "diff_station"},
                {13, "nav_status"},
                {14, "cs"},
                {15, "cr_lf"}
            }},
        };
        const int STABLE_DATA_FRROM_END_GSV_ = 3;
        const int NO_CR_CL_IDX_ = 1;
        std::regex headerRegex_{"\\$(?:(?:GP|GL|GA|GB|GN)(?:GGA|GLL|GSA|RMC|VTG|GST|ZDA|DTM|GNS)|(?:GP|GL|GA|GB)GSV)"};
        std::regex gaaRegex_{"\\$(GP|GL|GA|GB|GN)(GGA),(\\d{6}.\\d{2})?,(\\d{4}.\\d{5})?,[NS]?,(\\d{5}.\\d{5})?,[EW]?,[012456]?,(\\d{2})?,(\\d{1,2}.\\d{1,5})?,(\\d{1,5}.\\d{1,5})?,M?,(\\d{1,5}.\\d{1,5})?,M?,(\\d{1,5}.\\d{1,5})?,\\*[\\da-fA-F]{2}\\n?\\r?"};
        std::regex gllRegex_{"\\$(GP|GL|GA|GB|GN)(GLL),(\\d{4}.\\d{5})?,[NS]?,(\\d{5}.\\d{5})?,[EW]?,(\\d{6}.\\d{2})?,[AV]?,[AV]?\\*[\\da-fA-F]{2}\r?\n?"};
        std::regex gsaRegex_{"\\$(GP|GL|GA|GB|GN)(GSA),[AM]?,[123]?,(\\d{0,3},){0,12}(\\d{1,2}.\\d{1,3},){0,3}[\\dA-F]{1,2}\\*[\\dA-F]{2}\r?\n?"};
        std::regex gsvRegex_{"\\$(GP|GL|GA|GB|GN)(GSV),(\\d{1})?,(\\d{1})?,(\\d{1,3})?(,\\d{0,3}?,\\d{0,2}?,\\d{0,3}?,\\d{0,2}?){0,4},[\\dA-F]{1,2}\\*[\\dA-F]{2}\r?\n?"};
        std::regex rmcRegex_{"\\$(GP|GL|GA|GB|GN)(RMC),\\d{6}?.\\d{2}?,[NEAFDR]?,\\d{4}.\\d{5,6}?,[NS]?,\\d{5}.\\d{5,6}?,[EW]?,\\d{1,3}.\\d{1,3}?,\\d{0,3}.\\d{1,3}?,\\d{6}?,\\d{0,2},\\d{0,2},[NEAFDR]?,V?\\*[\\dA-F]{2}\r?\n?"};
        std::regex vtgRegex_{"\\$(GP|GL|GA|GB|GN)(VTG),(\\d{1,3}.\\d{1,2})?,[T],(\\d{1,3}.\\d{1,2})?,[M],(\\d{1,3}.\\d{1,3})?,[N],(\\d{1,3}.\\d{1,3})?,[K],[ADEN]?\\*[0-9A-F]{2}\r?\n?"};
        std::regex gstRegex_{"\\$(GP|GL|GA|GB|GN)(GST),(\\d{6}.\\d{2})?,(\\d{1,5}.\\d{1,5})?,(\\d{1,5}.\\d{1,5})?,(\\d{1,5}.\\d{1,5})?,(\\d{1,5}.\\d{1,5})?,(\\d{1,2}.\\d{1,3})?,(\\d{1,2}.\\d{1,3})?,(\\d{1,2}.\\d{1,3})?\\*[0-9A-F]{2}\r?\n?"};
        std::regex zdaRegex_{"\\$(GP|GL|GA|GB|GN)(ZDA),(\\d{6}.\\d{2})?,(\\d{2})?,(\\d{2})?,(\\d{4})?,00,00\\*[0-9A-F]{2}\r?\n?"};
        std::regex dtmRegex_{"\\$(GP|GL|GA|GB|GN)(DTM),(W84|P90|999)?,,(\\d{1,3}.\\d{1,2})?,[NS]?,(\\d{1,3}.\\d{1,2})?,[EW]?,(-?\\d{1,3}.\\d{1,2})?,W84\\*[\\dA-F]{2}\r?\n?"};
        std::regex gnsRegex_{"\\$(GP|GL|GA|GB|GN)(GNS),(\\d{6}.\\d{1,2})?,(\\d{4}.\\d{5,6})?,[NS]?,(\\d{5}.\\d{5,6})?,[WE]?,([NEFRAD]{4})?,[\\d]{1,2}?,(\\d{1,2}.\\d{1,2})?,(\\d{1,4}.\\d{1,5})?,(\\d{1,4}.\\d{1,5})?,(\\d{1,2}.\\d{1,2})?,(\\d{1,3})?,V\\*[\\dA-F]{2}\r?\n?"};    
    };
