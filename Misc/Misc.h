

#ifndef QQGROUPVERIFYER_MISC_H
#define QQGROUPVERIFYER_MISC_H


#include <vector>
#include <string_view>
#include <nlohmann/json.hpp>
#include <expected>


namespace Misc {
    uint32_t getBknBySkey(std::string_view Skey);

    std::expected<std::vector<uint64_t>, std::string> jsonUinParser(std::string_view JsonString);

}


#endif //QQGROUPVERIFYER_MISC_H
