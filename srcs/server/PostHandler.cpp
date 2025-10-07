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
    return (result);
}

HttpResponse Server::handle_json(const std::string &body, const std::string &path, LocationBloc &location)
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
            return generate_error_response(500, " Internel Server Error", "File exists but read failed", location);

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
        //if no upload permission
        if (!location.upload_enable)
            return generate_error_response(403, "Forbidden", "You do not have permissions to upload files", location);

        std::string file_path = path + "/" + generate_upload_filename();

        int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            return generate_error_response(500, "Internel Server Error", "File exists but read failed", location);
        write(fd, (body).c_str(), (body).size());
        close(fd);
            return generate_success_response(201, "Created", "File uploaded successfully");;
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

HttpResponse Server::handle_multipart(const std::string &body, const std::string &path, const std::string &content_type, LocationBloc &location)
{
    std::string boundary;
    size_t start = 0;
    int files_created = 0;
    (void)path;

    try 
    {
        boundary = get_boundary(content_type);
    }
    catch (std::exception &e){ return (generate_error_response(400, "Bad Request", "Missing of invalid boundary", location)); }

    std::string closing_boundary = boundary + "--";
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
            return generate_error_response(400, "Bad Request", "Malformed part", location);

        std::string headers = body.substr(pos, header_end - pos);
        size_t content_start = header_end + 4;

        //find next boundary to know content end
        size_t next_boundary = body.find(boundary, content_start);
        if (next_boundary == std::string::npos)
            return generate_error_response(400, "Bad Request", "Boundary mismatch", location);

        std::string content = body.substr(content_start, next_boundary - content_start);

        //parse Content-Disposition
        std::cout << "HERE : " << content << std::endl;
        size_t cd_pos = headers.find("Content-Disposition:");
        if (cd_pos == std::string::npos)
            continue;

        std::string cd_line = headers.substr(cd_pos);
        size_t name_pos = cd_line.find("name=");
        size_t filename_pos = cd_line.find("filename=");

        //if there is a filename
        if (filename_pos != std::string::npos)
        {
            //if no upload permission
            if (!location.upload_enable)
                return generate_error_response(403, "Forbidden", "You do not have permissions to upload files", location);

            //extract the filename
            size_t start_q = cd_line.find("\"", filename_pos);
            size_t end_q   = cd_line.find("\"", start_q + 1);
            std::string filename = cd_line.substr(start_q + 1, end_q - (start_q + 1));

            if (filename.empty())
                return generate_error_response(400, "Bad Request", "Missing filename", location);

            //construct file path
            std::string file_path = location.upload_path + "/" + filename;
            //save file
            int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
                return generate_error_response(500, "Internel Server Error", "Read failed", location);
            write(fd, (content).c_str(), (content).size());
            close(fd);
            files_created++;
        }

        //if no files and just a field. dump body in a random file
        else if (name_pos != std::string::npos)
        {
            //if no upload permission
            if (!location.upload_enable)
                return generate_error_response(403, "Forbidden", "You do not have permissions to upload files", location);

            std::string file_path = path + "/" + generate_upload_filename();

            int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
                return generate_error_response(500, "Internel Server Error", "File exists but read failed", location);
            write(fd, (content).c_str(), (content).size());
            close(fd);
            return generate_success_response(201, "Created", "File uploaded successfully");;
        }
        //move to the next part
        start = next_boundary;
    }
    if (files_created > 0)
        return generate_success_response(201, "OK", "File(s) created successfully");
    return generate_success_response(200, "OK", "Form(s) processed successfully");
}

HttpResponse Server::handle_url_encoded(const std::string &body, const std::string &path, LocationBloc &location)
{
    std::map<std::string, std::string>              sorted_body = parse_raw_body(body);
    std::map<std::string, std::string>::iterator    it;
    std::string                                     file_path;

    it = sorted_body.find("filename");

    //construct full file path if it exists
    if (it != sorted_body.end())
        file_path = path + "/" + it->second;

    //if file exists already
    if (it != sorted_body.end() && access(file_path.c_str(), F_OK) == 0)
    {
        //if no upload permission
        if (!location.upload_enable)
            return generate_error_response(403, "Forbidden", "You do not have permissions to upload files", location);

        //if you don't have permission to write
        if (access(file_path.c_str(), W_OK) < 0)
        {
            return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource", location);
        }
        
        //overwrite the file
        int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            return generate_error_response(500, "Internel Server Error", "File exists but read failed", location);

        //if there is content write it
        it = sorted_body.find("content");
        if (it != sorted_body.end())
            write(fd, (it->second).c_str(), (it->second).size());
        close(fd);

        return generate_success_response(200, "OK", "File overwrite successfull");
    }

    //is file doesn't exist
    else if (it != sorted_body.end())
    {
        //if no upload permission
        if (!location.upload_enable)
            return generate_error_response(403, "Forbidden", "You do not have permissions to upload files", location);

        std::string file_path = path + "/" + it->second;

        int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            return generate_error_response(500, "Internel Server Error", "File exists but read failed", location);

        //if there is content write it
        it = sorted_body.find("content");
        if (it != sorted_body.end())
            write(fd, (it->second).c_str(), (it->second).size());
        close(fd);

        return generate_success_response(201, "Created", "File uploaded successfully");
    }

    //no filename, create a random filename a dump body inside
    else
    {
        //if no upload permission
        if (!location.upload_enable)
            return generate_error_response(403, "Forbidden", "You do not have permissions to upload files", location);

        std::string file_path = path + "/" + generate_upload_filename();

        int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            return generate_error_response(500, "Internel Server Error", "File exists but read failed", location);
        write(fd, (body).c_str(), (body).size());
        close(fd);

        return generate_success_response(201, "Created", "File uploaded successfully");
    }
    return generate_success_response(200, "OK", "Data proccessed successfully");
}

