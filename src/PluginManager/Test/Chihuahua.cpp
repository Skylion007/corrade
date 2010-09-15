/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "Dog.h"

namespace Map2X { namespace PluginManager { namespace Test {

class Chihuahua: public Dog {
    public:
        inline Chihuahua(AbstractPluginManager* manager = 0, const std::string& plugin = ""):
            Dog(manager, plugin) {}

        virtual std::string name() { return "Rodriguez"; }
};

}}}

PLUGIN_REGISTER(Map2X::PluginManager::Test::Chihuahua,
                "cz.mosra.Map2X.PluginManager.Test.AbstractAnimal/1.0")
