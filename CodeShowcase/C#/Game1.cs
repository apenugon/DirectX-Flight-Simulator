using System;
using System.Collections.Generic;
using System.Linq;
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
    public struct VertexPositionTextureWeighted
    {
        public Vector3 Position;
        public Vector3 Normal;
        public Vector4 TextureCoordinate;
        public Vector4 TexWeights;


        public static int SizeInBytes = (3 + 3 + 4 + 4) * sizeof(float);
        public static VertexElement[] VertexElements = new VertexElement[]
        {
            new VertexElement(0, 0, VertexElementFormat.Vector3, VertexElementMethod.Default, VertexElementUsage.Position, 0),
            new VertexElement(0, sizeof(float) * 3, VertexElementFormat.Vector3, VertexElementMethod.Default, VertexElementUsage.Normal, 0),
            new VertexElement(0, sizeof(float) * 6, VertexElementFormat.Vector4, VertexElementMethod.Default, VertexElementUsage.TextureCoordinate, 0),
            new VertexElement(0, sizeof(float) * 10, VertexElementFormat.Vector4, VertexElementMethod.Default, VertexElementUsage.TextureCoordinate, 1)

        };

    }
    /// <summary>
    /// This is the main type for your game
    /// </summary>
    public class Game1 : Microsoft.Xna.Framework.Game
    {
        //Declare variables
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        GraphicsDevice device;
        Effect effect; 

        //Declare vars relating to the camera
        Vector3 cameraPos = new Vector3(130, 30, -50);
        Vector3 cameraFinalTarget = new Vector3(0, 0, 0);
        Vector3 cameraRotatedUpVector = new Vector3(0, 1, 0);
        float upDownRotation = -MathHelper.Pi / 10;
        float leftRightRotation = MathHelper.PiOver2;
        const float rotationSpeed = 0.3f;
        const float moveSpeed = 3.0f;
        Matrix viewMatrix;
        Matrix projectionMatrix;
        Matrix LODProjectionMatrix;
        Matrix LOD2ProjectionMatrix;
        MouseState originalMouseState;
        BoundingFrustum cameraFrustum;
        

        //Declare the heightmap textures
        Texture2D heightMap;
        Texture2D colorMap;
        VertexDeclaration heightMapVertexDeclaration;
        VertexBuffer heightMapVertexBuffer;
        private float[,] heightData; //Declares a variable that will store the x and y heightmap data
        int[] terrainIndices;
        VertexPositionTextureWeighted[] TextureVertices;
        IndexBuffer terrainIndexBuffer;
        Quadtree A;
        internal LinkedList<Node> Nodes = new LinkedList<Node>();
        float terrainWidth;
        float terrainHeight;
        Texture2D groundTexture;
        Texture2D grassTexture;
        Texture2D dirtTexture;

        Model Plane;
        Vector3 modelPosition = Vector3.Zero;
        float modelRotation = 0.0f;
       


        public Game1()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
        }

        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {
            // TODO: Add your initialization logic here
         
            base.Initialize();
        }

        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            device = GraphicsDevice;// Create a new SpriteBatch, which can be used to draw textures.
            Mouse.SetPosition(device.Viewport.Width / 2, device.Viewport.Height / 2);
            originalMouseState = Mouse.GetState();
            effect = Content.Load<Effect>("Effects");
            spriteBatch = new SpriteBatch(GraphicsDevice);
            LoadTextures();
            terrainWidth = heightMap.Width;
            terrainHeight = heightMap.Height;
            UpdateViewMatrix();
         //   viewMatrix = Matrix.CreateLookAt(new Vector3(0, 100, 100), new Vector3(0, 0, 0), new Vector3(0, 1, 0));
            projectionMatrix = Matrix.CreatePerspectiveFieldOfView(MathHelper.PiOver4, device.Viewport.AspectRatio, 0.3f, 2500.0f);

            Plane = Content.Load<Model>("Spaceship");
            Matrix lulz = Matrix.CreatePerspectiveFieldOfView(MathHelper.PiOver4, device.Viewport.AspectRatio, 0.3f, 1.2f);
            LODProjectionMatrix = Matrix.CreatePerspectiveFieldOfView(MathHelper.PiOver4, device.Viewport.AspectRatio, 0.3f, 350.0f);
            LOD2ProjectionMatrix = Matrix.CreatePerspectiveFieldOfView(MathHelper.PiOver4, device.Viewport.AspectRatio, 0.3f, 1000.0f);
            cameraFrustum = new BoundingFrustum(viewMatrix * projectionMatrix);
            A = new Quadtree(heightMap, colorMap, heightData, TextureVertices, cameraPos, cameraFrustum, device);
            A.LODFrustum = new BoundingFrustum(viewMatrix * projectionMatrix);
            A.LOD2Frustum = new BoundingFrustum(viewMatrix * LOD2ProjectionMatrix);
            constructVerts(A.heightData, A.grassData, A.dirtData, A.stoneData);

            
            // TODO: use this.Content to load your game content here
        }
        private void UpdateViewMatrix()
        {
            Matrix cameraRotation = Matrix.CreateRotationX(upDownRotation) * Matrix.CreateRotationY(leftRightRotation);

            Vector3 cameraOriginalTarget = new Vector3(0, 0, -1);
            Vector3 cameraOriginalUpVector = new Vector3(0, 1, 0);
        
            Vector3 cameraRotatedTarget = Vector3.Transform(cameraOriginalTarget, cameraRotation);
            cameraFinalTarget = cameraPos + cameraRotatedTarget;

            cameraRotatedUpVector = Vector3.Transform(cameraOriginalUpVector, cameraRotation);

            viewMatrix = Matrix.CreateLookAt(cameraPos, cameraFinalTarget, cameraRotatedUpVector);
        }
        /// <summary>
        /// Generates the heightmap height data.
        /// </summary>
        /// <param name="heightMap">The heightmap texture2d.</param>
       

        /// <summary>
        /// Loads all textures in the file
        /// </summary>
        private void LoadTextures()
        {
            heightMap = Content.Load<Texture2D>("Heightmap Plains");
            groundTexture = Content.Load<Texture2D>("ground");
            grassTexture = Content.Load<Texture2D>("Grass");
            colorMap = Content.Load<Texture2D>("Texmap");
            dirtTexture = Content.Load<Texture2D>("moss-on-rocks-texture");
        }
        /// <summary>
        /// UnloadContent will be called once per game and is the place to unload
        /// all content.
        /// </summary>
        protected override void UnloadContent()
        {
            // TODO: Unload any non ContentManager content here
        }
