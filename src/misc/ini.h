/**
 * The MIT License (MIT)
 * Copyright (c) <2015> <carriez.md@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
  *
  */

#ifndef INI_HPP
#define INI_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>



struct SimpleIniConfig {
    SimpleIniConfig(std::string filePath) {
        std::string data;
        {
            std::ostringstream sstr;
            sstr << std::ifstream(filePath).rdbuf();
            data = sstr.str();
        }

        auto rgx = std::regex("^(?!\\s*[#;])\\s*([^=]+?)\\s*=\\s*(.*)\\s*$");

        std::smatch match;
        while(std::regex_search(data, match, rgx)){
            auto key = match[1].str();
            auto value = match[2].str();
            values[key] = value;
            data = match.suffix().str();
        }
    }

    template<typename T>
    bool Read(const std::string key, T* dest, T defaultValue) const {

        auto v = values.find(key);
        if (v != values.end()) {
            auto& valueStr = v->second;

            if constexpr (std::is_same_v<T, std::string>) {
                *dest = valueStr;
            }
            else if constexpr (std::is_same_v<T, char*>) {
                *dest = valueStr;
            }
            else {
                std::stringstream ss(valueStr);
                T value;
                ss >> value;
                if (ss.fail()) {
                    return false;
                }
                *dest = value;
                return true;
            }
            return true;
        }

        *dest = defaultValue;
        return false;
    }

    template<typename T>
    bool Read(const std::string key, T* dest) const {
        return Read(key, dest, *dest);
    }

private:
    std::unordered_map<std::string, std::string> values;
};

#endif // INI_HPP

