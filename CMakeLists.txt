cmake_minimum_required(VERSION 3.5.0)

include(../PluginRules.cmake)

add_sources(${PLUGIN_NAME}
	StimControl.cpp
	StimControl.hpp
	StimControlEditor.cpp
	StimControlEditor.hpp
)

