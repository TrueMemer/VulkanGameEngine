#include "PCH.hpp"
#include "File.hpp"

bool File::create(std::string && pPath, Mode pFileMode)
{
	if (pPath.length() > 127)
		return false;
	meta.path = pPath;
	meta.fileMode = pFileMode;

	return create(meta.fileMode);
}

bool File::create(Mode pFileMode)
{
	if (meta.path.length() == 0)
		return false;

	meta.fileMode = pFileMode;

	file.open(meta.path.c_str(), (std::_Ios_Openmode)meta.fileMode);
}

bool File::open(std::string && pPath, Mode pFileMode)
{
	return open(pPath, pFileMode);
}

bool File::open(std::string & pPath, Mode pFileMode)
{
	if (pPath.length() > 127)
		return false;
	meta.path = pPath;
	meta.fileMode = pFileMode;

	return open(meta.fileMode);
}

bool File::open(Mode pFileMode)
{
	if (meta.path.length() == 0)
		return false;

	meta.fileMode = pFileMode;

	file.open(meta.path.c_str(), (std::ios_base::openmode)meta.fileMode);
	if (file.good() && file.is_open() && !file.bad())
	{
		file.seekg(0, std::ios_base::end);
		meta.size = (size_t)file.tellg();
		file.seekg(0);
		return true;
	}
	else
		return false;
}

void File::write(void * data, U32 size)
{
	file.write((char*)data, size);
}

void File::writeStr(std::string & string, bool includeTerminator, bool writeCapacity)
{
	if (!writeCapacity)
		file.write(string.c_str(), string.length() + (includeTerminator ? 1 : 0));
	else
		file.write(string.c_str(), string.size() + (includeTerminator ? 1 : 0));
}

void File::writeStr(std::string & string, bool includeTerminator)
{
	file.write(string.c_str(), string.length() + (includeTerminator ? 1 : 0));
}

void File::read(void * data, U32 size)
{
	file.read((char*)data, size);
}

void File::read(std::string & string, U32 length)
{
	string.resize(length);
	file.read((char*)string.c_str(), length);
}

void File::readStr(std::string & string, char delim, int size)
{
	if (size == -1)
	{
		char lastChar[2];
		lastChar[1] = 0;
		file.read(lastChar, 1);
		while (*lastChar != delim && *lastChar != '\0' && !file.eof())
		{
			string.append(lastChar);
			file.read(lastChar, 1);
		}
	}
	else
	{
		string.resize(size);
		file.read(const_cast<char*>(string.c_str()), size);
	}
}

void File::readFile(void * buffer)
{
	readArray((char*)buffer, meta.size);
}

char File::peekChar()
{
	return file.peek();
}

char File::pullChar()
{
	char pull;
	file.read(&pull, 1);
	return pull;
}