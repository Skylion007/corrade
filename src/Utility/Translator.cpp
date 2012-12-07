/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "Translator.h"

namespace Corrade { namespace Utility {

std::set<Translator*>* Translator::instances() {
    static std::set<Translator*>* _instances = new std::set<Translator*>;
    return _instances;
}

std::string* Translator::_locale() {
    static std::string* locale = new std::string;
    return locale;
}

void Translator::setLocale(const std::string& locale) {
    *_locale() = locale;

    /* Reload dynamically set languages */
    for(auto it = instances()->cbegin(); it != instances()->cend(); ++it) {
        /* Dynamic filename */
        if(!(*it)->primaryDynamicFilename.empty())
            /* primaryDynamicFilename is cleared, pass a copy to avoid loading
               empty filename */
            (*it)->setPrimary((*it)->primaryDynamicFilename.substr(0));

        /* Dynamic group */
        else if((*it)->primaryDynamicGroup)
            (*it)->setPrimary((*it)->primaryDynamicGroup, true);
    }
}

Translator::~Translator() {
    for(auto it = localizations.cbegin(); it != localizations.cend(); ++it)
        delete it->second;

    /* Destroy configuration files, if present. Translations loaded directly
        from groups should be deleted by caller. */
    delete primaryFile;
    delete fallbackFile;

    instances()->erase(this);
}

void Translator::setPrimary(const std::string& file) {
    /* Cleaning primary translation */
    if(file.empty()) return setPrimary(nullptr);

    Configuration* c = new Configuration(replaceLocale(file), Configuration::Flag::ReadOnly);
    setPrimary(c);
    primaryFile = c;

    /* Dynamic translations */
    primaryDynamicGroup = nullptr;
    if(file.find_first_of('#') != std::string::npos)
        primaryDynamicFilename = file;
    else
        primaryDynamicFilename.clear();
}

void Translator::setFallback(const std::string& file) {
    /* Cleaning fallback translation */
    if(file.empty()) return setFallback(nullptr);

    Configuration* c = new Configuration(file, Configuration::Flag::ReadOnly);
    setFallback(c);
    fallbackFile = c;
}

void Translator::setPrimary(const Corrade::Utility::ConfigurationGroup* group, bool dynamic) {
    /* Dynamic translations */
    primaryDynamicFilename.clear();
    if(dynamic && group) {
        primaryDynamicGroup = group;
        primary = group->group(locale());
    } else {
        primaryDynamicGroup = nullptr;
        primary = group;
    }

    /* Delete previous file */
    delete primaryFile;
    primaryFile = nullptr;

    /* Reload all localizations from new files */
    for(auto it = localizations.cbegin(); it != localizations.cend(); ++it)
        get(it->first, it->second, 0);
}

void Translator::setFallback(const Corrade::Utility::ConfigurationGroup* group) {
    /* Delete previous file */
    delete fallbackFile;
    fallbackFile = nullptr;

    fallback = group;

    /* If primary is dynamic */
    if(primaryDynamicGroup) setPrimary(primaryDynamicGroup, true);

    /* Reload all localizations from new files */
    for(auto it = localizations.cbegin(); it != localizations.cend(); ++it)
        get(it->first, it->second, 0);
}

const std::string* Translator::get(const std::string& key) {
    /* First try to find existing localization */
    auto found = localizations.find(key);
    if(found != localizations.end()) return found->second;

    /* If not found, load from configuration */
    std::string* text = new std::string;
    localizations.insert(std::make_pair(key, text));
    get(key, text, 0);

    return text;
}

bool Translator::get(const std::string& key, std::string* text, int level) const {
    const ConfigurationGroup* g;
    switch(level) {
        case 0: g = primary; break;
        case 1: g = fallback; break;
        default: text->clear(); return false;
    }

    /* If not in current level, try next */
    if(!g || !g->value(key, text)) return get(key, text, ++level);

    return true;
}

std::string Translator::replaceLocale(const std::string& filename) const {
    std::size_t pos = filename.find_first_of('#');
    if(pos == std::string::npos) return filename;

    return filename.substr(0, pos) + locale() + filename.substr(pos+1);
}

}}
