#include "brex.h"
// json.hpp is included via brex.h -> common.h

namespace brex
{
    // RegexToplevelEntry
    void RegexToplevelEntry::emitJSON(json& j) const
    {
        j["isNegated"] = this->isNegated;
        j["isFrontCheck"] = this->isFrontCheck;
        j["isBackCheck"] = this->isBackCheck;
        json opt_val;
        this->opt->emitJSON(opt_val);
        j["opt"] = opt_val;
    }

    // RegexComponent and its derived classes
    void RegexSingleComponent::emitJSON(json& j) const
    {
        // A RegexSingleComponent is represented as the JSON object of its entry.
        this->entry.emitJSON(j);
    }

    void RegexAllOfComponent::emitJSON(json& j) const
    {
        // A RegexAllOfComponent is represented as a JSON array of its "musts" entries.
        j = json::array(); // Make j an array
        for(const auto& entry_ref : this->musts) {
            json entry_val;
            entry_ref.emitJSON(entry_val);
            j.push_back(entry_val);
        }
    }

    // Regex
    void Regex::emitJSON(json& j) const
    {
        j["isPath"] = (this->rtag == RegexKindTag::Path);
        j["isChar"] = (this->ctag == RegexCharInfoTag::Unicode); // true for Unicode, false for Char

        if (this->preanchor != nullptr) {
            json pre_val;
            this->preanchor->emitJSON(pre_val);
            j["preanchor"] = pre_val;
        } else {
            j["preanchor"] = nullptr;
        }

        if (this->postanchor != nullptr) {
            json post_val;
            this->postanchor->emitJSON(post_val);
            j["postanchor"] = post_val;
        } else {
            j["postanchor"] = nullptr;
        }

        // 're' is mandatory based on jparse logic, but good practice to check.
        if (this->re != nullptr) {
            json re_val;
            this->re->emitJSON(re_val);
            j["re"] = re_val;
        } else {
            j["re"] = nullptr; // Should ideally not happen if object is valid
        }
    }

    RegexOpt* RegexOpt::jparse(json j)
    {
        if(!j.is_object()) {
            BREX_ABORT("RegexOpt JSON is not an object or is null");
        }
        if(!j.contains("tag") || !j["tag"].is_string()) {
            BREX_ABORT("RegexOpt JSON missing 'tag' or 'tag' is not a string");
        }
        auto tag_str = j["tag"].get<std::string>();

        // Dispatch to derived class jparse methods based on tag_str
        if(tag_str == "LiteralOpt") {
            return LiteralOpt::jparse(j);
        }
        else if(tag_str == "CharRangeOpt") { return CharRangeOpt::jparse(j); }
        else if(tag_str == "CharClassDotOpt") { return CharClassDotOpt::jparse(j); }
        else if(tag_str == "NamedRegexOpt") { return NamedRegexOpt::jparse(j); }
        else if(tag_str == "EnvRegexOpt") { return EnvRegexOpt::jparse(j); }
        else if(tag_str == "StarRepeatOpt") { return StarRepeatOpt::jparse(j); }
        else if(tag_str == "PlusRepeatOpt") { return PlusRepeatOpt::jparse(j); }
        else if(tag_str == "RangeRepeatOpt") { return RangeRepeatOpt::jparse(j); }
        else if(tag_str == "OptionalOpt") { return OptionalOpt::jparse(j); }
        else if(tag_str == "AnyOfOpt") { return AnyOfOpt::jparse(j); }
        else if(tag_str == "SequenceOpt") { return SequenceOpt::jparse(j); }
        else { 
            BREX_ABORT("Unknown RegexOpt tag"); // Should not happen if tag is validated
        }
    }

    RegexComponent* RegexComponent::jparse(json j)
    {
        if(!j.is_array()) {
            return RegexSingleComponent::jparse(j);
        }
        else {
            return RegexAllOfComponent::jparse(j);
        }
    }

}
