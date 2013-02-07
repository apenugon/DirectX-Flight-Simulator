using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.GamerServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Media;
using Microsoft.Xna.Framework.Net;
using Microsoft.Xna.Framework.Storage;

namespace FlyGame
{
    class QuadNode
    {
        public Quadtree tree;
        public readonly QuadNode parent;
        public readonly QuadNode north;
        public readonly QuadNode south;
        public readonly QuadNode east;
        public readonly QuadNode west;
        private int width;
        private int height;
        public int Xmax;
        public int Zmax;
        public int Xmin;
        public int Zmin;
        public QuadNode topRight;
        public QuadNode topLeft;
        public QuadNode bottomRight;
        public QuadNode bottomLeft;
        int level;
        public bool isLeaf = false;
        VertexPositionTextureWeighted[] vertices;
        public int[] vertIndices;
        public int[] LODIndices;
        public int[] LOD2Indices;
        public int LOD = -1;
        VertexDeclaration NodeDeclaration;
        public VertexBuffer NodeVertexBuffer;
        public IndexBuffer NodeIndexBuffer;
        public IndexBuffer LODIndexBuffer;
        public IndexBuffer LOD2IndexBuffer;

        internal LinkedListNode<QuadNode> myNode;

        public BoundingBox box;

        public QuadNode(Quadtree tree, int minX, int maxX, int minZ, int maxZ, int treeLevel, QuadNode Parent, float minWidth, float minHeight, float width, Matrix worldMatrix, float[,] texData, int pos)
        {
            this.tree = tree;
            this.parent = Parent;
            this.Xmax = maxX;
            this.Xmin = minX;
            this.Zmax = maxZ;
            this.Zmin = minZ;
            this.width = maxX - minX;
            this.height = maxZ - minZ;
            this.box.Min = new Vector3(minX, 0, -maxZ);
            this.box.Max = new Vector3(maxX, 250 * 2, -minZ);
            if (treeLevel == 2)
                tree.nodeList2.Add(this);
            if (maxX - minX > minWidth && maxZ - minZ > minHeight)
            {
                topLeft = new QuadNode(tree, minX, (maxX + minX) / 2, (maxZ + minZ) / 2, maxZ, treeLevel + 1, this, minWidth, minHeight, width / 2f, worldMatrix, texData, 1);
                topRight = new QuadNode(tree, (maxX + minX) / 2, maxX, (maxZ + minZ) / 2, maxZ, treeLevel + 1, this, minWidth, minHeight, width / 2f, worldMatrix, texData, 2);
                bottomLeft = new QuadNode(tree, minX, (maxX + minX) / 2, minZ, (maxZ + minZ) / 2, treeLevel + 1, this, minWidth, minHeight, width / 2f, worldMatrix, texData, 3);
                bottomRight = new QuadNode(tree, (maxX + minX) / 2, maxX, minZ, (maxZ + minZ) / 2, treeLevel + 1, this, minWidth, minHeight, width / 2f, worldMatrix, texData, 4);
                this.box.Min = Vector3.Transform(this.box.Min, worldMatrix);
                this.box.Max = Vector3.Transform(this.box.Max, worldMatrix);
            }
            else
            {
                this.isLeaf = true;
              //  constructVerts(vertices1); 
                tree.nodeList.Add(this);
            }
           
        }
        public void constructVerts1(float[,] heightData, GraphicsDevice device, float[,] grassData, float[,] dirtData, float[,] stoneData)
        {
            vertices = new VertexPositionTextureWeighted[(width) * height * 2];
            for (int x = Xmin - 6; x < Xmax; x++)
                for (int y = Zmin - 3; y < Zmax; y++)
                {
                    vertices[(Xmax - x) + (Zmax - y) * (width )].Position = new Vector3(x, heightData[Math.Abs(x),Math.Abs(y)], -y);
                    vertices[(Xmax - x) + (Zmax - y) * (width)].TextureCoordinate.X = (float)x / 30.0f;
                    vertices[(Xmax - x) + (Zmax - y) * width].TextureCoordinate.Y = (float)y / 30.0f;

                    vertices[(Xmax - x) + (Zmax - y) * width].TexWeights.X = MathHelper.Clamp(Math.Abs(stoneData[Math.Abs(x), Math.Abs(y)] - 30.0f) / 80.0f, 0, 10);
                    vertices[(Xmax - x) + (Zmax - y) * width].TexWeights.Y = MathHelper.Clamp(Math.Abs(grassData[Math.Abs(x),Math.Abs(y)] - 30.0f) / 80.0f, 0, 10);
                    vertices[(Xmax - x) + (Zmax - y) * width].TexWeights.Z = MathHelper.Clamp(Math.Abs(stoneData[Math.Abs(x), Math.Abs(y)] - 10.0f) / 80.0f, 0, 10);

                    float total = vertices[(Xmax - x) + (Zmax -  y) * width].TexWeights.X;
                    total += vertices[(Xmax - x) + (Zmax - y) * width].TexWeights.Y;
                    total += vertices[(Xmax - x) + (Zmax - y) * width].TexWeights.Z;

                    vertices[(Xmax - x) + (Zmax - y) * width].TexWeights.X /= total;
                    vertices[(Xmax - x) + (Zmax - y) * width].TexWeights.Y /= total;
                    vertices[(Xmax - x) + (Zmax - y) * width].TexWeights.Z /= total;
                }
            NodeDeclaration = new VertexDeclaration(device, VertexPositionTextureWeighted.VertexElements);
            LoadTerrainIndices();
            LoadNormals();
            CopyToBuffers(device);
        }
       
