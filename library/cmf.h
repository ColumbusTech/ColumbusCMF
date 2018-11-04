#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zstd.h>

/**
* @file cmf.h
* @brief File including all structs and functions of CMF C library.
*/

/*!
* Two-dimensional vector struct. Used for texture coordinates.
*/
typedef struct
{
	float X;
	float Y;
} CMF_vec2;

/*!
* Three-dimensial vector struct. Used for vertex positions and normals.
*/
typedef struct
{
	float X;
	float Y;
	float Z;
} CMF_vec3;

/*!
* Vertex struct.
*/
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

	for (uint32_t i = 0; i < Count * 3; i++)
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
/*!
* @brief Loads CMF from file.
*
* @param FileName Name of file, which would be read.
* @param OutCount Valid pointer to count of polygons.
* @return Buffer which read from file or NULL if error was occured.
*/
CMF_Vertex* CMF_Load(const char* FileName, uint32_t* OutCount)
{
	FILE* File = fopen(FileName, "rb");
	if (File == NULL) return NULL;

	uint8_t Magic[21];
	uint32_t Count = 0x00;
	uint8_t Compression = 0x00;

	//Get file size
	fseek(File, 0, SEEK_END);
	uint64_t FileSize = ftell(File);
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
	Vertices = (CMF_Vertex*)malloc(Count * 3 * sizeof(CMF_Vertex));

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

static void FillBuffers(uint32_t Count, float* VBuffer, float* UBuffer, float* NBuffer, CMF_Vertex* Vertices)
{
	uint64_t VCounter = 0x00;
	uint64_t UCounter = 0x00;
	uint64_t NCounter = 0x00;

	for (uint32_t i = 0; i < Count * 3; i++)
	{
		VBuffer[VCounter++] = Vertices[i].Position.X;
		VBuffer[VCounter++] = Vertices[i].Position.Y;
		VBuffer[VCounter++] = Vertices[i].Position.Z;

		UBuffer[UCounter++] = Vertices[i].UV.X;
		UBuffer[UCounter++] = Vertices[i].UV.Y;

		NBuffer[NCounter++] = Vertices[i].Normal.X;
		NBuffer[NCounter++] = Vertices[i].Normal.Y;
		NBuffer[NCounter++] = Vertices[i].Normal.Z;
	}
}
/*!
* @brief Saves CMF to file.
*
* @param Count Count of **polygons** in model.
* @param Compression 0x00 for no compression, 0xFF for ZSTD compression.
* @param Vertices Vertex buffer of model, size must be at least (Count * 3).
* @param FileName Name of file in which would be written vertex data.
* @return Returns 0 if saving was successful, otherwise returns 1.
*/
int CMF_Save(uint32_t Count, uint8_t Compression, CMF_Vertex* Vertices, const char* FileName)
{
	if (Compression != 0x00 && Compression != 0xFF) return 1;

	FILE* File = fopen(FileName, "wb");
	if (File == NULL) return 1;

	//Write header
	fwrite("COLUMBUS MODEL FORMAT", 21, 1, File);
	fwrite(&Count, sizeof(uint32_t), 1, File);
	fwrite(&Compression, sizeof(uint8_t), 1, File);

	float* VBuffer = (float*)malloc(Count * 3 * 3 * sizeof(float));
	float* UBuffer = (float*)malloc(Count * 3 * 2 * sizeof(float));
	float* NBuffer = (float*)malloc(Count * 3 * 3 * sizeof(float));

	FillBuffers(Count, VBuffer, UBuffer, NBuffer, Vertices);

	switch (Compression)
	{
		case 0x00: //No compression
		{
			fwrite(VBuffer, Count * 3 * 3 * sizeof(float), 1, File);
			fwrite(UBuffer, Count * 3 * 2 * sizeof(float), 1, File);
			fwrite(NBuffer, Count * 3 * 3 * sizeof(float), 1, File);

			break;
		}

		case 0xFF: //ZSTD compression
		{
			uint64_t DataCount = (Count * 3 * 3)
			                   + (Count * 3 * 2)
			                   + (Count * 3 * 3);

			uint64_t DataSize = DataCount * sizeof(float);
			uint64_t Bound = ZSTD_compressBound(DataSize);
			uint64_t Counter = 0;

			float* Data = (float*)malloc(DataCount * sizeof(float));
			uint8_t* Compressed = (uint8_t*)malloc(Bound);

			memcpy(Data, VBuffer,  Count * 3 * 3 * sizeof(float)); Data += Count * 3 * 3;
			memcpy(Data, UBuffer,  Count * 3 * 2 * sizeof(float)); Data += Count * 3 * 2;
			memcpy(Data, NBuffer,  Count * 3 * 3 * sizeof(float)); Data += Count * 3 * 3;
			Data -= DataCount;

			uint64_t CompressedSize = ZSTD_compress(Compressed, Bound, Data, DataSize, 1);
			free(Data);
			fwrite(Compressed, CompressedSize, 1, File);
			free(Compressed);

			break;
		}
	};

	free(VBuffer);
	free(UBuffer);
	free(NBuffer);

	fclose(File);
	return 0;
}
