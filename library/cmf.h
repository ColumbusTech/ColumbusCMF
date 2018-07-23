#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zstd.h>

/*
* Helper structs for storing vertex data
*/
typedef struct
{
	float X;
	float Y;
} CMF_vec2;

typedef struct
{
	float X;
	float Y;
	float Z;
} CMF_vec3;

typedef struct
{
	CMF_vec3 Position;
	CMF_vec2 UV;
	CMF_vec3 Normal;
} CMF_Vertex;

static void ProcessVertices(uint32_t Count, float* VBuffer, float* UBuffer, float* NBuffer, CMF_Vertex* Out)
{
	uint64_t VCounter = 0x00;
	uint64_t UCounter = 0x00;
	uint64_t NCounter = 0x00;

	for (uint32_t i = 0; i < Count; i++)
	{
		Out[i].Position.X = VBuffer[VCounter++];
		Out[i].Position.Y = VBuffer[VCounter++];
		Out[i].Position.Z = VBuffer[VCounter++];

		Out[i].UV.X = UBuffer[UCounter++];
		Out[i].UV.Y = UBuffer[UCounter++];

		Out[i].Normal.X = NBuffer[NCounter++];
		Out[i].Normal.Y = NBuffer[NCounter++];
		Out[i].Normal.Z = NBuffer[NCounter++];
	}
}

CMF_Vertex* CMF_Load(const char* FileName, uint32_t* OutCount)
{
	FILE* File = fopen(FileName, "rb");
	if (File == NULL) return NULL;

	uint8_t Magic[21];
	uint32_t Count = 0x00;
	uint8_t Compression = 0x00;

	//Get file size
	uint64_t FileSize = 0x00;
	fseek(File, 0, SEEK_END);
	FileSize = ftell(File);
	fseek(File, 0, SEEK_SET);

	//Check file signature
	fread(Magic, 1, 21, File);
	if (memcmp(Magic, "COLUMBUS MODEL FORMAT", 21) != 0) return NULL;

	//Read header
	fread(&Count, 1, sizeof(uint32_t), File);
	fread(&Compression, 1, sizeof(uint8_t), File);

	float* VBuffer = NULL;
	float* UBuffer = NULL;
	float* NBuffer = NULL;
	CMF_Vertex* Vertices = NULL;

	VBuffer = (float*)malloc(Count * 3 * 3 * sizeof(float));
	UBuffer = (float*)malloc(Count * 3 * 2 * sizeof(float));
	NBuffer = (float*)malloc(Count * 3 * 3 * sizeof(float));
	Vertices = (CMF_Vertex*)malloc(Count * sizeof(CMF_Vertex));

	switch (Compression)
	{
		case 0x00: //No compression
		{
			fread(VBuffer, Count * 3 * 3 * sizeof(float), 1, File);
			fread(UBuffer, Count * 3 * 2 * sizeof(float), 1, File);
			fread(NBuffer, Count * 3 * 3 * sizeof(float), 1, File);

			break;
		}

		case 0xFF: //ZSTD compression
		{
			uint8_t* FileBuf = (uint8_t*)malloc(FileSize - 26);
			fread(FileBuf, FileSize - 26, 1, File);

			uint64_t DecompressedSize = ZSTD_getDecompressedSize(FileBuf, FileSize - 26);
			uint8_t* Decompressed = (uint8_t*)malloc(DecompressedSize);
			ZSTD_decompress(Decompressed, DecompressedSize, FileBuf, FileSize - 26);
			free(FileBuf);

			memcpy(VBuffer, Decompressed, Count * 3 * 3 * sizeof(float)); Decompressed += Count * 3 * 3 * sizeof(float);
			memcpy(UBuffer, Decompressed, Count * 3 * 2 * sizeof(float)); Decompressed += Count * 3 * 2 * sizeof(float);
			memcpy(NBuffer, Decompressed, Count * 3 * 3 * sizeof(float)); Decompressed += Count * 3 * 3 * sizeof(float);

			Decompressed -= DecompressedSize;
			free(Decompressed);

			break;
		}
	}

	*OutCount = Count;
	ProcessVertices(Count, VBuffer, UBuffer, NBuffer, Vertices);

	free(VBuffer);
	free(UBuffer);
	free(NBuffer);

	fclose(File);
	return Vertices;
}




























