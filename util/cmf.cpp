#include <cstdio>
#include <cstdint>
#include <cstring>
#include "cmf_cmf.h"

enum FileType
{
	CMF,
	Undefined
};

struct CommandLineFlags
{
	bool Help;
	bool Compress;
};

std::vector<Vertex> Vertices;

FileType GetFileType(const char* FileName)
{
	FILE* File = fopen(FileName, "rb");

	if (File == NULL)
	{
		return Undefined;
	}

	uint8_t* Magic[21];

	fread(Magic, 1, 21, File);

	if (memcmp(Magic, "COLUMBUS MODEL FORMAT", 21) == 0)
	{
		fclose(File);
		return CMF;
	}

	fclose(File);

	return Undefined;
}

bool Load(const char* FileName)
{
	FileType Type = GetFileType(FileName);

	switch (Type)
	{
	case Undefined: return false;                 break;
	case CMF: return LoadCMF(FileName, Vertices); break;
	}

	return true;
}

bool Save(const char* FileName, CommandLineFlags Flags)
{
	return SaveCMF(FileName, Vertices, Flags.Compress);
}

void PrintUsing()
{
	printf("Using\n");
	printf("cmf [input] [output] [flags]\n\n");
	printf("Flags\n");
	printf("-h --help\n");
	printf("-c --compress\n");
}

CommandLineFlags CheckFlags(int argc, char** argv)
{
	CommandLineFlags Flags;

	for (int i = 1; i < argc; i++)
	{
		if (memcmp(argv[1], "-h", 2) == 0 || memcmp(argv[1], "--help", 6) == 0)
		{
			Flags.Help = true;
		}

		if (memcmp(argv[1], "-c", 2) == 0 || memcmp(argv[1], "--compress", 10) == 0)
		{
			Flags.Compress = true;
		}
	}

	return Flags;
}

int main(int argc, char** argv)
{
	printf("Columbus Model File Util\n\n");
	CommandLineFlags Flags = CheckFlags(argc, argv);

	if (Flags.Help)
	{
		PrintUsing();
		return 0;
	}

	if (argc < 3)
	{
		PrintUsing();
		return 1;
	}

	if (!Load(argv[1]))
	{
		printf("Error: failed to load file '%s'\n", argv[1]);
		printf("Use -h or --help for help\n");
		return 1;
	}

	if (!Save(argv[2], Flags))
	{
		printf("Error: failed to save file '%s'\n", argv[2]);
		printf("Use -h or --help for help\n");
		return 1;
	}

	return 0;
}


















