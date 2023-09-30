

#include "Misc.h"

uint32_t Misc::getBknBySkey(std::string_view Skey) {
    unsigned  uSalt = 5381;
    for (auto &ch: Skey) {
        uSalt = uSalt + (uSalt << 5) + ch;
    }
    return uSalt & 0x7FFFFFFF;
}

std::vector<uint64_t> Misc::jsonUinParser(std::string_view JsonString, std::shared_ptr<ExceptionHandler> &ExceptionHandler) {
    std::vector<uint64_t> uins;
    try {
        nlohmann::json  json = nlohmann::json::parse(JsonString);
        for (const auto &i: json) {
            nlohmann::json arrayElement = nlohmann::json::parse(to_string(i));
            uins.emplace_back(arrayElement["uin"].get<uint64_t>());
        }
    } catch (nlohmann::json::parse_error &exception) {
        ExceptionHandler->throwExceptionWithOutput(exception.what(), __FUNCTION__, __LINE__);
    } catch (std::exception &exception) {
        ExceptionHandler->throwExceptionWithOutput(exception.what(), __FUNCTION__, __LINE__);
    }
    return uins;
}