HttpResponse    Server::handle_file_response(const std::string &path, LocationBloc &location, const std::string &body, int flag)
{
    //if file exists already check for W persmission
    if (flag == 2)
    {
        if (access(path.c_str(), W_OK) < 0)
            return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource", location);
    }

    //no upload permission
    if (!location.upload_enable)
        return generate_error_response(403, "Forbidden", "You do not have permissions to upload files", location);

    //create new file or overwite
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        return generate_error_response(500, "Internal Server Error", "Failed to create new file.", location);
    }

    //write body content if any
    if (!body.empty())
        write(fd, body.c_str(), body.size());

    close(fd);

    //if file was already created return 200
    if (flag == 2)
        return generate_success_response(200, "OK", "File modified successfully");
    //if a file was created return 201
    return generate_success_response(201, "Created", "File created successfully");
}

HttpResponse    Server::handle_generic_type(const std::string &body, const std::string &path, LocationBloc &location)
{
    //if no upload permission
    if (!location.upload_enable)
        return generate_error_response(403, "Forbidden", "You do not have permissions to upload files", location);

    std::string file_path = path + "/" + generate_upload_filename();

    int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return generate_error_response(500, "Internel Server Error", "Read failed", location);
    write(fd, (body).c_str(), (body).size());
    close(fd);

    return generate_success_response(201, "Created", "File uploaded successfully");
}

HttpResponse    Server::generate_post_response(HttpRequest &req, LocationBloc &location)
{
    HttpResponse res;

    std::string target = req.getTarget();

    std::cout << "Post method called for target : " << target << std::endl;

    std::cout << "Location matched to : " << location.path << std::endl;
    location.print();

    //check if POST is an accepted method in the location
    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "POST") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve POST method", location);

    //check body size
    if(req.getBody().size() > location.client_max_body_size)
        return generate_error_response(413, "Payload Too Large", "Body too large", location);

    //construct the path of the ressource based on the root and upload path
    std::string path = get_POST_ressource_path(target, location);
    std::cout << "Contructed ressource path: " << path << std::endl;

    //Check existence of the target
    struct stat st;
    if (stat(path.c_str(), &st) < 0)
    {
        //if is exists but incorrect permissions
        if (errno == EACCES)
            return generate_error_response(403, "Forbidden", "You do not have permission to access this resource", location);

        //if not exist make it a file if parent directory exists and permits POST
        if (errno == ENOENT)
        {
            //VERIFY WHEN YOU DO UPLOAD/QWERTY/
            //you should not create a file if qwerty/ (check upstream)
            return handle_file_response(path, location, req.getBody(), 1);
        }
        else
        {
            return generate_error_response(404, "Not Found", "The requested resource does not exist", location);
        }
    }

    //if target exists but is a directory
    if (S_ISDIR(st.st_mode))
    {
        //if upload not enabled
        if (location.upload_enable == false)
            return generate_error_response(403, "Forbidden", "Requested location doesn't allow uploads", location);

        //check directory write and x permission
        if (access(path.c_str(), W_OK | X_OK) < 0)
        {
            return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource", location);
        }

        //There are three types of POST methods (multipart, url encoded, json) that need parsing

        std::map<std::string, std::string>::const_iterator it = req.getHeaders().find("Content-Type");

        //if the client didn't specify a content type we can't parse it. give it a default one 
        if (it == req.getHeaders().end())
        {
            return (handle_generic_type(req.getBody(), path, location));
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
            return (handle_url_encoded(req.getBody(), path, location));
        }

        //same as url encoded but with json format
        else if (content_type == "application/json")
        {
            return (handle_json(req.getBody(), path, location));
        }

        //if it is multipart form data, there are one or more files to do
        else if (content_type == "multipart/form-data")
        {
            return (handle_multipart(req.getBody(), path, it->second, location));
        }

        //if any other Content-Type we can't safely parse it so return 415
        else
        {
            return (handle_generic_type(req.getBody(), path, location));
        }
    }

    //check if it is CGI
    size_t dot = path.rfind('.');
    if (dot != std::string::npos)
    {
        //construct the file extension
        std::string ext = path.substr(dot);

        //check in location.cgi_extension
        for (size_t i = 0; i < location.cgi_extension.size(); ++i)
        {
            if (ext == location.cgi_extension[i])
            {
                //file should be executed as CGI
                return generate_cgi_response(path, req, location);
            }
        }
    }

    //if it is a file that already exists check for permission and ovewrite
    return (handle_file_response(path, location, req.getBody(), 2));
}

