


#include <fmt/format.h>
#include <fmt/std.h>
#include <WinhttpAPI.h>
#include <vector>
#include <string>
#include <string_view>
#include <ranges>
#include <format>
#include <boost/locale.hpp>

#include "ExceptionHandler/ExceptionHandler.h"
#include "Misc/Misc.h"


/*class Cookie {
private:
    std::map<std::string, std::string>           m_key_value;
    std::map<std::string, decltype(m_key_value)> m_host_cookies;

    // 解析一行 cookie 文件, 以 ; 分割 kv 串, 以 = 分割 kv
    static std::vector<std::pair<std::string, std::string>> parseCookie(std::string_view LineCookie) {
        std::vector<std::pair<std::string, std::string>> result;

        auto                          cookieTokens = LineCookie | std::views::split(';');
        std::vector<std::string_view> vecCookitTokens(cookieTokens.begin(), cookieTokens.end());

        for (auto one: vecCookitTokens) {
            auto                          tokenTokens = one | std::views::split('=');
            std::vector<std::string_view> vecTokenTokens(tokenTokens.begin(), tokenTokens.end());
            if (vecTokenTokens.size() == 2) {
                result.emplace_back(vecTokenTokens[0], vecTokenTokens[1]);
            }
        }
        return result;
    }

public:

    void parseCookiesFromHeaders(const std::string &Headers) {
        const std::string_view sv("Set-Cookie: ");

        auto lineHeaders = Headers | std::views::split(std::string_view{"\r\n"});

        std::vector<std::string_view> vecLineHeaders(lineHeaders.begin(), lineHeaders.end());
        for (auto                     one: vecLineHeaders) {
            if (one.starts_with(sv)) {
                auto result = parseCookie(one.substr(sv.size()));

                std::string     host = "*";
                for (const auto &one2: result) {
                    if (one2.first == "Path") {
                        host.append(one2.second);
                    } else if (one2.first == "Domain") {
                        host.insert(host.begin(), one2.second.begin(), one2.second.end());
                    }
                }
                add(result[0].first, result[1].second, host);
                m_host_cookies[host][result[0].first] = result[0].second;
            }
        }
    }

    void add(const std::string &CookieKey, const std::string &CookieValue, const std::string &Host = "*") {
        m_host_cookies[Host][CookieKey] = CookieValue;
    }

    Cookie &clear() {
        m_host_cookies.clear();
        m_key_value.clear();
        return *this;
    }

    std::string get(const std::string &CookieKey, const std::string &Host = "*") {
        return m_host_cookies.at(Host).at(CookieKey);
    }

    std::string toString(const std::string &Host = "*") {
        std::string result;
        const auto  &map = m_host_cookies[Host];
        for (auto   it   = map.begin(); it != map.end(); ++it) {
            if (std::next(it) != map.end()) {
                result.append(it->first).append("=").append(it->second).append("; ");
            } else {
                result.append(it->first).append("=").append(it->second);
                break;
            }
        }
        return result;
    }
};*/

class Cookie {
private:
    std::map<std::string, std::string> m_key_value;
    std::shared_ptr<ExceptionHandler>  m_exceptionHandler;
public:
    explicit Cookie(std::shared_ptr<ExceptionHandler> &ExceptionHandler) : m_exceptionHandler(ExceptionHandler) {}

    void parseCookiesFromHeaders(const std::string &Headers) {
        const std::string_view sv("Set-Cookie: ");

        for (auto one: Headers | std::views::split(std::string_view{"\r\n"})) {
            std::string str{one.begin(), one.end()};
            if (str.starts_with(sv)) {
                size_t begin = str.find(' ');
                size_t mid   = str.find('=', begin);
                size_t end   = str.find(';', mid);
                if (end == std::string::npos) {
                    m_exceptionHandler->throwExceptionWithOutput("illegal cookie", __FUNCTION__, __LINE__);
                }
                add(str.substr(begin + 1, mid - (begin + 1)), str.substr(mid + 1, end - (mid + 1)));
            }
        }
    }

    void add(const std::string &CookieKey, const std::string &CookieValue) {
        m_key_value[CookieKey] = CookieValue;
    }

    Cookie &clear() {
        m_key_value.clear();
        return *this;
    }

    std::string get(const std::string &CookieKey) {
        try {
            return m_key_value.at(CookieKey);
        } catch (std::exception &exception) {
            m_exceptionHandler->throwExceptionWithOutput("can not find element", __FUNCTION__, __LINE__);
        }
        return {};
    }

    std::string toString() {
        std::string result;

        for (auto it = m_key_value.begin(); it != m_key_value.end(); ++it) {
            if (std::next(it) != m_key_value.end()) {
                result.append(std::format("{}={}; ", it->first, it->second));
            } else {
                result.append(std::format("{}={}", it->first, it->second));
                break;
            }
        }
        return result;
    }
};

std::shared_ptr<ExceptionHandler> g_exceptionHandler(new ExceptionHandler);
std::shared_ptr<Cookie>           g_qqCookies(new Cookie(g_exceptionHandler));