/*        private void UpdateViewMatrix()
        {
            Vector3 cameraInitialTarget = new Vector3(0, 0, 0);
        }
*/        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            // Allows the game to exit
            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed)
                this.Exit();
            float timeDiff = (float)gameTime.ElapsedGameTime.TotalMilliseconds / 1000.0f;
            Input(timeDiff);
            // TODO: Add your update logic here
        //    projectionMatrix = Matrix.CreatePerspectiveFieldOfView(MathHelper.PiOver4, device.Viewport.AspectRatio, 1.0f, 300.0f);
            Console.Write("Hello");
           A.UpdateFrustum(cameraFrustum, cameraPos);
            base.Update(gameTime);
        }
        public void constructVerts(float[,] heightData, float[,] grassData, float[,] dirtData, float[,] stoneData)
        {
            int width = A.heightMap.Width;
            int height = A.heightMap.Height;
            TextureVertices = new VertexPositionTextureWeighted[(width) * height * 2];
            for (int x = 0; x < width; x++)
                for (int y = 0; y < height; y++)
                {
                    TextureVertices[x + y * (width)].Position = new Vector3(x, heightData[Math.Abs(x), Math.Abs(y)], -y);
                    TextureVertices[x + y * (width)].TextureCoordinate.X = (float)x / 30.0f;
                    TextureVertices[x + y * width].TextureCoordinate.Y = (float)y / 30.0f;

                    TextureVertices[x + y * width].TexWeights.X = MathHelper.Clamp(Math.Abs(stoneData[Math.Abs(x), Math.Abs(y)] - 30.0f) / 80.0f, 0, 10);
                    TextureVertices[x + y * width].TexWeights.Y = MathHelper.Clamp(Math.Abs(grassData[Math.Abs(x), Math.Abs(y)] - 30.0f) / 80.0f, 0, 10);
                    TextureVertices[x + y * width].TexWeights.Z = MathHelper.Clamp(Math.Abs(stoneData[Math.Abs(x), Math.Abs(y)] - 10.0f) / 80.0f, 0, 10);

                    float total = TextureVertices[x + y * width].TexWeights.X;
                    total += TextureVertices[x + y * width].TexWeights.Y;
                    total += TextureVertices[x + y * width].TexWeights.Z;

                    TextureVertices[x + y * width].TexWeights.X /= total;
                    TextureVertices[x + y * width].TexWeights.Y /= total;
                    TextureVertices[x + y * width].TexWeights.Z /= total;
                }
            LoadTerrainIndices();
            LoadNormals();
            heightMapVertexDeclaration = new VertexDeclaration(device, VertexPositionTextureWeighted.VertexElements);
            heightMapVertexBuffer = new VertexBuffer(device, TextureVertices.Length * VertexPositionTextureWeighted.SizeInBytes, BufferUsage.WriteOnly);
            heightMapVertexBuffer.SetData(TextureVertices);

        }
        private void LoadTerrainIndices()
        {
            int height = heightMap.Height;
            int width = heightMap.Width;
            int counter = 0;
            terrainIndices = new int[(height - 1) * (width - 1) * 6];
            for (int y = 0; y < height - 1; y++)
                for (int x = 0; x < width - 1; x++)
                {
                    int lleft = x + y * width;
                    int tleft = x + (y + 1) * width;
                    int lright = (x + 1) + y * width;
                    int tright = (x + 1) + (y + 1) * width;

                    terrainIndices[counter++] = tleft;
                    terrainIndices[counter++] = lright;
                    terrainIndices[counter++] = lleft;

                    terrainIndices[counter++] = tleft;
                    terrainIndices[counter++] = tright;
                    terrainIndices[counter++] = lright;

                }
        }
        private void LoadNormals()
        {
            for (int i = 0; i < TextureVertices.Length; i++)
                TextureVertices[i].Normal = new Vector3(0, 0, 0);

            for (int i = 0; i < terrainIndices.Length / 3; i++)
            {
                int point1 = terrainIndices[i * 3];
                int point2 = terrainIndices[i * 3 + 1];
                int point3 = terrainIndices[i * 3 + 2];

                Vector3 sideOne = TextureVertices[point1].Position - TextureVertices[point2].Position;
                Vector3 sideTwo = TextureVertices[point1].Position - TextureVertices[point3].Position;
                Vector3 Normal = Vector3.Cross(sideOne, sideTwo);

                TextureVertices[point1].Normal += Normal;
                TextureVertices[point2].Normal += Normal;
                TextureVertices[point3].Normal += Normal;
            }
            for (int i = 0; i < TextureVertices.Length; i++)
            {
                TextureVertices[i].Normal.Normalize();
            }
        }
   /*     private void CopyToBuffers()
        {
            heightMapVertexBuffer = new VertexBuffer(device, TextureVertices.Length * VertexPositionTextureWeighted.SizeInBytes, BufferUsage.WriteOnly);
            heightMapVertexBuffer.SetData(TextureVertices);
            terrainIndexBuffer = new IndexBuffer(device, typeof(int), terrainIndices.Length, BufferUsage.WriteOnly);
            terrainIndexBuffer.SetData(terrainIndices);
        }
   */     private void Input(float amount)
        {
            MouseState currentState = Mouse.GetState();
            if (currentState != originalMouseState)
            {
                float xDifference = currentState.X - originalMouseState.X;
                float yDifference = currentState.Y - originalMouseState.Y;
                leftRightRotation -= rotationSpeed * xDifference * amount;
                upDownRotation -= rotationSpeed * yDifference * amount;
                Mouse.SetPosition(device.Viewport.Width / 2, device.Viewport.Height / 2);
                UpdateViewMatrix();
            }
            Vector3 moveVector = new Vector3(0, 0, 0);
            KeyboardState keyState = Keyboard.GetState();
            if (keyState.IsKeyDown(Keys.Up) || keyState.IsKeyDown(Keys.W))
                moveVector += new Vector3(0, 0, -30);
            if (keyState.IsKeyDown(Keys.Down) || keyState.IsKeyDown(Keys.S))
                moveVector += new Vector3(0, 0, 30);
            if (keyState.IsKeyDown(Keys.Left) || keyState.IsKeyDown(Keys.A))
                moveVector += new Vector3(-30, 0, 0);
            if (keyState.IsKeyDown(Keys.Right) || keyState.IsKeyDown(Keys.D))
                moveVector += new Vector3(30, 0, 0);
            if (keyState.IsKeyDown(Keys.Q))
                moveVector += new Vector3(0, 30, 0);
            if (keyState.IsKeyDown(Keys.Z))
                moveVector += new Vector3(0, -30, 0);
            addToCameraPosition(moveVector * amount);
            cameraFrustum.Matrix = viewMatrix * projectionMatrix;
            A.Frustum = cameraFrustum;
            A.LODFrustum.Matrix = viewMatrix * LODProjectionMatrix;
            A.LOD2Frustum.Matrix = viewMatrix * LOD2ProjectionMatrix;
        }
        private void addToCameraPosition(Vector3 vectorToAdd)
        {
            Matrix CameraRotation = Matrix.CreateRotationX(upDownRotation) * Matrix.CreateRotationY(leftRightRotation);
            Vector3 rotatedVector = Vector3.Transform(vectorToAdd, CameraRotation);
            cameraPos += moveSpeed * rotatedVector;
            UpdateViewMatrix();
        }
        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {            
            
            // TODO: Add your drawing code here
      //      DrawTerrain();

            device.Clear(Color.CornflowerBlue);
            device.RenderState.CullMode = CullMode.CullCounterClockwiseFace;
            device.RenderState.FillMode = FillMode.Solid;
            effect.CurrentTechnique = effect.Techniques["Technique1"];
            Matrix worldMatrix = Matrix.CreateTranslation(-terrainWidth / 2.0f, 0, terrainHeight / 2.0f) * Matrix.CreateScale(2.0f);
            effect.Parameters["xWorld"].SetValue(worldMatrix);
            effect.Parameters["xView"].SetValue(viewMatrix);
            effect.Parameters["xProjection"].SetValue(projectionMatrix);
            effect.Parameters["xEnableLighting"].SetValue(true);
            Vector3 LightDirection = new Vector3(1.0f, 10.0f, -1.0f);
            LightDirection.Normalize();
            effect.Parameters["xLightDirection"].SetValue(LightDirection);
            effect.Parameters["xAmbient"].SetValue(0.1f);
            effect.Parameters["xRocks"].SetValue(groundTexture);
            effect.Parameters["xGrass"].SetValue(grassTexture);
            effect.Parameters["xDirt"].SetValue(dirtTexture);
            effect.Begin();
            foreach (EffectPass pass in effect.CurrentTechnique.Passes)
            {
                pass.Begin();
                device.VertexDeclaration = heightMapVertexDeclaration;
                device.Vertices[0].SetSource(heightMapVertexBuffer, 0, VertexPositionTextureWeighted.SizeInBytes);
                A.root.Draw(device, effect, viewMatrix, projectionMatrix, terrainWidth, terrainHeight, TextureVertices);
                pass.End();
            }
            effect.End();
            DrawModel(Plane);
   //         foreach (QuadNode node in A.nodeList)
   //             BoundingBoxRenderer.Render(node.box, device, viewMatrix, projectionMatrix, Color.CornflowerBlue);
   //         foreach (QuadNode node in A.nodeList2)
   //             BoundingBoxRenderer.Render(node.box, device, viewMatrix, projectionMatrix, Color.Yellow);
   //         BoundingFrustumRenderer.Render(A.Frustum, device, viewMatrix, projectionMatrix, Color.Green);
            base.Draw(gameTime);
        }
        public void DrawModel(Model model)
        {
            Matrix[] transforms = new Matrix[model.Bones.Count];
            model.CopyAbsoluteBoneTransformsTo(transforms);

            foreach (ModelMesh mesh in model.Meshes)
            {
                foreach (BasicEffect effect in mesh.Effects)
                {
                    effect.EnableDefaultLighting();
                    effect.World = transforms[mesh.ParentBone.Index] * Matrix.CreateRotationY(modelRotation) * Matrix.CreateTranslation(modelPosition) * Matrix.CreateScale(0.5f);
                    effect.View = Matrix.CreateLookAt(cameraPos, cameraFinalTarget, cameraRotatedUpVector);
                    effect.Projection = Matrix.CreatePerspectiveFieldOfView(MathHelper.ToRadians(45.0f), device.Viewport.AspectRatio, 1.0f, 1000f);
                }
                mesh.Draw();
            }
        }
                


            
        private void DrawTerrain()
        {
            GraphicsDevice.Clear(Color.CornflowerBlue);
    
            int terrainWidth = heightMap.Width;
            int terrainHeight = heightMap.Height;
            device.RenderState.CullMode = CullMode.None;
            Matrix worldMatrix = Matrix.CreateTranslation(-terrainWidth / 2.0f, 0, terrainHeight / 2.0f);
            effect.CurrentTechnique = effect.Techniques["Technique1"];
            effect.Parameters["xWorld"].SetValue(worldMatrix);
            effect.Parameters["xView"].SetValue(viewMatrix);
            effect.Parameters["xProjection"].SetValue(projectionMatrix);
            effect.Parameters["xEnableLighting"].SetValue(true);
            Vector3 LightDirection = new Vector3(1.0f, -1.0f, -1.0f);
            LightDirection.Normalize();
            effect.Parameters["xLightDirection"].SetValue(LightDirection);
            effect.Parameters["xAmbient"].SetValue(0.1f);
            effect.Begin();
            foreach (EffectPass pass in effect.CurrentTechnique.Passes)
            {
                pass.Begin();
                device.VertexDeclaration = heightMapVertexDeclaration;
                device.Indices = terrainIndexBuffer;
                device.Vertices[0].SetSource(heightMapVertexBuffer, 0, VertexPositionTextureWeighted.SizeInBytes);
                device.DrawIndexedPrimitives(PrimitiveType.TriangleStrip, 0, 0, TextureVertices.Length, 0, terrainIndices.Length / 3);
                pass.End();
            }
        
            effect.End();
        }
    }
}
