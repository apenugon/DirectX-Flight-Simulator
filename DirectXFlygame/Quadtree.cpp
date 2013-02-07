#include "DXUT.h"
#include "Quadtree.h"


//-----------------------------------------------------------
//Constructor
//-----------------------------------------------------------

Quadtree::Quadtree(const char* fileName, int Width, int Height)
{
	g_HeightMapFile = fileName;
	g_Width = Width;
	g_Height = Height;
	heightFromFile();
	createIndices();
	loadNormals();
	D3DXMatrixIdentity(&worldMatrix);
	D3DXMatrixTranslation(&worldMatrix, (float)g_Width/2.0f, 0, (float)g_Height/2.0f);
}
//-------------------------------------------------------
//Accessor Methods
//-------------------------------------------------------
int* Quadtree::getIndices()
{
	return indices;
}

VertexPositionColor* Quadtree::getVertices()
{
	return Vertices;
}

D3DXMATRIX Quadtree::getWorldMatrix()
{
	return worldMatrix;
}

//-------------------------------------------------------
//Heightmap Data loading methods
//-------------------------------------------------------

bool Quadtree::heightFromFile()
{
	FILE* pFile;
	int error;
	UINT count;

	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize;
	unsigned char* bitmapImage;
	unsigned char height;

	error = fopen_s(&pFile, g_HeightMapFile, "rb");
	if (error != 0)
	{
		return false;
	}
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, pFile);
	if (count != 1)
		return false;

	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);
	if (count != 1)
		return false;

	g_Width = bitmapInfoHeader.biWidth;
	g_Height = bitmapInfoHeader.biHeight;

	imgSize = g_Width * g_Height * 3;

	bitmapImage = new unsigned char[imgSize];
	if (!bitmapImage)
		return false;
	fseek(pFile, bitmapFileHeader.bfOffBits, SEEK_SET);

	count = fread(bitmapImage, 1, imgSize, pFile);
	if(count != imgSize)
		return false;

	error = fclose(pFile);
	if (error != 0)
		return false;
	numVertices = g_Width * g_Height * 2;
	Vertices = new VertexPositionColor[g_Width * g_Height * 2];
	if (!Vertices)
		return false;

	int index = 0;
	int k = 0;
	for (int j = 0; j < g_Width; j++)
	{
		for (int i = 0; i < g_Height; i++)
		{
			index = j + i * g_Width;
			D3DXVECTOR3 Position;
			Vertices[index].Position = Position = D3DXVECTOR3((float)i, (float)bitmapImage[k]/2.0f, (float)j);

			Vertices[index].Color = D3DXVECTOR4((float)j/(float)g_Width, 1.0f, float(i)/float(g_Height), 1.0f);

			Vertices[index].UV.x = Position.x/(sqrt(Position.x * Position.x + Position.y * Position.y + Position.z * Position.z));
			Vertices[index].UV.y = Position.y/(sqrt(Position.x * Position.x + Position.y * Position.y + Position.z * Position.z));

			k+=3;
		}
	}

	delete [] bitmapImage;
	bitmapImage = 0;

	return true;
}

void Quadtree::createIndices()
{
	int counter = 0;
	indices = new int[(g_Height - 1) * (g_Width - 1) * 4];
	numIndices = (g_Height - 1) * (g_Width - 1) * 4;
	for (int y = 0; y < g_Height - 1; y++)
		for (int x = 0; x < g_Width - 1; x++)
		{
			int lleft = x + y * g_Width;
			int tleft = x + (y + 1) * g_Width;
			int lright = (x + 1) + y * g_Width;
			int tright = (x + 1) + (y + 1) * g_Width;

			indices[counter++] = tleft;
			//indices[counter++] = lright;
			indices[counter++] = lleft;

			//indices[counter++] = tleft;
			indices[counter++] = tright;
			indices[counter++] = lright;

		}
}

