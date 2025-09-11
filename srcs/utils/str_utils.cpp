/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   str_utils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 13:14:02 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/10 13:14:21 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

std::vector<std::string> ft_split(const std::string &line, const std::string &separators)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t pos;

    while (start < line.size())
    {
        // find first non-separator
        start = line.find_first_not_of(separators, start);
        if (start == std::string::npos)
            break;

        // find next separator after that
        pos = line.find_first_of(separators, start);
        if (pos == std::string::npos)
        {
            tokens.push_back(line.substr(start));
            break;
        }
        else
        {
            tokens.push_back(line.substr(start, pos - start));
            start = pos + 1;
        }
    }

    return tokens;
}

int get_closing_bracket_index(std::string &str)
{
    int     count = 1;
    int     i = 7;

    while (i < str.size())
    {
        if (str[i] == '{')
            count++;
        else if (str[i] == '}')
            count--;
        if (count == 0)
            return (i);
        i++;
    }
    return (i);
}

std::string commenttrim(std::string &str)
{
    std::string trimmed;
    size_t      hash_pos = str.find_first_of("#");

    if (hash_pos == std::string::npos)
        return (str);
    trimmed = str.substr(0, hash_pos);
    return (trimmed);
}

bool    is_closed(std::string str)
{
    int     count = 0;;
    size_t  i = 0;

    while (i < str.size())
    {
        if (str[i] == '{')
            count++;
        else if (str[i] == '}')
            count--;
        if (count < 0)
            return (false);
        i++;
    }

    if (count != 0)
        return (false);
    return (true);
}

std::string spacetrim(std::string &line)
{
    std::ostringstream  str;
    bool                in_space = false;

    for (size_t i = 0; i < line.size(); i++)
    {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\r' || line[i] == '\f' || line[i] == '\v')
        {
            if (!in_space)
            {
                str << ' ';
                in_space = true;
            }
        }
        else
        {
            str << line[i];
            in_space = false;
        }
    }
    return (str.str());
}

std::string brackettrim(std::string &line)
{
    std::ostringstream  str;

    for (size_t i = 0; i < line.size(); i++)
    {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\r' || line[i] == '\f' || line[i] == '\v')
        {
            if (i != 0 && (line[i - 1] == '}' || line[i - 1] == '{' || line[i - 1] == ';'))
                continue ;
            else if ((i != line.size() - 1) && (line[i + 1] == '}' || line[i + 1] == '{') || line[i + 1] == ';')
                continue ;
            str << line[i];
        }
        else
        {
            str << line[i];
        }
    }
    return (str.str());
}


//put it in utils
std::string strtrim(std::string &str)
{
    size_t      first_valid_char;
    size_t      last_valid_char;
    size_t      length;
    std::string trimmed;

    if (str.empty())
        return ("");
    //finding position of first non space char
    first_valid_char = str.find_first_not_of(" \t\v\f\r");
     if (first_valid_char == std::string::npos)
        return ""; // string is all whitespace
    //finding position of last non space char
    last_valid_char = str.find_last_not_of(" \t\v\f\r");
    //finding the length
    length = last_valid_char - first_valid_char + 1;
    //creating the string
    trimmed = str.substr(first_valid_char, length);
    return (trimmed);
}

size_t safe_atosize_t(const std::string &str)
{
    if (str.empty())
        throw std::runtime_error(str + ": missing value");

    // Check all chars are digits
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (!std::isdigit(str[i]))
            throw std::runtime_error(str + ": invalid characters: " + str);
    }

    // Convert using stringstream
    std::istringstream iss(str);
    std::string str_value = iss.str();
    size_t value = 0;
    iss >> value;

    if (iss.fail())
        throw std::runtime_error(str + ": conversion failed");
    // Overflow check
    if (str_value.size() > 20)
        throw std::runtime_error(str + ": value too large");
    if (value > std::numeric_limits<size_t>::max())
        throw std::runtime_error(str + ": value too large");
    return (value);
}
