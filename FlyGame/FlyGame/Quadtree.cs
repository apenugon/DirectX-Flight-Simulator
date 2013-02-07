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

//Credit goes to Jon Ferraris for the basic theory/sample code

namespace FlyGame
{
    public unsafe struct Node //Defines a node structure
    {
        public bool isLeaf; //Whether or not it is the lowest subnode
        public Vector3[] boundingCoords; //Boundaries for the node's area
        public int[] VertexStrip1;
        public int[] VertexStrip2;
        public int[] VertexStrip3;
        public int[] VertexStrip4;
        public int[] Indices;
        public int YCoord;
        public int XCoord;
        public int ID; //The ID of the node
        public int parentID; //The ID of the node's parent
        public Node[] children; //The Children of the node
        VertexPositionTextureWeighted Vertexes;
        public BoundingBox Borders;
       
        
    }
    public struct Vertex
    {
        public float X, Z;
    }
    public static class BoundingFrustumRenderer
    {
        #region Fields

        static VertexPositionColor[] verts = new VertexPositionColor[8];
        static int[] indices = new int[]
        {
            0, 1,
            1, 2,
            2, 3,
            3, 0,
            0, 4,
            1, 5,
            2, 6,
            3, 7,
            4, 5,
            5, 6,
            6, 7,
            7, 4,
        };

        static BasicEffect effect;
        static VertexDeclaration vertDecl;

        #endregion

        /// <summary>
        /// Renders the bounding frustum for debugging purposes.
        /// </summary>
        /// <param name="frustum">The frustum to render.</param>
        /// <param name="graphicsDevice">The graphics device to use when rendering.</param>
        /// <param name="view">The current view matrix.</param>
        /// <param name="projection">The current projection matrix.</param>
        /// <param name="color">The color to use drawing the lines of the frustum.</param>
        public static void Render(
            BoundingFrustum frustum,
            GraphicsDevice graphicsDevice,
            Matrix view,
            Matrix projection,
            Color color)
        {
            if (effect == null)
            {
                effect = new BasicEffect(graphicsDevice, null);
                effect.VertexColorEnabled = true;
                effect.LightingEnabled = false;
                vertDecl = new VertexDeclaration(graphicsDevice, VertexPositionColor.VertexElements);
            }

            Vector3[] corners = frustum.GetCorners();
            for (int i = 0; i < 8; i++)
            {
                verts[i].Position = corners[i];
                verts[i].Color = color;
            }

            graphicsDevice.VertexDeclaration = vertDecl;

            effect.View = view;
            effect.Projection = projection;

            effect.Begin();
            foreach (EffectPass pass in effect.CurrentTechnique.Passes)
            {
                pass.Begin();

                graphicsDevice.DrawUserIndexedPrimitives(
                    PrimitiveType.LineList,
                    verts,
                    0,
                    8,
                    indices,
                    0,
                    indices.Length / 2);

                pass.End();
            }
            effect.End();
        }
    }
    public static class BoundingBoxRenderer
    {
        #region Fields

        static VertexPositionColor[] verts = new VertexPositionColor[8];
        static int[] indices = new int[]
        {
            0, 1,
            1, 2,
            2, 3,
            3, 0,
            0, 4,
            1, 5,
            2, 6,
            3, 7,
            4, 5,
            5, 6,
            6, 7,
            7, 4,
        };

        static BasicEffect effect;
        static VertexDeclaration vertDecl;

        #endregion

        /// <summary>
        /// Renders the bounding box for debugging purposes.
        /// </summary>
        /// <param name="box">The box to render.</param>
        /// <param name="graphicsDevice">The graphics device to use when rendering.</param>
        /// <param name="view">The current view matrix.</param>
        /// <param name="projection">The current projection matrix.</param>
        /// <param name="color">The color to use drawing the lines of the box.</param>
        public static void Render(
            BoundingBox box,
            GraphicsDevice graphicsDevice,
            Matrix view,
            Matrix projection,
            Color color)
        {
            if (effect == null)
            {
                effect = new BasicEffect(graphicsDevice, null);
                effect.VertexColorEnabled = true;
                effect.LightingEnabled = false;
                vertDecl = new VertexDeclaration(graphicsDevice, VertexPositionColor.VertexElements);
            }

            Vector3[] corners = box.GetCorners();
            for (int i = 0; i < 8; i++)
            {
                verts[i].Position = corners[i];
                verts[i].Color = color;
            }

            graphicsDevice.VertexDeclaration = vertDecl;

            effect.View = view;
            effect.Projection = projection;

            effect.Begin();
            foreach (EffectPass pass in effect.CurrentTechnique.Passes)
            {
                pass.Begin();

                graphicsDevice.DrawUserIndexedPrimitives(
                    PrimitiveType.LineList,
                    verts,
                    0,
                    8,
                    indices,
                    0,
                    indices.Length / 2);

                pass.End();
            }
            effect.End();
        }
    }
    class Quadtree
    {
        int GridWidth;
        int GridHeight;
        public float[,] heightData;
        public float[,] grassData;
        public float[,] dirtData;
        public float[,] stoneData;
        VertexPositionTextureWeighted[] vertices;
        int TotalTreeID;
        Vector3[] norm = new Vector3[4];
        int[] staticInt = new int[4];

