


#include <boost/locale.hpp>
#include <fmt/format.h>
#include <fmt/std.h>
#include <cpr/cpr.h>
#include <vector>
#include <string>
#include <string_view>
#include <format>

#include "ExceptionHandler/ExceptionHandler.h"
#include "Misc/Misc.h"


std::shared_ptr<ExceptionHandler> g_exceptionHandler(new ExceptionHandler);

cpr::Cookies &cookieMerge(cpr::Cookies &InputCookies, cpr::Cookies &PatchCookie) {
    for (const auto &one: PatchCookie) {
        InputCookies.emplace_back(one);
    }
    return InputCookies;
}

std::string getValueByCookies(cpr::Cookies &InputCookies, std::string_view Key) {
    std::string     value;
    for (const auto &cookie: InputCookies) {
        if (cookie.GetName() == Key) {
            value = cookie.GetValue();
            break;
        }
    }
    return value;
}

std::string u16Tou8(const std::u16string &U16Str) {
    return boost::locale::conv::utf_to_utf<char, char16_t>(U16Str);
}

std::string usc2ToUtf8(const std::string &UscStr) noexcept {
    std::string      u8Str;
    std::u16string   u16Str;
    for (std::size_t i = 0; i < UscStr.size(); ++i) {
        while (i + 5 < UscStr.size() && UscStr[i] == '\\' && UscStr[i + 1] == 'u') {
            auto hexValue = (char16_t) std::stol(std::string{UscStr.data() + i + 2, UscStr.data() + i + 6}, nullptr, 16);
            u16Str.push_back(hexValue);
            i += 6;
        }
        if (!u16Str.empty()) {
            u8Str.append(u16Tou8(u16Str));
            u16Str.clear();
        }
        if (i < UscStr.size())
            u8Str.push_back(UscStr[i]);
    }
    return u8Str;
}

void getFirstSessionCookies(cpr::Cookies &OutCookies) {
    cpr::Url url = {"https://xui.ptlogin2.qq.com/cgi-bin/xlogin"
                    "?pt_disable_pwd=1"
                    "&appid=715030901"
                    "&daid=73"
                    "&hide_close_icon=1"
                    "&pt_no_auth=1"
                    "&s_url=https%3A%2F%2Fqun.qq.com%2Fmember.html%3Fsource%3Dpanelstar%23"};

    auto response = cpr::Get(url);

    cookieMerge(OutCookies, response.cookies);
}

std::vector<uint64_t> getLoginUins(cpr::Cookies &OutCookies) {
    std::string pt_local_token = getValueByCookies(OutCookies, "pt_local_token");

    cpr::Url url = std::format("https://localhost.ptlogin2.qq.com:4301/pt_get_uins"
                               "?callback=ptui_getuins_CB"
                               "&pt_local_tk={}", pt_local_token);

    auto response = cpr::Get(url, cpr::Header{{"Referer", "https://xui.ptlogin2.qq.com/"}}, OutCookies);

    size_t jsonBegin = response.text.find('[');
    size_t jsonEnd   = response.text.find(']', jsonBegin);
    if (jsonEnd == std::string::npos) {
        g_exceptionHandler->throwExceptionWithOutput("can not find uin json!", __FUNCTION__, __LINE__);
    }

    cookieMerge(OutCookies, response.cookies);
    return Misc::jsonUinParser(response.text.substr(jsonBegin, (jsonEnd + 1) - jsonBegin), g_exceptionHandler);
}

void getClientKey(cpr::Cookies &OutCookies, uint64_t Uin) {
    std::string pt_local_token = getValueByCookies(OutCookies, "pt_local_token");

    cpr::Url url = std::format("https://localhost.ptlogin2.qq.com:4301/pt_get_st"
                               "?clientuin={}"
                               "&pt_local_tk={}"
                               "&callback=__jp0", std::to_string(Uin), pt_local_token);

    auto response = cpr::Get(url, cpr::Header{{"Referer", "https://xui.ptlogin2.qq.com/"}}, OutCookies);

    cookieMerge(OutCookies, response.cookies);
}

void getLoginCookies(cpr::Cookies &OutCookies) {
    std::string pt_local_token = getValueByCookies(OutCookies, "pt_local_token");
    std::string clientuin      = getValueByCookies(OutCookies, "clientuin");

    cpr::Url url = std::format("https://ssl.ptlogin2.qq.com/jump"
                               "?clientuin={}"
                               "&keyindex=19"
                               "&pt_aid=715030901"
                               "&daid=73"
                               "&u1=https%3A%2F%2Fqun.qq.com%2Fmember.html%3Fsource%3Dpanelstar%23"
                               "&pt_local_tk={}"
                               "&pt_3rd_aid=0"
                               "&ptopt=1"
                               "&style=40", clientuin, pt_local_token);


    auto response = cpr::Get(url, cpr::Header{{"Referer", "https://xui.ptlogin2.qq.com/"}}, OutCookies);
    cookieMerge(OutCookies, response.cookies);


    // login
    size_t begin = response.text.find("https");
    size_t end   = response.text.find('\'', begin);
    if (end == std::string::npos) {
        g_exceptionHandler->throwExceptionWithOutput("can not find login url", __FUNCTION__, __LINE__);
    }
    url      = response.text.substr(begin, end - begin);
    response = cpr::Get(url, cpr::Header{{"Referer", "https://xui.ptlogin2.qq.com/"}}, OutCookies);
    cookieMerge(OutCookies, response.cookies);
}

void getGroupList(cpr::Cookies &OutCookies) {
    std::string bkn = std::to_string(Misc::getBknBySkey(getValueByCookies(OutCookies, "skey")));

    cpr::Url url = std::format("https://qun.qq.com/cgi-bin/qun_mgr/get_group_list"
                               "?bkn={}", bkn);

    auto response = cpr::Get(url, cpr::Header{{"Referer", "https://xui.ptlogin2.qq.com/"}}, OutCookies);

    fmt::println("{}", boost::locale::conv::from_utf(usc2ToUtf8(response.text), "GBK"));
}

int main() {
    setbuf(stdout, nullptr);


    cpr::Cookies cookies;

    getFirstSessionCookies(cookies);
    auto uins = getLoginUins(cookies);

    for (uint64_t &uin: uins) {
        getClientKey(cookies, uin);
        getLoginCookies(cookies);
        getGroupList(cookies);
    }
    return 0;
}
