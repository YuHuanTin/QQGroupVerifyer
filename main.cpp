


#include <cpr/cpr.h>
#include <vector>
#include <string>
#include <string_view>
#include <print>
#include <expected>

#include "Misc/Misc.h"


cpr::Cookies cookieMerge(const cpr::Cookies &InputCookies, cpr::Cookies &OutputCookies) {
    std::for_each(InputCookies.begin(), InputCookies.end(), [&OutputCookies](const cpr::Cookie &cookie) {
        OutputCookies.emplace_back(cookie);
    });
    return OutputCookies;
}

std::string getValueByCookies(const cpr::Cookies &InputCookies, std::string_view Key) {
    auto it = std::find_if(InputCookies.begin(), InputCookies.end(), [&Key](const cpr::Cookie &cookie) {
        return cookie.GetName() == Key;
    });
    if (it == InputCookies.end())
        return {};
    return it->GetValue();
}

std::expected<cpr::Cookies, std::string> getFirstSessionCookies() {
    cpr::Url        url        = "https://xui.ptlogin2.qq.com/cgi-bin/xlogin";
    cpr::Parameters parameters = {
            {"pt_disable_pwd",  "1"},
            {"appid",           "715030901"},
            {"daid",            "73"},
            {"hide_close_icon", "1"},
            {"pt_no_auth",      "1"},
            {"s_url",           "https://qun.qq.com/member.html?source=panelstar#"}};

    auto response = cpr::Get(url, parameters);
    if (response.error)
        return std::unexpected(response.error.message);
    return response.cookies;
}

std::expected<std::vector<uint64_t>, std::string> getLoginUins(cpr::Cookies &OutCookies) {
    std::string pt_local_token = getValueByCookies(OutCookies, "pt_local_token");

    cpr::Url        url        = "https://localhost.ptlogin2.qq.com:4301/pt_get_uins";
    cpr::Parameters parameters = {
            {"callback",    "ptui_getuins_CB"},
            {"pt_local_tk", pt_local_token}};
    cpr::Header     header     = {{"Referer", "https://xui.ptlogin2.qq.com/"}};

    auto response = cpr::Get(url, parameters, header, OutCookies);
    size_t jsonBegin = response.text.find('[');
    size_t jsonEnd   = response.text.find(']', jsonBegin);
    if (jsonEnd == std::string::npos) {
        return std::unexpected("cannot find json");
    }

    OutCookies = cookieMerge(response.cookies, OutCookies);
    return Misc::jsonUinParser(response.text.substr(jsonBegin, (jsonEnd + 1) - jsonBegin));
}

std::expected<cpr::Cookies, std::string> getClientKey(uint64_t Uin, const cpr::Cookies &InputCookies) {
    std::string pt_local_token = getValueByCookies(InputCookies, "pt_local_token");

    cpr::Url        url        = "https://localhost.ptlogin2.qq.com:4301/pt_get_st";
    cpr::Parameters parameters = {
            {"clientuin",   std::to_string(Uin)},
            {"pt_local_tk", pt_local_token},
            {"callback",    "__jp0"}};
    cpr::Header     header     = {{"Referer", "https://xui.ptlogin2.qq.com/"}};

    auto response = cpr::Get(url, parameters, header, InputCookies);
    if (response.error)
        return std::unexpected(response.error.message);
    return cookieMerge(InputCookies, response.cookies);
}

std::expected<cpr::Cookies, std::string> getLoginCookies(const cpr::Cookies &InputCookies) {
    std::string pt_local_token = getValueByCookies(InputCookies, "pt_local_token");
    std::string clientuin      = getValueByCookies(InputCookies, "clientuin");

    cpr::Url        url        = "https://ssl.ptlogin2.qq.com/jump";
    cpr::Parameters parameters = {
            {"clientuin",   clientuin},
            {"keyindex",    "19"},
            {"pt_aid",      "715030901"},
            {"daid",        "73"},
            {"u1",          "https://qun.qq.com/member.html?source=panelstar#"},
            {"pt_local_tk", pt_local_token},
            {"pt_3rd_aid",  "0"},
            {"ptopt",       "1"},
            {"style",       "40"}};
    cpr::Header     header     = {{"Referer", "https://xui.ptlogin2.qq.com/"}};

    auto response = cpr::Get(url, parameters, header, InputCookies);
    if (response.error)
        return std::unexpected(response.error.message);

    response.cookies = cookieMerge(InputCookies, response.cookies);


    // login
    size_t begin = response.text.find("https");
    size_t end   = response.text.find('\'', begin);
    if (end == std::string::npos)
        return std::unexpected("cannot find login url");

    url      = response.text.substr(begin, end - begin);
    response         = cpr::Get(url, header, response.cookies);
    if (response.error)
        return std::unexpected(response.error.message);
    return cookieMerge(InputCookies, response.cookies);
}

std::expected<std::string, std::string> getGroupList(const cpr::Cookies &InputCookies) {
    std::string bkn = std::to_string(Misc::getBknBySkey(getValueByCookies(InputCookies, "skey")));

    cpr::Url        url        = "https://qun.qq.com/cgi-bin/qun_mgr/get_group_list";
    cpr::Parameters parameters = {{"bkn", bkn}};
    cpr::Header     header     = {{"Referer", "https://xui.ptlogin2.qq.com/"}};
    auto            response   = cpr::Get(url, parameters, header, InputCookies);

    if (response.error)
        return std::unexpected(response.error.message);
    return response.text;
}

int main() {
    // disable output buffering
    setvbuf(stdout, nullptr, _IONBF, 0);

    // setting global encoding utf-8
    std::locale::global(std::locale("zh_CN.UTF-8"));


    auto expectedCookies = getFirstSessionCookies();
    if (!expectedCookies) {
        std::println("can not get cookies: {}", expectedCookies.error());
        return -1;
    }

    auto uins = getLoginUins(expectedCookies.value());
    if (!uins) {
        std::println("can not get uins: {}", uins.error());
        return -1;
    }

    std::for_each(uins.value().begin(), uins.value().end(), [&expectedCookies](uint64_t uin) {
        auto u = getClientKey(uin, expectedCookies.value())
                .and_then(getLoginCookies)
                .and_then(getGroupList);

        if (!u)
            std::println("can not get group list: {}", u.error());
        else
            std::println("{}", u.value());
    });


    return 0;
}
