#!/usr/bin/env luajit
package.cpath = package.cpath .. ';?.dylib'
local img = require 'libImageLua'	-- says it's missing Common::File::getExtension 
