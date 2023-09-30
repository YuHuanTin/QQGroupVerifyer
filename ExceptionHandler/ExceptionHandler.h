

#ifndef QQGROUPVERIFYER_EXCEPTIONHANDLER_H
#define QQGROUPVERIFYER_EXCEPTIONHANDLER_H


#include <string_view>

class ExceptionHandler {
private:
    bool m_hasException = false;
public:
    void throwExceptionWithOutput(std::string_view ExceptionMessage, std::string_view Where);

    void throwExceptionWithOutput(std::string_view ExceptionMessage, std::string_view Where, int Line);

};


#endif //QQGROUPVERIFYER_EXCEPTIONHANDLER_H
