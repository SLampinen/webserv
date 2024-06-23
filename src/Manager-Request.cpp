#include "manager.hpp"

// GET
void Manager::handleGet(std::string receivedData, std::vector<struct pollfd> fds, int i)
{
	std::string response;
	size_t start = receivedData.find('/');
	size_t end = receivedData.find(' ', start);
	std::string file = receivedData.substr(start + 1, end - start - 1);
	std::string fileExt = ".html";
	std::string fileName;

	// Extract file name and extension
	if (file.find('.') != std::string::npos)
	{
		fileExt = file.substr(file.find('.'));
		fileName = file.substr(0, file.find('.'));
	}
	else
	{
		fileName = file;
	}

	std::cout << "File ext = " << fileExt << std::endl;

	// Handle query parameters
	if (fileExt.find("?") != std::string::npos)
	{
		end = fileExt.find("?");
		fileExt = fileExt.substr(0, end);
	}

	size_t index;

	// Find the server index
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
		{
			break;
		}
	}

	std::cout << "HERE!" << std::endl;
	std::cout << "File ext = " << fileExt << " and cgi ext = " << serverList.at(serverIndex.at(index).second).getCGIExt() << std::endl;
	std::cout << serverList.size() << " and " << serverIndex.size() << " and " << index << std::endl;

	// Check if the file extension matches the CGI extension
	if (fileExt.compare(serverList.at(serverIndex.at(index).second).getCGIExt()) == 0)
	{
		std::cout << "starting CGI" << std::endl;
		serverList.at(serverIndex.at(index).second).log(receivedData);
		handleCGI(receivedData, fds, i);
	}
	else
	{
		// for debugging
		std::cout << "here, making up a response, i = " << i << std::endl;
		serverList.at(serverIndex.at(index).second).log(receivedData);
		response = serverList.at(serverIndex.at(index).second).buildHTTPResponse(fileName, fileExt);

		if (serverList.at(serverIndex.at(index).second).getClientBodySize() != 0 && serverList.at(serverIndex.at(index).second).getClientBodySize() < response.length())
		{
			std::cout << "response too large" << std::endl;
			std::string body = "ERROR 413 Request Entity Too Large";
			response = serverList.at(serverIndex.at(index).second).makeHeader(413, body.size());
			response.append(body);
		}

		send(fds[i].fd, response.c_str(), response.length(), 0);
	}
}

// ! added by rleskine
size_t getServer(std::vector<std::pair<int, size_t> > const &server_index, int fd) {
	size_t index = 0;
	for (; index < server_index.size(); index++) {
		if (server_index.at(index).first == fd) break;
	}
	if (index == server_index.size())
		throw std::runtime_error("Manager::handleGet couldn't match to server");
	return index;
}

// ! added by rleskine
std::string getFilePath(std::string const &header) {
	size_t pos = header.find(' ') + 2; // ! + 1 if leading slash
	std::string filepath = header.substr(pos, header.find(' ', pos) - pos);
	if (filepath.find('?') != std::string::npos)
		filepath = filepath.substr(0, filepath.find('?') + 1);
	return filepath;
}

// ! added by rleskine, works with chunk-branch
void Manager::handleGet2(std::string receivedData, std::vector<struct pollfd> fds, int i) {
	//size_t pos = receivedData.find(' ') + 2; // ! + 1 if leading slash
	//std::string filepath = receivedData.substr(pos, receivedData.find(' ', pos) - pos);
	//if (filepath.find('?') != std::string::npos)
	//	filepath = filepath.substr(0, filepath.find('?') + 1);
	Server &server = serverList.at(serverIndex.at(getServer(serverIndex, fds[i].fd)).second);
	server.log(receivedData);
	if (getFilePath(receivedData).find(server.getCGIExt()) != std::string::npos)
		return (handleCGI(receivedData, fds, i));
	std::string response = server.buildHTTPResponse(getFilePath(receivedData), "");
	if (server.getClientBodySize() && server.getClientBodySize() < response.size()) {
		std::string const body("ERROR 413 Request Entity Too Large");
		response = server.makeHeader(413, body.size());
		response.append(body);
	}
	send(fds[i].fd, response.c_str(), response.length(), 0);
}


