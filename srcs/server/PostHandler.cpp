/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:42:38 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:42:39 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

std::string get_content_Type(std::string file);

static char fromHex(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    return 0;
}

static std::string urlDecode(const std::string &src)
{
    std::string result;

    for (size_t i = 0; i < src.size(); ++i)
    {
        if (src[i] == '%' && i + 2 < src.size() && std::isxdigit(src[i+1]) && std::isxdigit(src[i+2]))
        {
            char h1 = src[i + 1];
            char h2 = src[i + 2];
            char decoded = (fromHex(h1) << 4) | fromHex(h2);
            result.push_back(decoded);
            i += 2;
        }
        else if (src[i] == '+')
        {
            result.push_back(' ');
        }
        else
        {
            result.push_back(src[i]);
        }
    }
    return result;
}

std::map<std::string, std::string> parse_simple_json(const std::string &body)
{
    std::map<std::string, std::string> result;
    std::string key;
    std::string value;
    bool        in_key = false;
    bool        in_value = false;
    bool        reading_key = true;

    for (size_t i = 0; i < body.size(); i++)
    {
        char c = body[i];
        if (c == '"')
        {
            if (!in_key && reading_key)
            {
                in_key = true;
                key.clear();
            }
            else if (in_key && reading_key)
            {
                in_key = false;
                reading_key = false;
            }
            else if (!in_value && !reading_key)
            {
                in_value = true;
                value.clear();
            }
            else if (in_value && !reading_key)
            {
                in_value = false;
                result[key] = urlDecode(value);
                reading_key = true;
            }
        }
        else if (in_key)
            key += c;
        else if (in_value)
            value += c;
    }
    return (result);
}


std::map<std::string, std::string> parse_raw_body(const std::string &body)
{
    std::map<std::string, std::string> result;
    std::string pair;
    std::istringstream ss(body);

    while (std::getline(ss, pair, '&'))
    {
        size_t pos = pair.find('=');
        if (pos != std::string::npos)
        {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            //some chars are transformed by the browser like %10 or space becomes +
            key = urlDecode(key);
            value = urlDecode(value);
            result[key] = value;
        }
    }
    return result;
}

HttpResponse Server::handle_json(const std::string &body, const std::string &path)
{
    std::map<std::string, std::string> sorted_body = parse_simple_json(body);
    std::map<std::string, std::string>::iterator it;

    it = sorted_body.find("filename");
    
    //file upload
    if (it != sorted_body.end())
    {
        std::string file_path = path + "/" + it->second;

        int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            return generate_error_response(500, " Internel Server Error", "File exists but read failed");

        //if there is content write it
        it = sorted_body.find("content");
        if (it != sorted_body.end())
            write(fd, (it->second).c_str(), (it->second).size());
        close(fd);
        return generate_success_response(201, "OK", "File uploaded successfully");
    }

    //random data
    else
    {
        for (it = sorted_body.begin(); it != sorted_body.end(); ++it)
        {
            std::cout << it->first << " = " << it->second << std::endl;
        }
    }
    return generate_success_response(200, "OK", "Data processed successfully");
}

std::string Server::get_boundary(const std::string &content_type)
{
    std::string boundary;
    size_t sep;

    //find first ; after cntent-type
    sep = content_type.find(';');
    if (sep == std::string::npos)
        throw std::runtime_error("");
    //get parameters part
    std::string params = content_type.substr(sep + 1);
    //find boundary=
    sep = params.find("boundary=");
    if (sep == std::string::npos)
        throw std::runtime_error("");
    //extract value after boundary
    boundary = params.substr(sep + 9);
    if (boundary.empty())
         throw std::runtime_error("");
    //if another parameter follows cut it off
    sep = boundary.find(';');
    if (sep != std::string::npos)
        boundary = boundary.substr(0, sep);
    //trim spaces ( /r/n)
    while (!boundary.empty() && isspace(boundary[0]))
        boundary.erase(0, 1);
    while (!boundary.empty() && isspace(boundary[boundary.size()-1]))
        boundary.erase(boundary.size()-1);
    //add -- for use in body parsing
    boundary = "--" + boundary;
    return (boundary);
}

/* Example of a multipart request

    POST /upload HTTP/1.1\r\n
    Host: localhost:8080\r\n
    User-Agent: curl/7.68.0\r\n
    Accept: \r\n
    Content-Length: 307\r\n
    Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryabc123\r\n
    \r\n
    ------WebKitFormBoundaryabc123\r\n
    Content-Disposition: form-data; name="username"\r\n
    \r\n
    harold\r\n
    ------WebKitFormBoundaryabc123\r\n
    Content-Disposition: form-data; name="file"; filename="hello.txt"\r\n
    Content-Type: text/plain\r\n
    \r\n
    Hello this is the content of the file.\r\n
    ------WebKitFormBoundaryabc123--\r\n
*/

