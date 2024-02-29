#pragma once

#include "../common.h"

namespace bpath
{
    class AuthorityInfo
    {
    public:
        const std::optional<brex::ASCIIString> userinfo;
        const brex::ASCIIString host;

        AuthorityInfo(std::optional<brex::ASCIIString> userinfo, brex::ASCIIString host) : userinfo(userinfo), host(host) {;}

        std::u8string toBSQONFormat() const
        {
            std::u8string res = u8"//";
            if(this->userinfo.has_value()) {
                res += std::u8string(this->userinfo.value().cbegin(), this->userinfo.value().cend());
                res += u8"@";
            }
            res += std::u8string(this->host.cbegin(), this->host.cend());
            return res;
        }
    };

    class ElementInfo
    {
    public:
        const brex::ASCIIString ename;
        const std::optional<brex::ASCIIString> ext;

        ElementInfo(brex::ASCIIString ename, std::optional<brex::ASCIIString> ext) : ename(ename), ext(ext) {;}

        std::u8string toBSQONFormat() const
        {
            std::u8string res = std::u8string(this->ename.cbegin(), this->ename.cend());
            if(this->ext.has_value()) {
                res += u8".";
                res += std::u8string(this->ext.value().cbegin(), this->ext.value().cend());
            }
            return res;
        }
    };

    class Path
    {
    public:
        const brex::ASCIIString scheme;
        const std::optional<AuthorityInfo> authorityinfo;
        const std::vector<brex::ASCIIString> segments;

        Path(brex::ASCIIString scheme, std::optional<AuthorityInfo> authorityinfo, std::vector<brex::ASCIIString> segments) : scheme(scheme), authorityinfo(authorityinfo), segments(segments) {;}
        virtual ~Path() {;}

        virtual bool isElementPath() const { return false; }
        virtual bool isGroupPath() const { return false; }

        virtual std::u8string toBSQONFormat() const = 0;
    };

    class PathElement : public Path
    {
    public:
        const ElementInfo elementinfo;

        PathElement(brex::ASCIIString scheme, std::optional<AuthorityInfo> authorityinfo, std::vector<brex::ASCIIString> segments, ElementInfo elementinfo) : Path(scheme, authorityinfo, segments), elementinfo(elementinfo) {;}
        virtual ~PathElement() {;}

        bool isElementPath() const override final { return true; }

        std::u8string toBSQONFormat() const override final
        {
            std::u8string res = std::u8string(this->scheme.cbegin(), this->scheme.cend());
            res += u8":";
            if(this->authorityinfo.has_value()) {
                res += this->authorityinfo.value().toBSQONFormat();
            }
            res += u8"/";
            for(size_t i = 0; i < this->segments.size(); ++i) {
                res += std::u8string(this->segments[i].cbegin(), this->segments[i].cend()) + u8'/';
            }
            
            return res + this->elementinfo.toBSQONFormat();
        }
    };

    class PathGroup : public Path
    {
    public:
        PathGroup(brex::ASCIIString scheme, std::optional<AuthorityInfo> authorityinfo, std::vector<brex::ASCIIString> segments) : Path(scheme, authorityinfo, segments) {;}
        virtual ~PathGroup() {;}

        bool isGroupPath() const override final { return true; }

        std::u8string toBSQONFormat() const override final
        {
            std::u8string res = std::u8string(this->scheme.cbegin(), this->scheme.cend());
            res += u8":";
            if(this->authorityinfo.has_value()) {
                res += this->authorityinfo.value().toBSQONFormat();
            }
            res += u8"/";
            for(size_t i = 0; i < this->segments.size(); ++i) {
                if(i > 0) {
                    res += u8"/";
                }
                res += std::u8string(this->segments[i].cbegin(), this->segments[i].cend());
            }
            return res;
        }
    };
}