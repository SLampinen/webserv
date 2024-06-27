#include "manager.hpp"
#include "server.hpp"

// returns the index of server whose pair is fd from given vector
size_t getServer(std::vector<std::pair<int, size_t>> const &server_index, int fd)
{
	size_t index = 0;
	for (; index < server_index.size(); index++)
	{
		if (server_index.at(index).first == fd)
			break;
	}
	if (index == server_index.size())
		throw std::runtime_error("Manager::handleGet couldn't match to server");
	return index;
}

// extracts file path from header with leading slash
std::string getFilePath(std::string const &header)
{
	size_t pos = header.find(' ') + 1;
	std::string filepath = header.substr(pos, header.find(' ', pos) - pos);
	if (filepath.find('?') != std::string::npos)
		filepath = filepath.substr(0, filepath.find('?'));
	return filepath;
}

// matches server and configserver from request and copies the information (setLocation) from configserver to server
Server &Manager::prepareServer(int const method, std::string file_path, std::vector<struct pollfd> fds, int i, Response &response)
{
	Server &server = serverList.at(serverIndex.at(getServer(serverIndex, fds[i].fd)).second);
	ConfigServer &c_server = configserverList.at(serverIndex.at(getServer(serverIndex, fds[i].fd)).second);
	response = c_server.resolveRequest(method, file_path);
	if (c_server.isThereLocationMatch())
		server.setLocation(c_server.getMatchedLocation());
	else
	{
		Location emptyloc("");
		server.setLocation(emptyloc);
	}
	return (server);
}

// resolves 404 and 405 if location did not match (thus server doesn't have necessary information and fails)
bool Manager::prepareFailure(Response const &response, std::vector<struct pollfd> fds, int i)
{
	if (response.getType() != 404 && response.getType() != 405 && response.getType() != 302)
		return false;
	Server &server = serverList.at(serverIndex.at(getServer(serverIndex, fds[i].fd)).second);
	std::string response_data(server.makeHeader(response.getType(), 0));
	if (response.getType() == 302) {
		std::string insert_location = "\r\nLocation: " + response.getCGIPath();
		response_data.insert(response_data.find('\n'), insert_location);
	}
	size_t sendMessage = send(fds[i].fd, response_data.c_str(), response_data.length(), 0);
	checkCommunication(sendMessage, i);
	return (true);
}

void Manager::handleGet(std::string request_data, std::vector<struct pollfd> fds, int i)
{
	Response response;
	Server &server = prepareServer(REQ_GET, getFilePath(request_data), fds, i, response);
	if (prepareFailure(response, fds, i))
		return;
	if (response.getType() == RES_CGI)
		return (handleCGI(request_data, fds, i));
	std::string response_data(server.buildHTTPResponse(response.getPath().substr(server.getRootDir().size(), std::string::npos), ""));
	size_t sendMessage = send(fds[i].fd, response_data.c_str(), response_data.length(), 0);
	if (!checkCommunication(sendMessage, i))
		return;
}

// Handle CGI, prepareServer not needed since it has been run already
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
		std::string body = "CGI not available";
		response = serverList.at(serverIndex.at(index).second).makeHeader(418, body.size());
		response.append(body);
		cgiOnGoing[i] = 0;
		size_t sendMessage = send(fds[i].fd, response.c_str(), response.length(), 0);
		if (!checkCommunication(sendMessage, i))
			return;
	}
	else
	{
		int pid;
		pid = fork();
		if (pid < 0)
		{
			std::string body = "Internal server error";
			response = serverList.at(serverIndex.at(index).second).makeHeader(500, body.size());
			response.append(body);
			cgiOnGoing[i] = 0;
			size_t sendMessage = send(fds[i].fd, response.c_str(), response.length(), 0);
			if (!checkCommunication(sendMessage, i))
				return;
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
			pids.push_back(std::make_pair(pid, i));
		}
	}
}

void Manager::maxBodySize(std::string receivedData, size_t i, Server &server, std::vector<struct pollfd> fds)
{
	std::string contentLength = receivedData.substr(receivedData.find("Content-Length:") + 16);
	contentLength = contentLength.substr(0, contentLength.find("\r\n"));
	size_t length = std::stoi(contentLength);
	size_t clientBodySize = server.getClientBodySize();
	if (length > clientBodySize)
	{
		std::string response;
		std::stringstream responseStream;
		responseStream << "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 31\r\n\r\nBody size is over max body size";
		response = responseStream.str();
		size_t sendMessage = send(fds[i].fd, response.c_str(), response.length(), 0);
		checkCommunication(sendMessage, i);
		return;
	}
}

