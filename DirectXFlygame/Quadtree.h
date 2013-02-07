#include <D3DX11.h>
#include <string>
#include <array>
#include <fstream>
using namespace std; 


struct VertexPositionColor
{
	D3DXVECTOR3 Position;
	D3DXVECTOR4 Color;
	D3DXVECTOR3 Normal;
	D3DXVECTOR3 Tangent;
	D3DXVECTOR3 Bitangent;
	D3DXVECTOR2 UV;
};

class Quadtree
{
//----------------------------------------------
//Methods
//----------------------------------------------
public:
	Quadtree(const char* fileName, int Width, int Height);	
	VertexPositionColor* getVertices();
	D3DXMATRIX getWorldMatrix();
	int* getIndices();
	int numVertices;
	int numIndices;
	

	
	

protected:
	bool heightFromFile();
	void createIndices();
	void loadNormals();
	void computeTangentBitangent(D3DXVECTOR3& P1, D3DXVECTOR3& P2, D3DXVECTOR3& P3,
									D3DXVECTOR2& UV1, D3DXVECTOR2& UV2, D3DXVECTOR2& UV3,
									D3DXVECTOR3 &tangent, D3DXVECTOR3 &bitangent);
	const char* g_HeightMapFile;
	D3DXMATRIX worldMatrix;
	int* indices;
	int imgSize;
	int g_Width;
	int g_Height;
	VertexPositionColor* Vertices;
};