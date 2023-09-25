#include "resources/rapidXMLUtility.h"
#include <cstring>
#include <functional>

namespace hpl::rapidxml {

    void SetAttributeString(::rapidxml::xml_node<char>* node, const char* asName, const char* asVal) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            attr->value(node->document()->allocate_string(asVal));
        } else {
            attr = node->document()->allocate_attribute(node->document()->allocate_string(asName), node->document()->allocate_string(asVal));
            node->append_attribute(attr);
        }
    }

    void SetAttributeFloat(::rapidxml::xml_node<char>* node, const char* asName, float afVal) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%f", afVal);
        SetAttributeString(node, asName, buf);
    }
    
    void SetAttributeInt(::rapidxml::xml_node<char>* node, const char* asName, int alVal) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%d", alVal);
        SetAttributeString(node, asName, buf);
    }
    
    void SetAttributeBool(::rapidxml::xml_node<char>* node, const char* asName, bool abVal) {
        SetAttributeString(node, asName, abVal ? "true" : "false");
    }
    
    void SetAttributeVector2f(::rapidxml::xml_node<char>* node, const char* asName, const cVector2f& avVal) {
        char buf[512];
        sprintf(buf, "%g %g", avVal.x, avVal.y);
        SetAttributeString(node, asName, buf);
    }
    
    void SetAttributeVector3f(::rapidxml::xml_node<char>* node, const char* asName, const cVector3f& avVal) {
        char buf[512];
        sprintf(buf, "%g %g %g", avVal.x, avVal.y, avVal.z);
        SetAttributeString(node, asName, buf);
    }

    void SetAttributeColor(::rapidxml::xml_node<char>* node, const char* asName, const cColor& aVal) {
        char buf[512];
        sprintf(buf, "%g %g %g %g", aVal.r, aVal.g, aVal.b, aVal.a);
        SetAttributeString(node, asName, buf);
    }

    const char* GetAttributeString(::rapidxml::xml_node<char>* node, const char* asName, const char* asDefault) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            return attr->value();
        }
        return asDefault;
    }

    float GetAttributeFloat(::rapidxml::xml_node<char>* node, const char* asName, float afDefault) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            return atof(attr->value());
        }
        return afDefault;
    }

    int GetAttributeInt(::rapidxml::xml_node<char>* node, const char* asName, int alDefault) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            return atoi(attr->value());
        }
        return alDefault;
    }

    bool GetAttributeBool(::rapidxml::xml_node<char>* node, const char* asName, bool abDefault) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            return std::strcmp(attr->value(), "true") == 0;
        }
        return abDefault;
    }

    enum class SplitStatus {
        Continue,
        Break,
    };

    void SplitScan(const char* value, std::function<SplitStatus(const char*)> callback) {
        char buffer[32];
        size_t i = 0;
        for (const char* p = value; *p; ++p) {
            if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t' || *p == ',') {
                buffer[i] = '\0';
                SplitStatus status = callback(buffer);
                if (status == SplitStatus::Break) {
                    return;
                }
                i = 0;
            }

            buffer[i++] = *p;
        }
    }

    cVector2f GetAttributeVector2f(::rapidxml::xml_node<char>* node, const char* asName, const cVector2f& avDefault) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            cVector2f result;
            size_t index = 0;
            SplitScan(attr->value(), [&](const char* value) {
                result.v[index++] = atof(value);
                if (index >= 2) {
                    return SplitStatus::Break;
                }
                return SplitStatus::Continue;
            });
            return result;
        }
        return avDefault;
    }

    cVector2l GetAttributeVector2l(::rapidxml::xml_node<char>* node, const char* asName, const cVector2l& avDefault) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            cVector2l result;
            size_t index = 0;
            SplitScan(attr->value(), [&](const char* value) {
                result.v[index++] = atol(value);
                if (index >= 2) {
                    return SplitStatus::Break;
                }
                return SplitStatus::Continue;
            });
            return result;
        }
        return avDefault;
    }

    cVector3f GetAttributeVector3f(::rapidxml::xml_node<char>* node, const char* asName, const cVector3f& avDefault) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            cVector3f result;
            size_t index = 0;
            SplitScan(attr->value(), [&](const char* value) {
                result.v[index++] = atof(value);
                if (index >= 3) {
                    return SplitStatus::Break;
                }
                return SplitStatus::Continue;
            });
            return result;
        }
        return avDefault;
    }
    cColor GetAttributeColor(::rapidxml::xml_node<char>* node, const char* asName, const cColor& aDefault) {
        auto* attr = node->first_attribute(asName);
        if (attr) {
            cColor result;
            size_t index = 0;
            SplitScan(attr->value(), [&](const char* value) {
                result.v[index++] = atof(value);
                if (index >= 4) {
                    return SplitStatus::Break;
                }
                return SplitStatus::Continue;
            });
            return result;
        }
        return aDefault;
    }

} // namespace hpl::rapidxml