// POST
void Manager::handlePost(std::string receivedData, std::vector<struct pollfd> fds, size_t i)
{
	Response response;
	Server &server = prepareServer(REQ_POST, getFilePath(receivedData), fds, i, response);
	if (prepareFailure(response, fds, i))
		return;
	for (size_t j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			serverList.at(serverIndex.at(j).second).log("handlePost" + receivedData);
			break;
		}
	}
	maxBodySize(receivedData, i, server, fds);

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
		// boundary starts with --
		boundary = "--" + boundary;
		//std::cerr << "Boundary = " << boundary << std::endl;

		// helps at not crashing when input is chunked
		if (end == std::string::npos)
			end = 0;

		std::string rawData = receivedData.substr(end);
		if (path.compare("upload") == 0)
		{
			handleUpload(rawData, boundary, fds, i);
		}
		else
		{
			boundaries.push_back(std::make_pair("", boundary));
			boundaryUsed.push_back(0);
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
		size_t sendMessage = send(fds[i].fd, response.c_str(), response.length(), 0);
		if (!checkCommunication(sendMessage, i))
			return;
	}
}

// DELETE
void Manager::handleDelete(std::string receivedData, std::vector<struct pollfd> fds, int i)
{
	Response cresponse;
	Server &server = prepareServer(REQ_DEL, getFilePath(receivedData), fds, i, cresponse);
	(void)server;
	if (prepareFailure(cresponse, fds, i))
		return;
	for (size_t j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			serverList.at(serverIndex.at(j).second).log("handleDelete" + receivedData);
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
	size_t sendMessage = send(fds[i].fd, response.c_str(), response.length(), 0);
	if (!checkCommunication(sendMessage, i))
		return;
}

// OTHER
void Manager::handleOther(std::string receivedData, std::vector<struct pollfd> fds, int i)
{
	for (size_t j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			serverList.at(serverIndex.at(j).second).log("handleOther " + receivedData);
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
	size_t sendMessage = send(fds[i].fd, response.c_str(), response.length(), 0);
	if (!checkCommunication(sendMessage, i))
		return;
}

// Handle file upload
void Manager::handleUpload(std::string receivedData, std::string boundary, std::vector<struct pollfd> fds, int i)
{
	size_t index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
			break;
	}

	// Extract the filename from the received data
	size_t start = receivedData.find("filename=");
	size_t end = 0;
	if (start != std::string::npos)
	{
		start += 10;
		end = receivedData.find("\"", start);
	}
	std::string name;
	if (end != std::string::npos && end != 0)
		name = receivedData.substr(start, end - start);
	bool hasAName = true;
	if (name.empty())
	{
		hasAName = false;
	}

	std::ofstream theFile;
	std::string root = serverList.at(serverIndex.at(index).second).getRootDir().append("files/");
	name = root.append(name);
	for (size_t tbd = 0; tbd < fdsFileNames.size(); tbd++)
	{
		if (fdsFileNames.at(tbd).first == i)
		{
			fdsFileNames.erase(fdsFileNames.begin() + tbd);
			break;
		}
	}

	fdsFileNames.push_back(std::make_pair(i, name));

	boundaries.push_back(std::make_pair(name, boundary));
	boundaryUsed.push_back(0);

	theFile.open(name, std::ofstream::trunc);
	if (!theFile.is_open())
	{
		// If file cannot be opened, send an error response
		if (hasAName)
		{
			std::string response;
			std::stringstream responseStream;
			responseStream << "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 38\r\n\r\nUploading failed due to unknown reason";
			response = responseStream.str();
			size_t sendMessage = send(fds[i].fd, response.c_str(), response.length(), 0);
			checkCommunication(sendMessage, i);
		}
		if (receivedData.find(boundary + "--") != std::string::npos)
		{
			std::string response;
			std::stringstream responseStream;
			responseStream << "HTTP/1.1 400 Bad Request\r\nContent-Length: 45\r\n\r\nTrying to upload file without choosing a file";
			response = responseStream.str();
			size_t sendMessage = send(fds[i].fd, response.c_str(), response.length(), 0);
			checkCommunication(sendMessage, i);
		}
		
		return;
	}

	// Find the start and end of the file content in the received data
	start = receivedData.find("Content-Type");
	if (start != std::string::npos)
		start = receivedData.find("\n", start);
	else
		start = receivedData.find("\n");
	start = receivedData.find_first_not_of("\r\n", start);
	end = receivedData.find(boundary, start);
	bool lastBoundary = false;
	if (end != std::string::npos)
	{
		if (receivedData.at(end + boundary.size()) == '-' && receivedData.at(end + boundary.size() + 1) == '-')
		{
			lastBoundary = true;
		}

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
	if (lastBoundary)
	{
		boundaries.pop_back();
		boundaryUsed.pop_back();
	}

	// Send a success response
	std::string responsestr;
	std::stringstream responseStream;
	responseStream << "HTTP/1.1 200 OK\r\nContent-Length: 26\r\n\r\nFile uploaded successfully";
	responsestr = responseStream.str();
	size_t sendMessage = send(fds[i].fd, responsestr.c_str(), responsestr.length(), 0);
	if (!checkCommunication(sendMessage, i))
		return;
}

void Manager::handleContinue(std::string receivedData, int fdsIndex)
{
	size_t currentServer;
	for (currentServer = 0; currentServer < serverIndex.size(); currentServer++)
	{
		if (serverIndex.at(currentServer).first == fds[fdsIndex].fd)
		{
			serverList.at(serverIndex.at(currentServer).second).log("handleContinue" + receivedData);
			break;
		}
	}
	size_t indexB;
	size_t boundaryBegin;
	std::string name;
	size_t dataBegin = 0;
	size_t dataEnd = 0;
	bool ended = false;
	for (indexB = 0; indexB < boundaries.size(); indexB++)
	{
		if ((boundaryBegin = receivedData.find(boundaries.at(indexB).second)) != std::string::npos)
		{
			if (boundaryBegin == 0)
			{
				// curl
				size_t filenameStart = receivedData.find("filename=") + 10;
				size_t filenameEnd = receivedData.find("\"", filenameStart);
				name = receivedData.substr(filenameStart, filenameEnd - filenameStart);
				name = serverList.at(serverIndex.at(currentServer).second).getRootDir().append("files/") + name;
				dataBegin = receivedData.find("\r\n\r\n") + 4;
				dataEnd = receivedData.find(boundaries.at(indexB).second, dataBegin + 1);
				if (dataEnd == std::string::npos)
				{
					dataEnd = receivedData.size() - 1;
				}
				else
				{
					if (receivedData.at(dataEnd + boundaries.at(indexB).second.size()) == '-' && receivedData.at(dataEnd + boundaries.at(indexB).second.size() + 1) == '-')
					{
						ended = true;
					}
				}
				receivedData = receivedData.substr(dataBegin, dataEnd - dataBegin);
			}
			else
			{
				// firefox
				dataEnd = receivedData.find(boundaries.at(indexB).second, dataBegin + 1);
				if (dataEnd != std::string::npos)
				{
					if (receivedData.at(dataEnd + boundaries.at(indexB).second.size()) == '-' && receivedData.at(dataEnd + boundaries.at(indexB).second.size() + 1) == '-')
					{
						ended = true;
					}
				}
				receivedData = receivedData.substr(0, receivedData.find(boundaries.at(indexB).second));
			}
			break;
		}
	}
	if (name.empty())
	{
		if (indexB < boundaries.size())
		{
			name = boundaries.at(indexB).first;
		}
		else
		{
			int namesIndex = 0;
			for (size_t namesIndex = 0; namesIndex < fdsFileNames.size(); namesIndex++)
			{
				if (fdsIndex == fdsFileNames.at(namesIndex).first)
				{
					break;
				}
			}
			name = fdsFileNames.at(namesIndex).second;
		}
	}
	std::ofstream theFile;
	if (indexB < boundaryUsed.size())
	{
		if (boundaryUsed.at(indexB) == 1)
			theFile.open(name, std::ofstream::app);
		else
			theFile.open(name, std::ofstream::trunc);
		boundaryUsed.at(indexB) = 1;
	}
	else
	{
		boundaryUsed.at(indexB - 1) = 1;
		theFile.open(name, std::ofstream::app);
	}
	if (theFile.is_open() == 0)
	{
		std::string body = "Can't open file";
		std::string response = serverList.at(serverIndex.at(currentServer).second).makeHeader(500, body.size());
		response.append(body);
		size_t sendMessage = send(fds[fdsIndex].fd, response.c_str(), response.length(), 0);
		if (!checkCommunication(sendMessage, currentServer))
			return;
	}
	theFile << receivedData;
	theFile.close();
	// remove boundary from list if the last boundary has been found
	if (ended)
	{
		boundaries.erase(boundaries.begin() + indexB);
		boundaryUsed.erase(boundaryUsed.begin() + indexB);
		std::string body = "File uploaded successfully";
		std::string response = serverList.at(serverIndex.at(currentServer).second).makeHeader(200, body.size());
		response.append(body);
		size_t sendMessage = send(fds[fdsIndex].fd, response.c_str(), response.length(), 0);
		if (!checkCommunication(sendMessage, currentServer))
			return;
	}
}