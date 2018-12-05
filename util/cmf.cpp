#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include "cmf_cmf.h"
#include "../library/cmf.h"

enum FileType
{
	CMF,
	Undefined
};

struct CommandLineFlags
{
	bool Help = false;
	bool Compress = false;
	bool VerticesWrite = false;
	bool TexcoordsWrite = false;
	bool NormalsWrite = false;
};

std::vector<Vertex> Vertices;

FileType GetFileType(const char* FileName)
{
	FILE* File = fopen(FileName, "rb");

	if (File == NULL)
	{
		return Undefined;
	}

	uint8_t Magic[21];

	fread(Magic, 1, 21, File);

	if (memcmp(Magic, "COLUMBUS MODEL FORMAT", 21) == 0)
	{
		fclose(File);
		return CMF;
	}

	fclose(File);

	return Undefined;
}

bool FileExists(const char* FileName)
{
	bool Existance = false;
	FILE* File = fopen(FileName, "r");
	Existance = File != nullptr;
	fclose(File);
	return Existance;
}

bool FileMayBeCreated(const char* FileName)
{
	bool Oportunity = false;
	FILE* File = fopen(FileName, "w");
	Oportunity = File != nullptr;
	fclose(File);
	return Oportunity;
}

bool Load(const char* FileName)
{
	FileType Type = GetFileType(FileName);

	switch (Type)
	{
	case Undefined: return false;                 break;
	case CMF:
	{
		uint32_t Count;
		CMF_Vertex* Verts = CMF_Load(FileName, &Count);

		if (Verts == nullptr) return false;

		Vertex Vert;

		for (int i = 0; i < Count * 3; i++)
		{
			Vert.X = Verts[i].Position.X;
			Vert.Y = Verts[i].Position.Y;
			Vert.Z = Verts[i].Position.Z;

			Vert.U = Verts[i].UV.X;
			Vert.V = Verts[i].UV.Y;

			Vert.NX = Verts[i].Normal.X;
			Vert.NY = Verts[i].Normal.Y;
			Vert.NZ = Verts[i].Normal.Z;

			Vertices.push_back(Vert);
		}

		free(Verts);

		break;
	}
	}

	return true;
}

bool Save(const char* FileName, CommandLineFlags Flags)
{
	uint32_t Count = Vertices.size();
	CMF_Compression Compression = Flags.Compress ? CMF_COMPRESSION_ZSTD : CMF_COMPRESSION_NONE;

	struct CMF_Info Info;
	Info.compression = Compression;
	Info.num_vertices = Vertices.size();
	Info.num_arrays = 3;
	Info.arrays = new CMF_InfoArray[Info.num_arrays];

	float* Positions = new float[Vertices.size() * 3];
	float* Texcoords = new float[Vertices.size() * 2];
	float* Normals   = new float[Vertices.size() * 3];

	for (auto& Vert : Vertices)
	{
		*Positions++ = Vert.X;
		*Positions++ = Vert.Y;
		*Positions++ = Vert.Z;

		*Texcoords++ = Vert.U;
		*Texcoords++ = Vert.V;

		*Normals++ = Vert.NX;
		*Normals++ = Vert.NY;
		*Normals++ = Vert.NZ;
	}

	Positions -= Vertices.size() * 3;
	Texcoords -= Vertices.size() * 2;
	Normals   -= Vertices.size() * 3;

	Info.arrays[0].type = CMF_TYPE_POSITION;
	Info.arrays[1].type = CMF_TYPE_TEXCOORD;
	Info.arrays[2].type = CMF_TYPE_NORMAL;

	Info.arrays[0].format = CMF_FORMAT_FLOAT;
	Info.arrays[1].format = CMF_FORMAT_FLOAT;
	Info.arrays[2].format = CMF_FORMAT_FLOAT;

	Info.arrays[0].size = Vertices.size() * 3 * sizeof(float);
	Info.arrays[1].size = Vertices.size() * 2 * sizeof(float);
	Info.arrays[2].size = Vertices.size() * 3 * sizeof(float);

	Info.arrays[0].data = Positions;
	Info.arrays[1].data = Texcoords;
	Info.arrays[2].data = Normals;

	int Code = CMF_Save2(FileName, &Info);

	delete[] Positions;
	delete[] Texcoords;
	delete[] Normals;
	delete[] Info.arrays;

	return Code == 0;
}

void PrintUsing()
{
	printf("Using\n");
	printf("cmf [input] [output] [flags]\n\n");
	printf("Flags\n");
	printf("-h, --help         print this message\n");
	printf("-c, --compress     enable compression for output file\n");
	printf("-v, --vertices     enable writing vertices in output file\n");
	printf("-t, --texcoords    enable writing texture coordinates in output file\n");
	printf("-n, --normals      enable writing normals in output file\n");
}

CommandLineFlags CheckFlags(int argc, char** argv)
{
	CommandLineFlags Flags;

	for (int i = 1; i < argc; i++)
	{
		if (memcmp(argv[i], "-h", 2) == 0 || memcmp(argv[i], "--help", 6) == 0)
		{
			if (argc >= 3)
			{
				if (argv[2][0] != '-' && FileMayBeCreated(argv[2]))
				{
					remove(argv[2]);
				}
			}

			Flags.Help = true;
		}
		else
		if (memcmp(argv[i], "-c", 2) == 0 || memcmp(argv[i], "--compress", 10) == 0)
		{
			Flags.Compress = true;
		}
		else
		if (memcmp(argv[i], "-v", 2) == 0 || memcmp(argv[i], "--vertices", 10) == 0)
		{
			Flags.VerticesWrite = true;
		}
		else
		if (memcmp(argv[i], "-t", 2) == 0 || memcmp(argv[i], "--texcoords", 11) == 0)
		{
			Flags.TexcoordsWrite = true;
		}
		else
		if (memcmp(argv[i], "-n", 2) == 0 || memcmp(argv[i], "--normals", 9) == 0)
		{
			Flags.NormalsWrite = true;
		}
		else
		if (i == 1 && argv[1][0] != '-' && FileExists(argv[1]))
		{

		}
		else
		if (i == 2 && argv[2][0] != '-' && FileMayBeCreated(argv[2]))
		{

		}
		else
		{
			printf("Error: Invalid argument «%s»\n", argv[i]);
			printf("You may use «cmf --help» for full information\n");
			exit(1);
		}
	}

	return Flags;
}

int main(int argc, char** argv)
{
	printf("Columbus Model Format Util\n\n");
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


