// ! added by rleskine, version that works when merged with parsing
// void Manager::handleGet(std::string request_data, std::vector<struct pollfd> fds, int i) {
// 	size_t pos = request_data.find(' ') + 1;
// 	std::string filepath = request_data.substr(pos, request_data.find(' ', pos) - pos);
// 	if (filepath.find('?') != std::string::npos)
// 		filepath = filepath.substr(0, filepath.find('?') + 1);
// 	ConfigServerServer &server = serverList.at(serverIndex.at(getServer(serverIndex, fds[i].fd)).second);
// 	server.log(request_data);
// 	Response response = server.resolveRequest(REQ_GET, filepath);
// 	if (response.getType() == RES_CGI) // ! CGI
// 		return (handleCGI());
// 	else if (response.getType() == RES_DIR) // ! Directory
// 		return (handleDir());
// 	else if (response.getType() == RES_FILE) // ! File
// 		return (handleFile());
// 	throw std::runtime_error("Server returned invalid request type: " + std::to_string(response.getType()));
// }


// Handle CGI
void Manager::handleCGI(std::string receivedData, std::vector<struct pollfd> fds, int i)
{
	std::string response;
	cgiOnGoing[i] = 1;
	size_t index;

	// Find the server index
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
		{
			break;
		}
	}

	if (serverList.at(serverIndex.at(index).second).getCGIExt().empty() || serverList.at(serverIndex.at(index).second).getCGIPath().empty())
	{
		std::cout << "ERROR, CGI not set" << std::endl;
		std::string body = "CGI not available";
		response = serverList.at(serverIndex.at(index).second).makeHeader(418, body.size());
		response.append(body);
		cgiOnGoing[i] = 0;
		send(fds[i].fd, response.c_str(), response.length(), 0);
	}
	else
	{
		std::cout << "my fd = " << fds[i].fd << std::endl;
		std::cout << "Doing cgi" << std::endl;
		int pid;
		std::cout << "FORKING" << std::endl;
		pid = fork();
		if (pid < 0)
		{
			std::cout << "ERROR piderror" << std::endl;
			std::string body = "Internal server error";
			response = serverList.at(serverIndex.at(index).second).makeHeader(500, body.size());
			response.append(body);
			cgiOnGoing[i] = 0;
			send(fds[i].fd, response.c_str(), response.length(), 0);
		}
		else if (pid == 0)
		{
			std::string path = serverList.at(serverIndex.at(index).second).getCGIPath();
			std::string cmd = serverList.at(serverIndex.at(index).second).getRootDir();
			receivedData = receivedData.substr(receivedData.find("/") + 1);
			cmd.append(receivedData);
			if (cmd.find("?") != std::string::npos)
			{
				int end = cmd.find("?");
				cmd.erase(end);
			}
			std::cout << "path = " << path << std::endl;
			std::cout << "cmd = " << cmd << std::endl;
			std::string fName = serverList.at(serverIndex.at(index).second).getRootDir();
			fName.append("temp");
			fName.append(std::to_string(i));
			int fd = open(fName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0777);
			dup2(fd, 1);
			char *cmdArr[] = {const_cast<char *>(path.data()), const_cast<char *>(cmd.data()), NULL};
			execvp(cmdArr[0], cmdArr);
			exit(0);
		}
		else
		{
			std::cout << "pid is " << pid << std::endl;
			pids.push_back(std::make_pair(pid, i));
		}
	}
}

// POST
void Manager::handlePost(std::string receivedData, std::vector<struct pollfd> fds, int i)
{
	for (size_t j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			serverList.at(serverIndex.at(j).second).log(receivedData);
			break;
		}
	}

	if (receivedData.find("boundary") != std::string::npos)
	{
		std::string path;
		size_t start = receivedData.find("/") + 1;
		size_t end = receivedData.find(" ", start);
		path = receivedData.substr(start, end - start);
		start = receivedData.find("boundary=") + 9;
		end = receivedData.find_first_of("\r\n ", start);
		std::string boundary = receivedData.substr(start, end - start);
		end = receivedData.find(boundary, end);
		std::cerr << "Boundary = " << boundary << std::endl;

		// helps at not crashing when input is chunked
		if (end == std::string::npos)
			end = 0;

		std::string rawData = receivedData.substr(end);
		if (path.compare("upload") == 0)
		{
			handleUpload(rawData, boundary, fds, i);
		}
	}
	else
	{
		std::string response;
		std::string buffer;
		std::ifstream file("www/result.html");
		std::getline(file, buffer, '\0');
		std::stringstream headerStream;
		headerStream << "HTTP/1.1 200 OK\r\n"
					 << "Content-Length: " << buffer.size() << "\r\n"
					 << "\r\n";
		std::string header = headerStream.str();
		response.append(header);
		response.append(buffer);
		send(fds[i].fd, response.c_str(), response.length(), 0);
	}
}

