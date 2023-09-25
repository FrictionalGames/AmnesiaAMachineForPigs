#pragma once

#include "graphics/Color.h"
#include "math/MathTypes.h"
#include "resources/rapidXML/rapidxml.hpp"

#include <string_view>


namespace hpl::rapidxml {
    void SetAttributeString(::rapidxml::xml_node<char>* node, const char* asName, const char* asVal);
    void SetAttributeFloat(::rapidxml::xml_node<char>* node, const char* asName, float afVal);
    void SetAttributeInt(::rapidxml::xml_node<char>* node, const char* asName, int alVal);
    void SetAttributeBool(::rapidxml::xml_node<char>* node, const char* asName, bool abVal);
    void SetAttributeVector2f(::rapidxml::xml_node<char>* node, const char* asName, const cVector2f& avVal);
    void SetAttributeVector3f(::rapidxml::xml_node<char>* node, const char* asName, const cVector3f& avVal);
    void SetAttributeColor(::rapidxml::xml_node<char>* node, const char* asName, const cColor& aVal);

    const char* GetAttributeString(::rapidxml::xml_node<char>* node, const char* asName, const char* asDefault="");
    float GetAttributeFloat(::rapidxml::xml_node<char>* node, const char* asName, float afDefault=0);
    int GetAttributeInt(::rapidxml::xml_node<char>* node, const char* asName, int alDefault=0);
    bool GetAttributeBool(::rapidxml::xml_node<char>* node, const char* asName, bool abDefault=false);
    cVector2f GetAttributeVector2f(::rapidxml::xml_node<char>* node, const char* asName, const cVector2f& avDefault=0);
    cVector2l GetAttributeVector2l(::rapidxml::xml_node<char>* node, const char* asName, const cVector2l& avDefault=0);
    cVector3f GetAttributeVector3f(::rapidxml::xml_node<char>* node, const char* asName, const cVector3f& avDefault=0);
    cColor GetAttributeColor(::rapidxml::xml_node<char>* node, const char* asName, const cColor& aDefault=cColor(0,0));
}