        int newGridWidth;
        int newGridHeight;
        bool nodeTypeLeaf;
        public QuadNode root;
        int[] BoundingBox;
        Vector3 cameraPosition;
        public BoundingFrustum Frustum;
        public BoundingFrustum LODFrustum;
        public BoundingFrustum LOD2Frustum;
        public List<QuadNode> nodeList = new List<QuadNode>();
        public List<QuadNode> nodeList2 = new List<QuadNode>();
        public List<VertexPositionTextureWeighted> a;
        BoundingSphere[] spheres = new BoundingSphere[8];
        Plane[] frustumPlanes = new Plane[6];
        public Texture2D heightMap;
    
         public Quadtree(Texture2D heightMap, Texture2D colorMap, float[,] data, VertexPositionTextureWeighted[] verticesFromGame, Vector3 camPos, BoundingFrustum bounding, GraphicsDevice device)
         {
             this.heightMap = heightMap;
             cameraPosition = camPos;
             Frustum = bounding;
             vertices = verticesFromGame;
             LoadHeightMapPixelData(heightMap);
             LoadHeightMapTextureData(colorMap);
             SetupNodes(heightMap, device);
         }
         private void SetupNodes(Texture2D heightMap, GraphicsDevice device)
         {
             Matrix worldMatrix = Matrix.CreateTranslation(-heightMap.Width / 2.0f, 0, heightMap.Height / 2.0f) * Matrix.CreateScale(2.0f) ;
             root = new QuadNode(this, 0, heightMap.Width, 0, heightMap.Height, 0, null, heightMap.Width / 128, heightMap.Height / 128 , heightMap.Width, worldMatrix, grassData, 1);
             foreach (QuadNode node in nodeList)
             {
                 node.box.Min = Vector3.Transform(node.box.Min, worldMatrix);
                 node.box.Max = Vector3.Transform(node.box.Max, worldMatrix);
             }
             foreach (QuadNode node in nodeList)
                 if (node.isLeaf)
                 {
                     node.constructVerts(heightData, device, grassData, dirtData, stoneData);
                     node.constructLODVerts(heightData, device, grassData, dirtData, stoneData);
                     node.constructLOD2Verts(heightData, device, grassData, dirtData, stoneData);
                 }
            
         }
        public void UpdateFrustum(BoundingFrustum bounding, Vector3 camPos)
        {
            Frustum = bounding;
            cameraPosition = camPos;
        }
        private void LoadHeightMapPixelData(Texture2D heightMap)
        {
            float terrainWidth = heightMap.Width;
            float terrainHeight = heightMap.Height;

            Color[] heightMapColors = new Color[((int)terrainWidth) * ((int)terrainHeight)];
            heightMap.GetData(heightMapColors);
   
            heightData = new float[(int)terrainWidth, (int)terrainHeight];
            for (int x = 0; x < terrainWidth; x++)
                for (int y = 0; y < terrainHeight; y++)
                    heightData[x, y] = heightMapColors[x + y * (int)terrainWidth].R * 2; //The height scaling of the data pulled from the heightmap
        }
        private void LoadHeightMapTextureData(Texture2D heightMap)
        {
            float terrainWidth = heightMap.Width;
            float terrainHeight = heightMap.Height;

            Color[] heightMapColors = new Color[((int)terrainWidth) * ((int)terrainHeight)];
            heightMap.GetData(heightMapColors);

            stoneData = new float[(int)terrainWidth, (int)terrainHeight];
            for (int x = 0; x < terrainWidth; x++)
                for (int y = 0; y < terrainHeight; y++)
                    stoneData[x, y] = heightMapColors[x + y * (int)terrainWidth].G / 1; //The height scaling of the data pulled from the heightmap
            dirtData = new float[(int)terrainWidth, (int)terrainHeight];
            for (int x = 0; x < terrainWidth; x++)
                for (int y = 0; y < terrainHeight; y++)
                    dirtData[x, y] = heightMapColors[x + y * (int)terrainWidth].R / 1;
            grassData = new float[(int)terrainWidth, (int)terrainHeight];
            for (int x = 0; x < terrainWidth; x++)
                for (int y = 0; y < terrainHeight; y++)
                    grassData[x, y] = heightMapColors[x + y * (int)terrainWidth].B / 1;
        }
        public bool CheckVisible(BoundingFrustum frustum, QuadNode ANode)
        {
            
            int counter = 0;
            int rightIn;
            int outNo;
            int total = 0;
            bool visibility = true;
            frustumPlanes[0] = frustum.Near;
            frustumPlanes[1] = frustum.Far;
            frustumPlanes[2] = frustum.Top;
            frustumPlanes[3] = frustum.Bottom;
            frustumPlanes[4] = frustum.Left;
            frustumPlanes[5] = frustum.Right;
            Vector3[] corners = ANode.box.GetCorners();
            for (int z = 0; z < spheres.Length; z++)
            {
                spheres[z].Center = corners[z];
                spheres[z].Radius = 2.0f;
            }
            for (int x = 0; x < 6; x++)
            {
                rightIn = 0;
                outNo = 0;
                for (int i = 0; i < 8; i++)
                    if (frustumPlanes[x].Intersects(spheres[i]) == PlaneIntersectionType.Back)
                    {
                        outNo++;
                    }
                    else
                        rightIn++;
                if (rightIn == 8)
                    return false;
            }
            return visibility;
        
    /*        Matrix worldMatrix = Matrix.CreateTranslation(-heightMap.Width / 2.0f, 0, heightMap.Height / 2.0f);
            cameraPosition = Vector3.Transform(cameraPosition, worldMatrix);
            BoundingBox pos = new BoundingBox(new Vector3(cameraPosition.X - 1, cameraPosition.Y - 1, cameraPosition.Z - 1), cameraPosition);
            if (ANode.box.Min.X < cameraPosition.X  &&  cameraPosition.X < ANode.box.Max.X &&
                ANode.box.Min.Y < cameraPosition.Y  && cameraPosition.Y < ANode.box.Max.Y &&
                ANode.box.Min.Z < cameraPosition.Z  && cameraPosition.Z < ANode.box.Max.Z
                )
                return true;
            else
                return false;*/
        }
        /*   private void CreateFirstNodeLevel()
           {
               for (int x = 0; x < GridWidth / 8; x++)
                   for (int y = 0; y < GridHeight / 8; y++)
                   {
                       nodeList[0, x, y] = new Node();
                       nodeList[0, x, y].isLeaf = true;
                       nodeList[0, x, y].XCoord = x * 8;
                       nodeList[0, x, y].YCoord = y * 8;
                       nodeList[0, x, y].Indices = LoadIndicesFromNode(nodeList[0, x, y]);
                       nodeList[0, x, y].Borders = new BoundingBox(new Vector3(x * 8, 0, y * 8), new Vector3((x * 8) + 8, 20f, (y * 8) + 8));
                     
                   }
           }
           private int[] LoadIndicesFromNode(Node ANode)
           {
             
                   int counter = 0;
                   int[] terrIndices;
                   terrIndices = new int[8 * 8 * 8];
                   for (int y = ANode.YCoord; y < ANode.YCoord + 8; y++)
                       for (int x = ANode.XCoord; x < ANode.XCoord + 8; x++)
                       {
                           int lleft = x + y * GridWidth;
                           int tleft = x + (y + 1) * GridWidth;
                           int lright = (x + 1) + y * GridWidth;
                           int tright = (x + 1) + (y + 1) * GridWidth;

                           terrIndices[counter++] = tleft;
                           terrIndices[counter++] = lright;
                           terrIndices[counter++] = lleft;

                           terrIndices[counter++] = tleft;
                           terrIndices[counter++] = tright;
                           terrIndices[counter++] = lright;

                       }
                   return terrIndices;
             
           }
           private void CalculateNormals(Node ANode)
           {
               for (int i = 0; i < vertices.Length; i++)
                   vertices[i].Normal = new Vector3(0, 0, 0);
               for (int i = 0; i < ANode.Indices.Length / 3; i++)
               {
                   int point1 = ANode.Indices[i * 3];
                   int point2 = ANode.Indices[i * 3 + 1];
                   int point3 = ANode.Indices[i * 3 + 2];

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

           private void CreateUpperLevelNodes()
           {
               int length = nodeList.Length;
               for (int x = 1; x < TotalTreeID + 1; x++)
                   for (int y = 0; y < length / 2; y++)
                       for (int z = 0; z < length / 2; z++)
                       {
                           nodeList[x, y, z] = new Node();
                           try
                           {
                               nodeList[x, y, z].children = new Node[4];
                               try
                               {
                                   nodeList[x, y, z].children[0] = nodeList[x - 1, y, z];
                                   nodeList[x, y, z].children[1] = nodeList[x - 1, y + 1, z];
                                   nodeList[x, y, z].children[2] = nodeList[x - 1, y, z + 1];
                                   nodeList[x, y, z].children[3] = nodeList[x - 1, y + 1, z + 1];
                               }
                               catch (IndexOutOfRangeException)
                               {                                
                               }
                           }
                           catch (OutOfMemoryException) { } 
                       }
           }
           public void Draw()
           {
           }

          /*      private void CreateNode(int[] Bounding, int ParentID, int NodeID)
                {
                //    TotalTreeID = 0;
                    newGridWidth = (int)(vertices[Bounding[1]].Position.X - vertices[Bounding[0]].Position.X);
                    newGridHeight = (int)(vertices[Bounding[2]].Position.Z - vertices[Bounding[0]].Position.Z);
                    nodeList[0].ID = 0;
                    nodeList[0].isLeaf = false;
           //         nodeList[0].boundingCoords = Bounding;
                    nodeList[0].parentID = 0;
                    if (((newGridWidth / 2) <= 4) || ((newGridHeight/2<=4)))
                    {
                        nodeTypeLeaf = true;
                    }
                    else
                        nodeTypeLeaf = false;
                    pNode = nodeList[NodeID];
                    pNode.ID = NodeID;
                    pNode.parentID = ParentID;
                    pNode.boundingCoords = norm;
                    pNode.boundingCoords[0] = vertices[Bounding[0]].Position;

                    pNode.boundingCoords[1] = vertices[Bounding[1]].Position;

                    pNode.boundingCoords[2] = vertices[Bounding[2]].Position;

                    pNode.boundingCoords[3] = vertices[Bounding[3]].Position;
                    pNode.children = staticInt;
                    pNode.isLeaf = nodeTypeLeaf;
                    if (pNode.isLeaf)
                    {
                    }
                    else if (!pNode.isLeaf)
                    {
                        //Top left child node-----------------------  
                        BoundingBox = staticInt;
                        TotalTreeID++;
                 
                        pNode.children[0] = TotalTreeID;

                        //Top left corner
                        BoundingBox[0] = Bounding[0];
                        BoundingBox[1] = Bounding[0] + ((Bounding[1] - Bounding[0]) / 2);
                        BoundingBox[2] = Bounding[0] + ((Bounding[2] - Bounding[0]) / 2);
                        BoundingBox[3] = Bounding[0] + ((Bounding[2] - Bounding[0]) / 2) + ((Bounding[1] - Bounding[0]) / 2);
                        CreateNode(BoundingBox, NodeID, TotalTreeID);

                        //Top right child node -----------------------
                        TotalTreeID++;
                        pNode.children[1] = TotalTreeID;

                        BoundingBox[0] = Bounding[0] + ((Bounding[1] - Bounding[0]) / 2);
                        BoundingBox[1] = Bounding[1];
                        BoundingBox[2] = Bounding[0] + ((Bounding[2] - Bounding[0]) / 2) + ((Bounding[1] - Bounding[0]) / 2);
                        BoundingBox[3] = Bounding[0] + ((Bounding[2] - Bounding[0]) / 2) + ((Bounding[1] - Bounding[0]));

                        CreateNode(BoundingBox, NodeID, TotalTreeID);

           TotalTreeID++;
           pNode.children[2] = TotalTreeID;

           //between b[0] and b[2]
           BoundingBox[0] = Bounding[0]+((Bounding[2]-Bounding[0])/2);
           //middle of node
           BoundingBox[1] = Bounding[0]+((Bounding[2]-Bounding[0])/2)
                                       +((Bounding[1]-Bounding[0])/2);
           //Bottom-Left i.e. b[2]
           BoundingBox[2] = Bounding[2];
           //between b[2] and b[3]
           BoundingBox[3] = Bounding[2]+((Bounding[3]-Bounding[2])/2);

           CreateNode(BoundingBox,NodeID,TotalTreeID);

           //**************************************************************************

           TotalTreeID++;
           pNode.children[3] = TotalTreeID;

           //middle of node
           BoundingBox[0] = Bounding[0]+((Bounding[2]-Bounding[0])/2)
                                       +((Bounding[1]-Bounding[0])/2);
           //between b[1] and b[3]
           BoundingBox[1] = Bounding[0]+((Bounding[2]-Bounding[0])/2) + newGridWidth;
           //between b[2] and b[3]
           BoundingBox[2] = Bounding[2]+((Bounding[3]-Bounding[2])/2);
           //Bottom-Right i.e. b[3]
           BoundingBox[3] = Bounding[3];

           CreateNode(BoundingBox,NodeID,TotalTreeID);
  
                    }
                } */

    }
}
