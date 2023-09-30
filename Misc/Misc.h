

#ifndef QQGROUPVERIFYER_MISC_H
#define QQGROUPVERIFYER_MISC_H


#include <vector>
#include <string_view>
#include <nlohmann/json.hpp>
#include "../ExceptionHandler/ExceptionHandler.h"

namespace Misc {
    uint32_t getBknBySkey(std::string_view Skey);

    std::vector<uint64_t> jsonUinParser(std::string_view JsonString, std::shared_ptr<ExceptionHandler> &ExceptionHandler);

}


#endif //QQGROUPVERIFYER_MISC_H