// DELETE
void Manager::handleDelete(std::string receivedData, std::vector<struct pollfd> fds, int i)
{
	for (size_t j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			serverList.at(serverIndex.at(j).second).log(receivedData);
			break;
		}
	}
	std::string response;
	size_t index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
			break;
	}
	size_t start = receivedData.find("/");
	start = receivedData.find("/", start + 1);
	size_t end = receivedData.find(" ", start);
	std::string path = receivedData.substr(start, end - start);
	std::string rootedPath = serverList.at(serverIndex.at(index).second).getRootDir();
	rootedPath.append("files");
	rootedPath.append(path);
	std::ifstream file(rootedPath);
	std::string body;
	std::cout << "path name = " << rootedPath << std::endl;
	if (file.good())
	{
		body = "OK";
		unlink(rootedPath.c_str());
		response = serverList.at(serverIndex.at(index).second).makeHeader(200, body.size());
	}
	else
	{
		body = "Trying to delete what doesn't exist";
		response = serverList.at(serverIndex.at(index).second).makeHeader(200, body.size());
	}
	response.append(body);
	send(fds[i].fd, response.c_str(), response.length(), 0);
}

// OTHER
void Manager::handleOther(std::string receivedData, std::vector<struct pollfd> fds, int i)
{
	for (size_t j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			// std::cout << receivedData << std::endl;
			serverList.at(serverIndex.at(j).second).log(receivedData);
			break;
		}
	}

	std::string response;
	size_t index;

	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
			break;
	}

	std::string body = "Method Not Implemented";
	response = serverList.at(serverIndex.at(index).second).makeHeader(501, body.size());
	response.append(body);
	send(fds[i].fd, response.c_str(), response.length(), 0);
}


// Handle file upload
void Manager::handleUpload(std::string receivedData, std::string boundary, std::vector<struct pollfd> fds, int i)
{
	std::cout << "UPLOADING" << std::endl;
	std::cout << "i = " << i << std::endl;
	size_t index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
			break;
	}

	// Extract the filename from the received data
	size_t start = receivedData.find("filename=") + 10;
	size_t end = receivedData.find("\"", start);
	std::string name = receivedData.substr(start, end - start);

	std::cout << name << std::endl;

	std::ofstream theFile;
	std::string root = serverList.at(serverIndex.at(index).second).getRootDir().append("files/");
	name = root.append(name);
	for (size_t tbd = 0; tbd < fdsFileNames.size(); tbd++)
	{
		if (fdsFileNames.at(tbd).first == i)
		{
			fdsFileNames.erase(fdsFileNames.begin() + i);
			break;
		}
	}
	
	fdsFileNames.push_back(std::make_pair(i, name));
	std::cout << "name = " << name << std::endl;

	boundaries.push_back(std::make_pair(name, boundary));

	theFile.open(name);
	if (!theFile.is_open())
	{
		// If file cannot be opened, send an error response
		std::cout << "Is not open" << std::endl;
		std::string response;
		std::stringstream responseStream;
		responseStream << "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 38\r\n\r\nUploading failed due to unknown reason";
		response = responseStream.str();
		send(fds[i].fd, response.c_str(), response.length(), 0);
		return;
	}

	// Find the start and end of the file content in the received data
	start = receivedData.find("Content-Type");
    std::cout << "start = " << start << std::endl;
	if (start != std::string::npos)
		start = receivedData.find("\n", start);
	else
		start = receivedData.find("\n");
    std::cout << "start = " << start << std::endl;
	start = receivedData.find_first_not_of("\r\n", start);
	end = receivedData.find(boundary, start);
    // end = receivedData.find_last_of("\r\n", end);
	if (end != std::string::npos)
    {
        std::cout << "Boundary found, this is the end of firefox content" << std::endl;
        end = receivedData.find_last_of("\r\n", end) - 1;
        std::string fileContent = receivedData.substr(start, end - start);
            if (end > start)
        theFile << fileContent;
    }
    else
    {
        std::string fileContent = receivedData.substr(start);
        theFile << fileContent;
    }
	theFile.close();

	// Send a success response
	std::string response;
	std::stringstream responseStream;
	responseStream << "HTTP/1.1 200 OK\r\nContent-Length: 26\r\n\r\nFile uploaded successfully";
	response = responseStream.str();
	send(fds[i].fd, response.c_str(), response.length(), 0);
}

