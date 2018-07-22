#include <cstdio>
#include <cstdint>
#include <vector>
#include <zstd.h>
#include "util.h"

// CMF structure
// ____________
// | 21 bytes | Magic bytes "COLUMBUS MODEL FORMAT"
// |__________|
// ____________
// | 4 bytes  | 32-bit unsigned int value, count of polygons
// |__________|
// ____________
// |  1 byte  | 8-bit unsigned int value, compression
// |__________| 0x00 for no compression, 0xFF for ZSTD compression
//
// Compressed (or not) Data begins there
// ____________
// | V bytes  | vertex position data
// |__________|
// ____________
// | U bytes  | vertex texture coordinates data
// |__________|
// ____________
// | N bytes  | normal data
// |__________|


static void ProcessVertices(uint32_t Count, float* VBuffer, float* UBuffer, float* NBuffer, std::vector<Vertex>& Vertices);
bool LoadCMF(const char* FileName, std::vector<Vertex>& Vertices);
bool LoadCMFMemory(uint8_t* Data, uint64_t Size, std::vector<Vertex>& Vertices);
bool SaveCMF(const char* FileName, std::vector<Vertex> Vertices, bool Compressed);

void  ProcessVertices(uint32_t Count, float* VBuffer, float* UBuffer, float* NBuffer, std::vector<Vertex>& Vertices)
{
	if (Count == 0 &&
	    VBuffer == nullptr &&
	    UBuffer == nullptr &&
	    NBuffer == nullptr)
	{
		return;
	}

	Vertex Vert[3];

	uint64_t vcounter = 0;
	uint64_t ucounter = 0;
	uint64_t ncounter = 0;

	Vertices.reserve(Count);

	double ProgressOnPolygon = 1.0 / float(Count);
	double Progress = 0.0f;

	for (uint32_t i = 0; i < Count; i++)
	{
		for (uint32_t j = 0; j < 3; j++)
		{
			Vert[j].X = VBuffer[vcounter++];
			Vert[j].Y = VBuffer[vcounter++];
			Vert[j].Z = VBuffer[vcounter++];

			Vert[j].U = UBuffer[ucounter++];
			Vert[j].V = UBuffer[ucounter++];

			Vert[j].NX = NBuffer[ncounter++];
			Vert[j].NY = NBuffer[ncounter++];
			Vert[j].NZ = NBuffer[ncounter++];


			Vertices.push_back(Vert[j]);
		}

		Progress += ProgressOnPolygon;
		PrintProgress(Progress);
	}

	PrintProgress(1.0);
	printf("\n\n");
}

bool LoadCMF(const char* FileName, std::vector<Vertex>& Vertices)
{
	FILE* File = fopen(FileName, "rb");
	if (File == nullptr) return false;

	fseek(File, 0, SEEK_END);
	uint64_t Size = ftell(File);
	fseek(File, 0, SEEK_SET);

	uint8_t* FileBuf = new uint8_t[Size];
	if (fread(FileBuf, 1, Size, File) != Size) return false;
	fclose(File);

	bool Result = LoadCMFMemory(FileBuf, Size, Vertices);
	delete[] FileBuf;
	return Result;
}

bool LoadCMFMemory(uint8_t* Data, uint64_t Size, std::vector<Vertex>& Vertices)
{
	if (Data == nullptr &&
	    Size == 0)
	{
		return false;
	}

	Data += 21;
	uint32_t Count = 0;
	uint8_t Compression = 0;
	memcpy(&Count, Data, sizeof(uint32_t));      Data += sizeof(uint32_t);
	memcpy(&Compression, Data, sizeof(uint8_t)); Data += sizeof(uint8_t);

	printf("Loading CMF file...\n");

	float* VBuffer = new float[Count * 3 * 3]; //Vertex buffer
	float* UBuffer = new float[Count * 3 * 2]; //UV buffer
	float* NBuffer = new float[Count * 3 * 3]; //Normal buffer

	switch (Compression)
	{
		case 0x00: //No compression
		{
			memcpy(VBuffer, Data, Count * sizeof(float) * 3 * 3); Data += Count * sizeof(float) * 3 * 3;
			memcpy(UBuffer, Data, Count * sizeof(float) * 3 * 2); Data += Count * sizeof(float) * 3 * 2;
			memcpy(NBuffer, Data, Count * sizeof(float) * 3 * 3); Data += Count * sizeof(float) * 3 * 3;
			break;
		}

		case 0xFF: //ZSTD compression
		{
			uint64_t DecompressedSize = ZSTD_getDecompressedSize(Data, Size - 26);
			uint8_t* Decompressed = new uint8_t[DecompressedSize];
			ZSTD_decompress(Decompressed, DecompressedSize, Data, Size - 26);

			memcpy(VBuffer, Decompressed, Count * sizeof(float) * 3 * 3); Decompressed += Count * sizeof(float) * 3 * 3;
			memcpy(UBuffer, Decompressed, Count * sizeof(float) * 3 * 2); Decompressed += Count * sizeof(float) * 3 * 2;
			memcpy(NBuffer, Decompressed, Count * sizeof(float) * 3 * 3); Decompressed += Count * sizeof(float) * 3 * 3;

			Decompressed -= DecompressedSize;
			delete[] Decompressed;

			break;
		}
	}

	ProcessVertices(Count, VBuffer, UBuffer, NBuffer, Vertices);

	delete[] VBuffer;
	delete[] UBuffer;
	delete[] NBuffer;

	return true;
}

