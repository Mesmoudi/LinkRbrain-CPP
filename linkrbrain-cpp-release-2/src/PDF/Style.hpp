#ifndef LINKRBRAIN2019__SRC__PDF__STYLE_HPP
#define LINKRBRAIN2019__SRC__PDF__STYLE_HPP


#include <map>
#include <string>

#include "./StyleAttribute.hpp"
#include "./Color.hpp"

#include "Types/NullablePointer.hpp"


namespace PDF {

    struct Style {
    public:

        // hierarchy
        const bool has_parent() const {
            return parent.is_set();
        }
        Style& add_child(const std::string& key, Style style) {
            style.parent.set(this);
            auto it = children.insert({key, style});
            return it.first->second;
        }
        Style& get_child(const std::initializer_list<std::string>& keys) {
            Style* result = this;
            for (const auto& key : keys) {
                auto it = result->children.find(key);
                if (it == result->children.end()) {
                    result = & (result->add_child(key, {}));
                } else {
                    result = & it->second;
                }
            }
            return *result;
        }
        const bool ensure_hierarchy() {
            for (auto& [key, child] : children) {
                child.parent.set(this);
                child.ensure_hierarchy();
            }
            return true;
        }
        void set(const Style& style) {
            font_name = style.font_name;
            font_size = style.font_size;
            line_height = style.line_height;
            paragraph_spacing = style.paragraph_spacing;
            font_color = style.font_color;
            background_color = style.background_color;
            //
            bold = style.bold;
            italic = style.italic;
            underline = style.underline;
            link = style.link;
            //
            margin_top = style.margin_top;
            margin_bottom = style.margin_bottom;
            margin_left = style.margin_left;
            margin_right = style.margin_right;
        }
        void set(const Style& style, const std::initializer_list<std::string>& keys) {
            get_child(keys).set(style);
        }
        Types::NullablePointer<Style> parent;
        std::map<std::string, Style> children;

        // fonts
        StyleAttribute<std::string> font_name;
        StyleAttribute<float> font_size;
        StyleAttribute<float> line_height;
        StyleAttribute<float> paragraph_spacing;
        StyleAttribute<Color> font_color;
        StyleAttribute<Color> background_color;
        // unimplemented yet
        StyleAttribute<bool> bold;
        StyleAttribute<bool> italic;
        StyleAttribute<bool> underline;
        StyleAttribute<std::string> link;
        // margins
        StyleAttribute<float> margin_top;
        StyleAttribute<float> margin_bottom;
        StyleAttribute<float> margin_left;
        StyleAttribute<float> margin_right;

        // combine styles
        Style operator | (const Style& other) const {
            Style result = *this;
            result |= other;
            return result;
        }
        Style& operator |= (const Style& other) {
            font_name |= other.font_name;
            font_size |= other.font_size;
            line_height |= other.line_height;
            paragraph_spacing |= other.paragraph_spacing;
            font_color |= other.font_color;
            background_color |= other.background_color;
            //
            bold |= other.bold;
            italic |= other.italic;
            underline |= other.underline;
            link |= other.link;
            //
            margin_top |= other.margin_top;
            margin_bottom |= other.margin_bottom;
            margin_left |= other.margin_left;
            margin_right |= other.margin_right;
            //
            return *this;
        }
        const Style get_computed() const {
            Style result = *this;
            const Style* style = this;
            while (style->has_parent()) {
                style = style->parent.get_pointer();
                result = *style | result;
            }
            return result;
        }
        const Style get_computed(const std::initializer_list<std::string>& keys) const {
            const Style* style = this;
            for (const auto& key : keys) {
                auto it = style->children.find(key);
                if (it == style->children.end()) {
                    break;
                } else {
                    style = & it->second;
                }
            }
            return style->get_computed();
        }
    };

    static const Style no_style;

    static Style default_style = {
        .font_name = "Helvetica",
        .font_size = 10,
        .line_height = 1.5,
        .paragraph_spacing = 0.75,
        .font_color = {0, 0, 0},
        //
        .bold = false,
        .italic = false,
        .underline = false,
        //
        .margin_top = 1.5,
        .margin_bottom = 1.5,
        .margin_left = 2.,
        .margin_right = 2.,
        //
        .children = {
            {"table", {
                .margin_left = .5,
                .margin_right = .5,
                .children = {
                    {"header", {
                        .font_color = {1,1,1},
                        .background_color = {.1,.1,.1}
                    }},
                    {"odd", {
                        .background_color = {.9,.9,.9}
                    }}
                }
            }}
        },
    };
    static bool default_style_is_set = default_style.ensure_hierarchy();

} // PDF


#endif // LINKRBRAIN2019__SRC__PDF__STYLE_HPP