HttpResponse Server::handle_multipart(const std::string &body, const std::string &path, const std::string &content_type)
{
    std::string boundary;
    std::string closing_boundary = boundary + "--";
    std::map<std::string, std::string> content_received;
    size_t start = 0;
    int files_created = 0;

    try 
    {
        boundary = get_boundary(content_type);
    }
    catch (std::exception &e){ return (generate_error_response(400, "Bad Request", "Missing of invalid boundary")); }

    while (true)
    {
        //find the next boundary
        size_t pos = body.find(boundary, start);
        if (pos == std::string::npos)
            break;

        //stop if closing boundary
        if (body.compare(pos, closing_boundary.size(), closing_boundary) == 0)
            break;

        //move past boundary + /r/n
        pos += boundary.size();
        if (body.compare(pos, 2, "\r\n") == 0)
            pos += 2;

        //find headers/body separator
        size_t header_end = body.find("\r\n\r\n", pos);
        if (header_end == std::string::npos)
            return generate_error_response(400, "Bad Request", "Malformed part");

        std::string headers = body.substr(pos, header_end - pos);
        size_t content_start = header_end + 4;

        //find next boundary to know content end
        size_t next_boundary = body.find(boundary, content_start);
        if (next_boundary == std::string::npos)
            return generate_error_response(400, "Bad Request", "Boundary mismatch");

        std::string content = body.substr(content_start, next_boundary - content_start);

        //parse Content-Disposition
        size_t cd_pos = headers.find("Content-Disposition:");
        if (cd_pos == std::string::npos)
            continue;

        std::string cd_line = headers.substr(cd_pos);
        size_t name_pos = cd_line.find("name=");
        size_t filename_pos = cd_line.find("filename=");

        //if there is a filename -> create a file
        if (filename_pos != std::string::npos)
        {
            //extract the filename
            size_t start_q = cd_line.find("\"", filename_pos);
            size_t end_q   = cd_line.find("\"", start_q + 1);
            std::string filename = cd_line.substr(start_q + 1, end_q - (start_q + 1));

            if (filename.empty())
                return generate_error_response(400, "Bad Request", "Missing filename");

            //save file
            int fd = open((filename).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
                return generate_error_response(500, "Internel Server Error", "File exists but read failed");
            write(fd, (content).c_str(), (content).size());
            close(fd);
            content_received[filename] = content;
            files_created++;
        }

        //if no files and just a field
        else if (name_pos != std::string::npos)
        {
            //extract field name
            size_t start_q = cd_line.find("\"", name_pos);
            size_t end_q   = cd_line.find("\"", start_q + 1);
            std::string fieldname = cd_line.substr(start_q + 1, end_q - (start_q + 1));
            content_received[fieldname] = content;
        }

        //move to the next part
        start = next_boundary;
    }
    //DEBUGGING
    for (std::map<std::string, std::string>::const_iterator it = content_received.begin(); it != content_received.end(); ++it)
    {
        std::cout << it->first << " = " << it->second << std::endl;
    }

    if (files_created > 0)
        return generate_success_response(201, "OK", "File(s) created successfully");
    return generate_success_response(200, "OK", "Form(s) processed successfully");
}

HttpResponse Server::handle_url_encoded(const std::string &body, const std::string &path)
{
    std::map<std::string, std::string> sorted_body = parse_raw_body(body);
    std::map<std::string, std::string>::iterator it;

    it = sorted_body.find("filename");
    //file upload
    if (it != sorted_body.end())
    {
        std::string file_path = path + "/" + it->second;

        int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            return generate_error_response(500, "Internel Server Error", "File exists but read failed");

        //if there is content write it
        it = sorted_body.find("content");
        if (it != sorted_body.end())
            write(fd, (it->second).c_str(), (it->second).size());
        close(fd);

        return generate_success_response(201, "OK", "File uploaded successfully");
    }

    //random data
    else
    {
        return generate_error_response(400, "Bad Request", "Missing filename");
    }
    return generate_success_response(200, "OK", "Data proccessed successfully");
}

HttpResponse    Server::handle_file_response(const std::string &target, const std::string &body, int flag)
{
    //find the parent directory
    std::string parent_dir;
    std::string parent_dir_path;
    std::string file_path = "./www" + target;

    size_t slash_pos = target.find_last_of("/");
    if (target.find_first_of("/") == target.find_last_of("/"))
        parent_dir = "/";
    else
        parent_dir = target.substr(0, slash_pos);

    parent_dir_path = "./www" + parent_dir;

    //check if parent directory exist
    struct stat dir_st;
    if (stat(parent_dir_path.c_str(), &dir_st) < 0 || !S_ISDIR(dir_st.st_mode))
    {
        if (errno == EACCES)
            return generate_error_response(403, "Forbidden", "You do not have permission to access this resource.");
        else
            return generate_error_response(404, "Not Found", "Parent directory does not exist");
    }

    //check parent directory write and x permission
    if (access(parent_dir_path.c_str(), W_OK | X_OK) < 0)
    {
        return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource");
    }

    //check if it is in a location in the config
    std::map<std::string, LocationBloc>::iterator it = this->_locations.find(parent_dir);

    if (it == this->_locations.end())
    {
        generate_error_response(404, "Not Found", "The requested location does not exist");
    }

    //get the location bloc of the parent directory
    LocationBloc location = it->second;
    
    //check if POST is an accepted method in the location
    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "POST") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve POST method");

    //check for body size
    if(body.size() > location.client_max_body_size)
        return generate_error_response(405, "Method Not Allowed", "Body too large");

    //if file exists already check for W persmission
    if (flag == 2)
    {
        if (access(file_path.c_str(), W_OK) < 0)
        {
            return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource");
        }
    }

    //create new file or overwite
    int fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        return generate_error_response(500, "Internal Server Error", "Failed to create new file.");
    }

    //write body content if any
    if (!body.empty())
        write(fd, body.c_str(), body.size());

    close(fd);
    if (flag == 2)
        return generate_success_response(200, "OK", "File modified successfully");
    return generate_success_response(201, "OK", "File created successfully");
}

