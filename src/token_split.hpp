#include <fstream>
#include <sstream>
#include <string>

//if first non-sign, non-space character of a cell is numeric, std::stof() will function dependably
//note: sort character passes in while() loop by expected frequency (e.g. if more tokens are expected
//      to start with a '-' than a '+' or a ' ', place it first to make use of short-circuiting)

bool validNumeric(const std::string token) {

    unsigned int i = 0;

    //returns false if input string is comprised entirely 
    //of leading chars {' ','+','-'} which should default
    //to zero (false->zero) where function returns

    while (i < token.size() && (token[i] == ' ' || token[i] == '-' || token[i] == '+')) i++;

    if (i < token.size()) return isdigit(token[i]);

    return false;

}


//splitting algo optimized for different data types / groups of data
//types (e.g. uint32_t, int64_t, and int will all use std::atoi())

auto splitStr(const std::string buff, const unsigned int cols, const char delimeter) {

    std::stringstream str(buff);

    std::string token;

    auto tokens = new std::string[cols]();

    for (unsigned int i = 0; i < cols && std::getline(str, token, delimeter); i++) {
        
        tokens[i] = token;

    }

    return tokens;

}

auto splitFlp(const std::string buff, const unsigned int cols, const char delimeter) {

    std::stringstream str(buff);

    std::string token;

    auto tokens = new double[cols];

    for (unsigned int i = 0; i < cols && std::getline(str, token, delimeter); i++) {
        //stof() freaks out if passed a string of length 0 or one starting with a non-numeric char
        tokens[i] = validNumeric(token) ? std::stof(token) : 0.0;
    }

    return tokens;

}

template<typename dataType>
dataType * splitInt(const std::string buff, const unsigned int cols, const char delimeter) {

    std::stringstream str(buff);

    std::string token;

    auto tokens = new dataType[cols];

    for (unsigned int i = 0; i < cols && std::getline(str, token, delimeter); i++) {
        //stof() freaks out if passed a string of length 0 or one starting with a non-numeric char
        tokens[i] = validNumeric(token) ? (dataType)std::stoi(token) : 0.0;
    }

    return tokens;
}