void Quadtree::loadNormals()
{
	for (int i = 0; i < numVertices; i++)
	{
		Vertices[i].Normal = D3DXVECTOR3(0, 0, 0);
		Vertices[i].Tangent = D3DXVECTOR3(0, 0, 0);
		Vertices[i].Bitangent = D3DXVECTOR3(0, 0, 0);
	}

	for (int i = 0; i < numIndices/4; i++)
	{
		int point1 = indices[i * 4];
		int point2 = indices[i * 4 + 1];
		int point3 = indices[i * 4 + 2];
		int point4 = indices[i * 4 + 3];
		D3DXVECTOR3 NormalOne;
		D3DXVECTOR3 NormalTwo;
		D3DXVECTOR3 tangentOne;
		D3DXVECTOR3 tangentTwo;
		D3DXVECTOR3 bitangentOne;
		D3DXVECTOR3 bitangentTwo;
		
		D3DXVECTOR3 sideOneTriOne = Vertices[point1].Position - Vertices[point2].Position;
		D3DXVECTOR3 sideTwoTriOne = Vertices[point1].Position - Vertices[point3].Position;
		D3DXVECTOR3 sideOneTriTwo = Vertices[point4].Position - Vertices[point2].Position;
		D3DXVECTOR3 sideTwoTriTwo = Vertices[point4].Position - Vertices[point3].Position;
		D3DXVec3Cross(&NormalOne, &sideOneTriOne, &sideTwoTriOne);
		D3DXVec3Cross(&NormalTwo, &sideOneTriTwo, &sideTwoTriTwo);

		computeTangentBitangent(Vertices[point1].Position, Vertices[point2].Position, Vertices[point3].Position,
			Vertices[point1].UV, Vertices[point2].UV, Vertices[point3].UV, tangentOne, bitangentOne);
		computeTangentBitangent(Vertices[point4].Position, Vertices[point2].Position, Vertices[point3].Position,
			Vertices[point4].UV, Vertices[point2].UV, Vertices[point3].UV, tangentTwo, bitangentTwo);
		
		Vertices[point1].Normal += NormalOne;
		Vertices[point2].Normal += NormalOne;
		Vertices[point2].Normal -= NormalTwo;
		Vertices[point3].Normal += NormalOne;
		Vertices[point3].Normal -= NormalTwo;
		Vertices[point4].Normal -= NormalTwo;

		Vertices[point1].Tangent = tangentOne;
		Vertices[point1].Tangent -= Vertices[point1].Normal * D3DXVec3Dot(&Vertices[point1].Tangent, &Vertices[point1].Normal);
		Vertices[point1].Bitangent = bitangentOne;
		Vertices[point1].Bitangent -= Vertices[point1].Normal * D3DXVec3Dot(&Vertices[point1].Bitangent, &Vertices[point1].Normal);

		Vertices[point2].Tangent = tangentOne + tangentTwo;
		D3DXVec3Normalize(&Vertices[point2].Tangent, &Vertices[point2].Tangent);
		Vertices[point2].Tangent -= Vertices[point2].Normal * D3DXVec3Dot(&Vertices[point2].Tangent, &Vertices[point2].Normal);
		Vertices[point2].Bitangent = bitangentOne + bitangentTwo;
		D3DXVec3Normalize(&Vertices[point2].Bitangent, &Vertices[point2].Bitangent);
		Vertices[point2].Bitangent -= Vertices[point2].Normal * D3DXVec3Dot(&Vertices[point2].Bitangent, &Vertices[point2].Normal);


		Vertices[point3].Tangent = tangentOne + tangentTwo;
		D3DXVec3Normalize(&Vertices[point3].Tangent, &Vertices[point3].Tangent);
		Vertices[point3].Tangent -= Vertices[point3].Normal * D3DXVec3Dot(&Vertices[point3].Tangent, &Vertices[point3].Normal);
		Vertices[point3].Bitangent = bitangentOne + bitangentTwo;
		D3DXVec3Normalize(&Vertices[point3].Bitangent, &Vertices[point3].Bitangent);
		Vertices[point3].Bitangent -= Vertices[point3].Normal * D3DXVec3Dot(&Vertices[point3].Bitangent, &Vertices[point3].Normal);

		Vertices[point4].Tangent = tangentTwo;
		Vertices[point4].Tangent -= Vertices[point4].Normal * D3DXVec3Dot(&Vertices[point4].Tangent, &Vertices[point4].Normal);
		Vertices[point4].Bitangent = bitangentOne;
		Vertices[point4].Bitangent -= Vertices[point4].Normal * D3DXVec3Dot(&Vertices[point4].Bitangent, &Vertices[point4].Normal);

	}
	for (int i = 0; i < numVertices; i++)
	{
		D3DXVec3Normalize(&Vertices[i].Normal, &Vertices[i].Normal);
		D3DXVec3Normalize(&Vertices[i].Tangent, &Vertices[i].Tangent);
		D3DXVec3Normalize(&Vertices[i].Bitangent, &Vertices[i].Bitangent);
	}


	
}
void Quadtree::computeTangentBitangent(D3DXVECTOR3& P1, D3DXVECTOR3& P2, D3DXVECTOR3& P3,
									D3DXVECTOR2& UV1, D3DXVECTOR2& UV2, D3DXVECTOR2& UV3,
									D3DXVECTOR3 &tangent, D3DXVECTOR3 &bitangent)
{
	D3DXVECTOR3 Edge1 = P2 - P1;
	D3DXVECTOR3 Edge2 = P3 - P1;
	D3DXVECTOR2 Edge1UV = UV2 - UV1;
	D3DXVECTOR2 Edge2UV = UV3 - UV1;

	float cp = Edge1UV.y * Edge2UV.x - Edge1UV.x * Edge2UV.y;
	if (cp != 0.0f)
	{
		float mul = 1.0f/cp;
		tangent = (Edge1 * -Edge2UV.y + Edge2 * Edge1UV.y) * mul;
		bitangent = (Edge1 * -Edge2UV.x + Edge2 * Edge1UV.x) * mul;

		D3DXVec3Normalize(&tangent, &tangent);
		D3DXVec3Normalize(&bitangent, &bitangent);

	}
}

