#!/usr/bin/env luajit
package.cpath = package.cpath .. ';?.dylib'
--[[
right now I've got to copy the following libs into this folder:
	libCommon.dylib
	libImage.dylib
	libImageLua.dylib
	libLuaCxx.dylib
I should make a Make script to do that or something...
--]]
local img = require 'libImageLua'	-- says it's missing Common::File::getExtension 