bool SaveCMF(const char* FileName, std::vector<Vertex> Vertices, bool Compressed)
{
	FILE* File = fopen(FileName, "wb");

	if (File == nullptr)
	{
		return false;
	}

	uint32_t Count = Vertices.size() / 3;
	uint8_t Compression = Compressed ? 0xFF : 0x00;

	fwrite("COLUMBUS MODEL FORMAT", 1, 21, File);
	fwrite(&Count, 1, sizeof(uint32_t), File);
	fwrite(&Compression, 1, sizeof(uint8_t), File);

	double ProgressPerVertex = 1.0 / Vertices.size() * (3.0 / 8.0);
	double ProgressPerUV     = 1.0 / Vertices.size() * (2.0 / 8.0);
	double ProgressPerNormal = 1.0 / Vertices.size() * (3.0 / 8.0);
	double Progress = 0.0;

	if (!Compressed)
	{
		printf("Saving CMF file...\n");

		for (const auto& Vert : Vertices)
		{
			fwrite(&Vert.X, sizeof(float), 1, File);
			fwrite(&Vert.Y, sizeof(float), 1, File);
			fwrite(&Vert.Z, sizeof(float), 1, File);

			Progress += ProgressPerVertex;
			PrintProgress(Progress);
		}

		for (const auto& Vert : Vertices)
		{
			fwrite(&Vert.U, sizeof(float), 1, File);
			fwrite(&Vert.V, sizeof(float), 1, File);

			Progress += ProgressPerUV;
			PrintProgress(Progress);
		}

		for (const auto& Vert : Vertices)
		{
			fwrite(&Vert.NX, sizeof(float), 1, File);
			fwrite(&Vert.NY, sizeof(float), 1, File);
			fwrite(&Vert.NZ, sizeof(float), 1, File);

			Progress += ProgressPerNormal;
			PrintProgress(Progress);
		}

		PrintProgress(1.0);
		printf("\n");
	}
	else
	{
		printf("Compressing...\n");

		uint64_t DataCount = (Vertices.size() * 3)
		                   + (Vertices.size() * 2)
		                   + (Vertices.size() * 3);

		uint64_t DataSize = DataCount * sizeof(float);
		uint64_t Bound = ZSTD_compressBound(DataSize);
		uint64_t Counter = 0;

		float* Data = new float[DataCount];
		uint8_t* Compressed = new uint8_t[Bound];

		double CompressProgressPerVertex = 0.5 / Vertices.size() * (3.0 / 8.0);
		double CompressProgressPerUV     = 0.5 / Vertices.size() * (2.0 / 8.0);
		double CompressProgressPerNormal = 0.5 / Vertices.size() * (3.0 / 8.0);
		double CompressProgress = 0.0;

		for (const auto& Vert : Vertices)
		{
			Data[Counter++] = Vert.X;
			Data[Counter++] = Vert.Y;
			Data[Counter++] = Vert.Z;

			CompressProgress += CompressProgressPerVertex;

			static int Print = 0;

			if (Print % 3 == 0)
			{
				PrintProgress(CompressProgress);
			}

			Print++;
		}

		for (const auto& Vert : Vertices)
		{
			Data[Counter++] = Vert.U;
			Data[Counter++] = Vert.V;

			CompressProgress += CompressProgressPerUV;
			
			static int Print = 0;

			if (Print % 3 == 0)
			{
				PrintProgress(CompressProgress);
			}

			Print++;
		}

		for (const auto& Vert : Vertices)
		{
			Data[Counter++] = Vert.NX;
			Data[Counter++] = Vert.NY;
			Data[Counter++] = Vert.NZ;

			CompressProgress += CompressProgressPerNormal;
			
			static int Print = 0;

			if (Print % 3 == 0)
			{
				PrintProgress(CompressProgress);
			}

			Print++;
		}

		uint64_t CompressedDataSize = ZSTD_compress(Compressed, Bound, Data, DataSize, 1);
		delete[] Data;

		PrintProgress(1.0);
		printf("\n");

		printf("Saving CMF file...\n");

		double SaveProgressPerByte = 1.0 / CompressedDataSize;
		double SaveProgress = 0.0;

		for (uint64_t i = 0; i < CompressedDataSize; i++)
		{
			static int Print = 0;

			fwrite(&Compressed[i], sizeof(uint8_t), 1, File);
			SaveProgress += SaveProgressPerByte;

			if (Print % (3 * sizeof(float)) == 0)
			{
				PrintProgress(SaveProgress);
			}

			Print++;
		}

		delete[] Compressed;

		PrintProgress(1.0);
		printf("\n");
	}

	fclose(File);

	return true;
}
