// Handle chunked file upload
void Manager::handleChunk(std::string receivedData, std::vector<struct pollfd> fds, int fdsIndex, int boundariesIndex)
{
	size_t index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[fdsIndex].fd)
			break;
	}
	std::ofstream theFile;
	std::string name = boundaries.at(boundariesIndex).first;
	std::cout << "name = " << name << std::endl;
	theFile.open(name, std::ofstream::app);
	if (!theFile.is_open())
	{
		// If file cannot be opened, send an error response
		std::cout << "Is not open" << std::endl;
		std::string response;
		std::stringstream responseStream;
		responseStream << "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 38\r\n\r\nUploading failed due to unknown reason";
		response = responseStream.str();
		send(fds[fdsIndex].fd, response.c_str(), response.length(), 0);
		return;
	}

	// Append the received data to the file
	if (receivedData.find(boundaries.at(boundariesIndex).second) != std::string::npos)
	{
		std::cout << "Boundary found, and is :" << std::endl;
		std::cout << boundaries.at(boundariesIndex).second << std::endl;
		int end = receivedData.find(boundaries.at(boundariesIndex).second) - 1;
		int newEnd = receivedData.find_last_not_of("\r\n-", end - 1);
		std::cout << end << " and " << newEnd << std::endl;
		std::cout << "The char = " << receivedData.at(newEnd) << std::endl;
		std::string usefulData = receivedData.substr(0, end);
		// std::cerr << usefulData << std::endl;
		theFile << usefulData;
	}
	else
	{
		std::cout << "Boundary not found" << std::endl;
		theFile << receivedData;
	}
	theFile.close();
}

void Manager::handleContinue(std::string receivedData, int fdsIndex)
{
	for (size_t j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[fdsIndex].fd)
		{
			// std::cout << receivedData << std::endl;
			serverList.at(serverIndex.at(j).second).log(receivedData);
			break;
		}
	}
	std::cout << "CONTINUING" << std::endl;
	size_t indexB;
	std::cout << boundaries.at(0).second << std::endl;
	for (indexB = 0; indexB < boundaries.size(); indexB++)
	{
		if (receivedData.find(boundaries.at(indexB).second) != std::string::npos)
		{
			receivedData = receivedData.substr(0, receivedData.find(boundaries.at(indexB).second));
			receivedData = receivedData.substr(0, receivedData.find_last_of("\r\n") - 1);
			break;
		}
	}
	// std::cerr << receivedData << std::endl;
	std::cout << indexB << " and " << boundaries.size() << std::endl;
	if (indexB < boundaries.size())
	{
		std::cout << "is smaller" << std::endl;
		std::string name = boundaries.at(indexB).first;
		std::cout << "name of file = " << name << std::endl;
		std::ofstream theFile;
		theFile.open(name, std::ofstream::app);
		theFile << receivedData;
		theFile.close();
	}
	else
	{
		std::cout << "is same" << std::endl;
		int namesIndex = 0;
		for (size_t namesIndex = 0; namesIndex < fdsFileNames.size(); namesIndex++)
		{
			if (fdsIndex == fdsFileNames.at(namesIndex).first)
			{
				break;
			}
		}
		std::cout << "name of file = " << fdsFileNames.at(namesIndex).second << std::endl;
		std::ofstream theFile;
		theFile.open(fdsFileNames.at(namesIndex).second, std::ofstream::app);
		theFile << receivedData;
		theFile.close();
	}
}

void Manager::handleTimeout(int fdsIndex)
{
	char buffer[1024];
	int bytesReceived = recv(fds[fdsIndex].fd, buffer, sizeof(buffer), 0);
	std::string receivedData(buffer, bytesReceived);
	bytesReceived = recv(fds[fdsIndex].fd, buffer, sizeof(buffer), 0);
	while (bytesReceived > 0)
	{
		std::string rest(buffer, bytesReceived);
		receivedData.append(rest);
		bytesReceived = recv(fds[fdsIndex].fd, buffer, sizeof(buffer), 0);
	}
	if (receivedData.find(boundaries.at(fdsIndex).second) != std::string::npos)
	{
		receivedData = receivedData.substr(0, receivedData.find(boundaries.at(fdsIndex).second));
	}
	std::string name = boundaries.at(fdsIndex).first;
	std::ofstream theFile;
	theFile.open(name, std::ofstream::app);
	theFile << receivedData;
	theFile.close();
	size_t index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[fdsIndex].fd)
		{
			break;
		}
	}
	std::string body = "File uploaded successfully";
	std::string response = serverList.at(serverIndex.at(index).second).makeHeader(200, body.size());
	send(fds[fdsIndex].fd, response.c_str(), response.length(), 0);
	// clientStates[fds[fdsIndex].fd].transferInProgress = false;
}