HttpResponse    Server::generate_post_response(HttpRequest &req)
{
    HttpResponse res;

    std::string target = req.getTarget();
    std::string path = "./www" + target + "/";

    struct stat st;

    //Check existence
    if (stat(path.c_str(), &st) < 0)
    {
        if (errno == EACCES)
            return generate_error_response(403, "Forbidden", "You do not have permission to access this resource.");
        //if not exist make it a file if parent directory exists and permits POST
        if (errno == ENOENT)
        {
            //VERIFY WHEN YOU DO UPLOAD/QWERTY/
            //you should not create a file if qwerty/ (check upstream)
            return handle_file_response(target, req.getBody(), 1);
        }
        else
        {
            return generate_error_response(404, "Not Found", "The requested resource does not exist.");
        }
    }

    //if target is a directory we have to host the file there
    if (S_ISDIR(st.st_mode))
    {
        //check directory write and x permission
        if (access(path.c_str(), W_OK | X_OK) < 0)
        {
            return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource");
        }

        //get the information on that location
        std::map<std::string, LocationBloc>::iterator it1 = this->_locations.find(target);

        if (it1 == this->_locations.end())
        {
            generate_error_response(404, "Not Found", "The requested location does not exist");
        }

        //get the location bloc of the parent directory
        LocationBloc location = it1->second;

        //check if POST is an accepted method in the location
        if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "POST") == location.allowed_methods.end())
            return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve POST method");

        //check for body size
        if(req.getBody().size() > location.client_max_body_size)
            return generate_error_response(405, "Method Not Allowed", "Body too large");

        //There are three types of POST methods (multipart, url encoded, json) that need parsing

        std::map<std::string, std::string>::const_iterator it = req.getHeaders().find("Content-Type");

        //if the client didn't specify a content type we can't parse it 
        if (it == req.getHeaders().end())
        {
            return (generate_error_response(400, "Bad Request", "No Content-Type mentioned"));
        }

        //get the content type
        std::string content_type = it->second;

        //if there is an ; isolate the pure content type
        size_t sep = content_type.find(';');
        if (sep != std::string::npos)
            content_type = content_type.substr(0, sep);

        //url encoded -> body is in key value pairs and represent simple data to be put in a file our case
        if (content_type == "application/x-www-form-urlencoded")
        {
            return (handle_url_encoded(req.getBody(), path));
        }

        //same as url encoded but with json format
        else if (content_type == "application/json")
        {
            return (handle_json(req.getBody(), path));
        }

        //if it is multipart form data, there are one or more files to do
        else if (content_type == "multipart/form-data")
        {
            return (handle_multipart(req.getBody(), path, it->second));
        }
        //if any other Content-Type we can't safely parse it so return 415
        else
        {
            return (generate_error_response(415, "415 Unsupported Media Type", "Unrecognised Content-Type"));
        }
    }
    //if it is a file thar already exists check for permission and ovewrite
    return (handle_file_response(target, req.getBody(), 2));
}

