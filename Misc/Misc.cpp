

#include "Misc.h"

uint32_t Misc::getBknBySkey(std::string_view Skey) {
    unsigned  uSalt = 5381;
    for (auto &ch: Skey) {
        uSalt = uSalt + (uSalt << 5) + ch;
    }
    return uSalt & 0x7FFFFFFF;
}

std::expected<std::vector<uint64_t>, std::string> Misc::jsonUinParser(std::string_view JsonString) {
    std::vector<uint64_t> uins;
    try {
        nlohmann::json  json = nlohmann::json::parse(JsonString);
        for (const auto &i: json) {
            nlohmann::json arrayElement = nlohmann::json::parse(to_string(i));
            uins.emplace_back(arrayElement["uin"].get<uint64_t>());
        }
    } catch (nlohmann::json::parse_error &exception) {
        return std::unexpected(std::string{"json parse error: "} + exception.what());
    } catch (std::exception &exception) {
        return std::unexpected(exception.what());
    }
    return uins;
}