void getFirstSessionCookies(std::shared_ptr<Cookie> &QQCtx) {
    HttpRequestT  requestT;
    HttpResponseT responseT;

    requestT.url      = "https://xui.ptlogin2.qq.com/cgi-bin/xlogin"
                        "?pt_disable_pwd=1"
                        "&appid=715030901"
                        "&daid=73"
                        "&hide_close_icon=1"
                        "&pt_no_auth=1"
                        "&s_url=https%3A%2F%2Fqun.qq.com%2Fmember.html%3Fsource%3Dpanelstar%23";
    requestT.protocol = "GET";

    WinhttpAPI api(requestT, responseT);
    api.Request();
    QQCtx->parseCookiesFromHeaders(responseT.Headers);
}

std::vector<uint64_t> getLoginUins(std::shared_ptr<Cookie> &QQCtx) {
    HttpRequestT  requestT;
    HttpResponseT responseT;

    requestT.url      = std::format("https://localhost.ptlogin2.qq.com:4301/pt_get_uins"
                                    "?callback=ptui_getuins_CB"
                                    "&pt_local_tk={}", QQCtx->get("pt_local_token"));
    requestT.protocol = "GET";

    WinhttpAPI api(requestT, responseT);
    api.SetHeaders({{"Cookie",  QQCtx->toString()},
                    {"Referer", "https://xui.ptlogin2.qq.com/"}});

    api.Request();
    size_t jsonBegin = responseT.Body.find('[');
    size_t jsonEnd   = responseT.Body.find(']', jsonBegin);
    if (jsonEnd == std::string::npos) {
        g_exceptionHandler->throwExceptionWithOutput("can not find uin json!", __FUNCTION__, __LINE__);
    }

    return Misc::jsonUinParser(responseT.Body.substr(jsonBegin, (jsonEnd + 1) - jsonBegin), g_exceptionHandler);
}

void getClientKey(std::shared_ptr<Cookie> &QQCtx, uint64_t Uin) {
    HttpRequestT  requestT;
    HttpResponseT responseT;

    requestT.url      = std::format("https://localhost.ptlogin2.qq.com:4301/pt_get_st"
                                    "?clientuin={}"
                                    "&pt_local_tk={}"
                                    "&callback=__jp0", std::to_string(Uin), QQCtx->get("pt_local_token"));
    requestT.protocol = "GET";

    WinhttpAPI api(requestT, responseT);
    api.SetHeaders({{"Cookie",  QQCtx->toString()},
                    {"Referer", "https://xui.ptlogin2.qq.com/"}});

    api.Request();
    QQCtx->parseCookiesFromHeaders(responseT.Headers);
}

void getLoginUrl(std::shared_ptr<Cookie> &QQCtx) {
    HttpRequestT  requestT;
    HttpResponseT responseT;

    requestT.url      = std::format("https://ssl.ptlogin2.qq.com/jump"
                                    "?clientuin={}"
                                    "&keyindex=19"
                                    "&pt_aid=715030901"
                                    "&daid=73"
                                    "&u1=https%3A%2F%2Fqun.qq.com%2Fmember.html%3Fsource%3Dpanelstar%23"
                                    "&pt_local_tk={}"
                                    "&pt_3rd_aid=0"
                                    "&ptopt=1"
                                    "&style=40", QQCtx->get("clientuin"), QQCtx->get("pt_local_token"));
    requestT.protocol = "GET";

    WinhttpAPI api(requestT, responseT);
    api.SetHeaders({{"Cookie",  QQCtx->toString()},
                    {"Referer", "https://xui.ptlogin2.qq.com/"}});
    api.Request();
    QQCtx->parseCookiesFromHeaders(responseT.Headers);
}

void getGroupList(std::shared_ptr<Cookie> &QQCtx) {
    HttpRequestT  requestT;
    HttpResponseT responseT;

//    requestT.url      = "https://qun.qq.com/cgi-bin/qun_mgr/get_group_list" // 需要 p_skey uin skey p_uin RK
    requestT.url      = std::format("https://qqweb.qq.com/cgi-bin/anony/get_group_lst"
                                    "?bkn={}", std::to_string(Misc::getBknBySkey(QQCtx->get("skey"))));
    requestT.protocol = "GET";
    WinhttpAPI api(requestT, responseT);
    // p_skey 控制是否登录
    api.SetHeaders({{"Cookie",  QQCtx->toString()},
                    {"Referer", "https://xui.ptlogin2.qq.com/"}});
    api.Request();

    fmt::println("{}", QQCtx->toString());
    fmt::println("{}", boost::locale::conv::from_utf(responseT.Body, "GBK"));
}

int main() {
    setbuf(stdout, nullptr);


    getFirstSessionCookies(g_qqCookies);
    auto uins = getLoginUins(g_qqCookies);

    for (uint64_t &uin: uins) {
        getClientKey(g_qqCookies, uin);
        getLoginUrl(g_qqCookies);
        getGroupList(g_qqCookies);
    }
    return 0;
}