        public void constructVerts(float[,] heightData, GraphicsDevice device, float[,] grassData, float[,] dirtData, float[,] stoneData)
        {
            int counter = 0;
            int nWidth = tree.heightMap.Width;
            vertIndices = new int[(height) * (width) * 8];
            for (int y = Math.Abs(Zmin - 1); y < Zmax - 1; y++)
                for (int x = Math.Abs(Xmin - 1); x < Xmax - 1; x++)
                {
                    int lleft = x + y * nWidth;
                    int tleft = x + (y + 1) * nWidth;
                    int lright = (x + 1) + y * nWidth;
                    int tright = (x + 1) + (y + 1) * nWidth;

                    vertIndices[counter++] = tleft;
                    vertIndices[counter++] = lright;
                    vertIndices[counter++] = lleft;

                    vertIndices[counter++] = tleft;
                    vertIndices[counter++] = tright;
                    vertIndices[counter++] = lright;

                }
        }
        public void constructLODVerts(float[,] heightData, GraphicsDevice device, float[,] grassData, float[,] dirtData, float[,] stoneData)
        {
            int counter = 0;
            int nWidth = tree.heightMap.Width;
            LODIndices = new int[(height) * (width) * 8];
            for (int y = Math.Abs(Zmin - 1); y < Zmax - 1; y+=2)
                for (int x = Math.Abs(Xmin - 1); x < Xmax - 1; x+=2)
                {
                    int lleft = x + y * nWidth;
                    int tleft = x + (y + 2) * nWidth;
                    int lright = (x + 2) + y * nWidth;
                    int tright = (x + 2) + (y + 2) * nWidth;

                    LODIndices[counter++] = tleft;
                    LODIndices[counter++] = lright;
                    LODIndices[counter++] = lleft;

                    LODIndices[counter++] = tleft;
                    LODIndices[counter++] = tright;
                    LODIndices[counter++] = lright;

                }
        }
        public void constructLOD2Verts(float[,] heightData, GraphicsDevice device, float[,] grassData, float[,] dirtData, float[,] stoneData)
        {
            int counter = 0;
            int nWidth = tree.heightMap.Width;
            LOD2Indices = new int[(height) * (width) * 8];
            for (int y = Math.Abs(Zmin - 1); y < Zmax - 1; y += 5)
                for (int x = Math.Abs(Xmin - 1); x < Xmax - 1; x += 5)
                {
                    int lleft = x + y * nWidth;
                    int tleft = x + (y + 5) * nWidth;
                    int lright = (x + 5) + y * nWidth;
                    int tright = (x + 5) + (y + 5) * nWidth;

                    LOD2Indices[counter++] = tleft;
                    LOD2Indices[counter++] = lright;
                    LOD2Indices[counter++] = lleft;

                    LOD2Indices[counter++] = tleft;
                    LOD2Indices[counter++] = tright;
                    LOD2Indices[counter++] = lright;

                }
            CopyToBuffers(device);
        }
        private void LoadTerrainIndices()
        {
            int height = Zmax - Zmin;
            int counter = 0;
            vertIndices = new int[(height - 1) * (width - 1) * 9];
            for (int y = 0; y < height - 1; y++)
                for (int x = 0; x < width - 1; x++)
                {
                    int lleft = x + y * width;
                    int tleft = x + (y + 1) * width;
                    int lright = (x + 1) + y * width;
                    int tright = (x + 1) + (y + 1) * width;

                    vertIndices[counter++] = tleft;
                    vertIndices[counter++] = lright;
                    vertIndices[counter++] = lleft;

                    vertIndices[counter++] = tleft;
                    vertIndices[counter++] = tright;
                    vertIndices[counter++] = lright;

                }
        }
        private void CopyToBuffers(GraphicsDevice device)
        {
         //   NodeVertexBuffer = new VertexBuffer(device, vertices.Length * VertexPositionTextureWeighted.SizeInBytes, BufferUsage.WriteOnly);
         //   NodeVertexBuffer.SetData(vertices);
            NodeIndexBuffer = new DynamicIndexBuffer(device, typeof(int), vertIndices.Length, BufferUsage.WriteOnly);
            NodeIndexBuffer.SetData(vertIndices);
            LODIndexBuffer = new DynamicIndexBuffer(device, typeof(int), LODIndices.Length, BufferUsage.WriteOnly);
            LODIndexBuffer.SetData(LODIndices);

            LOD2IndexBuffer = new DynamicIndexBuffer(device, typeof(int), LOD2Indices.Length, BufferUsage.WriteOnly);
            LOD2IndexBuffer.SetData(LOD2Indices);
           
        }
        private void LoadNormals()
        {
            for (int i = 0; i < vertices.Length; i++)
                vertices[i].Normal = new Vector3(0, 0, 0);

            for (int i = 0; i < vertIndices.Length / 3; i++)
            {
                int point1 = vertIndices[i * 3];
                int point2 = vertIndices[i * 3 + 1];
                int point3 = vertIndices[i * 3 + 2];

                Vector3 sideOne = vertices[point1].Position - vertices[point2].Position;
                Vector3 sideTwo = vertices[point1].Position - vertices[point3].Position;
                Vector3 Normal = Vector3.Cross(sideOne, sideTwo);

                vertices[point1].Normal += Normal;
                vertices[point2].Normal += Normal;
                vertices[point3].Normal += Normal;
            }
            for (int i = 0; i < vertices.Length; i++)
            {
                vertices[i].Normal.Normalize();
            }
        }
        public void Draw(GraphicsDevice device, Effect effect, Matrix viewMatrix, Matrix projectionMatrix, float terrainWidth, float terrainHeight, VertexPositionTextureWeighted[] verts)
        {
            if (isLeaf)
            {
                if (tree.CheckVisible(tree.Frustum, this))
                {

                    if (tree.CheckVisible(tree.LODFrustum, this))
                    {
                        device.Indices = NodeIndexBuffer;
                        LOD = 3;
                    }
                    else if (tree.CheckVisible(tree.LOD2Frustum, this))
                    {
                        device.Indices = LODIndexBuffer;
                        LOD = 2;
                    }
                    else
                    {
                        device.Indices = LOD2IndexBuffer;
                        LOD = 1;
                    }
                        
                        device.DrawIndexedPrimitives(PrimitiveType.TriangleList, 0, 0, verts.Length, 0, (vertIndices.Length)/ 3);
                        
                    

                    

                }
            }
            else
            {
                if (tree.CheckVisible(tree.Frustum, this))
             //   if (isVisible(tree.Frustum))
                {
                    topLeft.Draw(device, effect, viewMatrix, projectionMatrix, terrainWidth, terrainHeight, verts);
                    topRight.Draw(device, effect, viewMatrix, projectionMatrix, terrainWidth, terrainHeight, verts);
                    bottomLeft.Draw(device, effect, viewMatrix, projectionMatrix, terrainWidth, terrainHeight, verts);
                    bottomRight.Draw(device, effect, viewMatrix, projectionMatrix, terrainWidth, terrainHeight, verts);
                }
            }
             

        }
        public bool isVisible(BoundingFrustum frustum)
        {
            if (this.box.Intersects(frustum.Top) == PlaneIntersectionType.Intersecting)
                return true;
            else
                return false;
        }
    }
}
