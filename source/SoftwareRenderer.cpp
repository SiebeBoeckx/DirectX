//External includes
#include "pch.h"
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "SoftwareRenderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"
#include <iostream>

using namespace dae;

SoftwareRenderer::SoftwareRenderer(SDL_Window* pWindow, Camera* pCamera, std::vector<Texture*> pTextures) :
	BaseRenderer(pWindow, pCamera)
	, m_pTexture{pTextures[0]}
	, m_pNormals{pTextures[1]}
	, m_pSpecular{pTextures[2]}
	, m_pPhongExponent{pTextures[3]}
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];
}

SoftwareRenderer::~SoftwareRenderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pTexture;
	delete m_pNormals;
	delete m_pSpecular;
	delete m_pPhongExponent;
}

void SoftwareRenderer::Update(const Timer* pTimer)
{
	m_pCamera->Update(pTimer);

	//rotate mesh
	const float rotationSpeed{ PI / 4 };
	if (m_IsRotating)
	{
		m_pMesh->m_WorldMatrix = Matrix::CreateRotationY(rotationSpeed * pTimer->GetElapsed()) * m_pMesh->m_WorldMatrix;
	}
}

void SoftwareRenderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	RenderMesh();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void SoftwareRenderer::VertexTransformationFunction(const std::vector<Vertex_PosTex>& vertices_in, std::vector<Vertex_Out>& vertices_out, const Matrix& meshWorldMatrix)
{
	vertices_out.resize(vertices_in.size());

	const Matrix worldViewProjectionMatrix = meshWorldMatrix * m_pCamera->viewMatrix * m_pCamera->projectionMatrix;

	for (int i{}; i < vertices_in.size(); ++i)
	{
		Vector4 transformPos = worldViewProjectionMatrix.TransformPoint(Vector4{ vertices_in[i].position, 1 });

		transformPos.x /= transformPos.w;
		transformPos.y /= transformPos.w;
		transformPos.z /= transformPos.w;

		vertices_out[i].position = transformPos;
		vertices_out[i].uv = vertices_in[i].TexCoord;
		vertices_out[i].normal = meshWorldMatrix.TransformVector(vertices_in[i].normal).Normalized(); //Normal and tangent in world space
		vertices_out[i].tangent = meshWorldMatrix.TransformVector(vertices_in[i].tangent).Normalized();
		vertices_out[i].viewDirection = (meshWorldMatrix.TransformPoint(vertices_in[i].position) - m_pCamera->origin).Normalized();
	}
}

void SoftwareRenderer::RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const
{
	ColorRGB finalColor{  };

	//checking if pixel is in triangle
	//Bounding box
	Vector2 topLeft{ std::min(v0.position.x - 1 , v1.position.x - 1), std::min(v0.position.y - 1, v1.position.y - 1) };
	Vector2 bottomRight{ std::max(v0.position.x + 1, v1.position.x + 1), std::max(v0.position.y + 1, v1.position.y + 1) };

	topLeft = Vector2{ std::min(topLeft.x - 1, v2.position.x - 1), std::min(topLeft.y - 1, v2.position.y - 1) };
	bottomRight = Vector2{ std::max(bottomRight.x + 1, v2.position.x + 1), std::max(bottomRight.y + 1, v2.position.y + 1) };

	topLeft.x = Clamp((int)topLeft.x, 0, m_Width - 1);
	bottomRight.x = Clamp((int)bottomRight.x, 0, m_Width - 1);
	topLeft.y = Clamp((int)topLeft.y, 0, m_Height - 1);
	bottomRight.y = Clamp((int)bottomRight.y, 0, m_Height - 1);

	for (int py{ int(topLeft.y) }; py < bottomRight.y; ++py)
	{
		for (int px{ int(topLeft.x) }; px < bottomRight.x; ++px)
		{
			const Vector2 pixelPos = Vector2{ (float)px, (float)py };

			//side A cross check
			const Vector2 sideA = Vector2{ v1.position.x - v0.position.x,
										   v1.position.y - v0.position.y };

			const Vector2 vertex1ToPixel = Vector2{ pixelPos.x - v0.position.x,
													pixelPos.y - v0.position.y };

			const float crossA{ Vector2::Cross(sideA, vertex1ToPixel) };

			if (crossA < 0) continue;

			//side B cross check
			const Vector2 sideB = Vector2{ v2.position.x - v1.position.x,
										   v2.position.y - v1.position.y };

			const Vector2 vertex2ToPixel = Vector2{ pixelPos.x - v1.position.x,
													pixelPos.y - v1.position.y };

			const float crossB{ Vector2::Cross(sideB, vertex2ToPixel) };

			if (crossB < 0) continue;

			//side C cross check
			const Vector2 sideC = Vector2{ v0.position.x - v2.position.x,
										   v0.position.y - v2.position.y };

			const Vector2 vertex3ToPixel = Vector2{ pixelPos.x - v2.position.x,
													pixelPos.y - v2.position.y };

			const float crossC{ Vector2::Cross(sideC, vertex3ToPixel) };

			if (crossC < 0) continue;

			//pixel is in triangle		
			float weight2 = crossA;
			float weight0 = crossB;
			float weight1 = crossC;

			const float totalWeight{ weight0 + weight1 + weight2 };

			weight0 /= totalWeight;
			weight1 /= totalWeight;
			weight2 /= totalWeight;

			//const float interpolatedDepth = v0.position.z * weight0 + v1.position.z * weight1 + v2.position.z * weight2; //Linear
			const float interpolatedZDepth = 1.f /
				(
					(1.f / v0.position.z) * weight0 +
					(1.f / v1.position.z) * weight1 +
					(1.f / v2.position.z) * weight2
					); //Quadratic-ish?

			if (interpolatedZDepth < 0 || interpolatedZDepth > 1) continue; //Interpolated depth not in [0,1] range, frustrum culling for z

			if (m_pDepthBufferPixels[px * m_Height + py] < interpolatedZDepth) continue; //Depth test

			m_pDepthBufferPixels[px * m_Height + py] = interpolatedZDepth; //Depth write

			//const Vector2 interpolatedUV = v0.uv * weight0 + v1.uv * weight1 + v2.uv * weight2; //Linear
			const float	interpolatedWDepth = 1.f /
					(
					(1.f / v0.position.w) * weight0 +
					(1.f / v1.position.w) * weight1 +
					(1.f / v2.position.w) * weight2
					); //Quadratic-ish?

			const Vector2 interpolatedUV = (((v0.uv / v0.position.w) * weight0) +
											((v1.uv / v1.position.w) * weight1) +
											((v2.uv / v2.position.w) * weight2))
											* interpolatedWDepth;		
		
			Vertex_Out outputPixel;
			outputPixel.position = Vector4{ pixelPos.x, pixelPos.y, interpolatedZDepth, interpolatedWDepth };

			outputPixel.uv = interpolatedUV;

			outputPixel.normal = ((((v0.normal / v0.position.w) * weight0) +
								 ((v1.normal / v1.position.w) * weight1) +
								 ((v2.normal / v2.position.w) * weight2))
								 * interpolatedWDepth).Normalized();

			outputPixel.tangent = ((((v0.tangent / v0.position.w) * weight0) +
								  ((v1.tangent / v1.position.w) * weight1) +
								  ((v2.tangent / v2.position.w) * weight2))
								  * interpolatedWDepth).Normalized();

			outputPixel.viewDirection = ((((v0.viewDirection / v0.position.w) * weight0) +
										((v1.viewDirection / v1.position.w) * weight1) +
										((v2.viewDirection / v2.position.w) * weight2))
										* interpolatedWDepth).Normalized();

			finalColor = PixelShading(outputPixel);

			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));

		}
	}
}

bool SoftwareRenderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void SoftwareRenderer::RenderMesh()
{
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	SDL_FillRect(m_pBackBuffer, &m_pBackBuffer->clip_rect, 100);

	ColorRGB clearColor = ColorRGB{ 100,100,100 };
	Uint32 clearColorUint = 0xFF000000 | (Uint32)clearColor.r | (Uint32)clearColor.g << 8 | (Uint32)clearColor.b << 16;
	SDL_FillRect(m_pBackBuffer, NULL, clearColorUint);

	//RENDER LOGIC

	//convert to screen space
	VertexTransformationFunction(m_pMesh->GetVertices(), m_pMesh->m_Vertices_out, m_pMesh->m_WorldMatrix);

	if (m_pMesh->m_topology == PrimitiveTopology::TriangleList)
	{
		for (int i{}; i < m_pMesh->GetIndices().size() / 3; ++i)
		{
			Vertex_Out v0 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i * 3]];
			Vertex_Out v1 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i * 3 + 1]];
			Vertex_Out v2 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i * 3 + 2]];

			if ((v0.position.x < -1 || v0.position.x > 1) || (v0.position.y < -1 || v0.position.y > 1)) continue;
			if ((v1.position.x < -1 || v1.position.x > 1) || (v1.position.y < -1 || v1.position.y > 1)) continue;
			if ((v2.position.x < -1 || v2.position.x > 1) || (v2.position.y < -1 || v2.position.y > 1)) continue;

			//NDC to raster space
			v0.position.x = (v0.position.x + 1) / 2.f * m_Width;
			v0.position.y = (1 - v0.position.y) / 2.f * m_Height;

			v1.position.x = (v1.position.x + 1) / 2.f * m_Width;
			v1.position.y = (1 - v1.position.y) / 2.f * m_Height;

			v2.position.x = (v2.position.x + 1) / 2.f * m_Width;
			v2.position.y = (1 - v2.position.y) / 2.f * m_Height;

			RenderTriangle(v0, v1, v2);
		}
	}
	else
	{
		for (int i{}; i < m_pMesh->GetIndices().size() - 2; ++i)
		{
			if (i % 2 != 0)
			{
				Vertex_Out v0 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i]];
				Vertex_Out v1 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i + 2]];
				Vertex_Out v2 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i + 1]];

				if ((v0.position.x < -1 || v0.position.x > 1) || (v0.position.y < -1 || v0.position.y > 1)) continue;
				if ((v1.position.x < -1 || v1.position.x > 1) || (v1.position.y < -1 || v1.position.y > 1)) continue;
				if ((v2.position.x < -1 || v2.position.x > 1) || (v2.position.y < -1 || v2.position.y > 1)) continue;

				//NDC to raster space
				v0.position.x = (v0.position.x + 1) / 2.f * m_Width;
				v0.position.y = (1 - v0.position.y) / 2.f * m_Height;

				v1.position.x = (v1.position.x + 1) / 2.f * m_Width;
				v1.position.y = (1 - v1.position.y) / 2.f * m_Height;

				v2.position.x = (v2.position.x + 1) / 2.f * m_Width;
				v2.position.y = (1 - v2.position.y) / 2.f * m_Height;

				RenderTriangle(v0, v1, v2);
			}
			else
			{
				Vertex_Out v0 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i]];
				Vertex_Out v1 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i + 1]];
				Vertex_Out v2 = m_pMesh->m_Vertices_out[m_pMesh->GetIndices()[i + 2]];

				if ((v0.position.x < -1 || v0.position.x > 1) || (v0.position.y < -1 || v0.position.y > 1)) continue;
				if ((v1.position.x < -1 || v1.position.x > 1) || (v1.position.y < -1 || v1.position.y > 1)) continue;
				if ((v2.position.x < -1 || v2.position.x > 1) || (v2.position.y < -1 || v2.position.y > 1)) continue;

				//NDC to raster space
				v0.position.x = (v0.position.x + 1) / 2.f * m_Width;
				v0.position.y = (1 - v0.position.y) / 2.f * m_Height;

				v1.position.x = (v1.position.x + 1) / 2.f * m_Width;
				v1.position.y = (1 - v1.position.y) / 2.f * m_Height;

				v2.position.x = (v2.position.x + 1) / 2.f * m_Width;
				v2.position.y = (1 - v2.position.y) / 2.f * m_Height;

				RenderTriangle(v0, v1, v2);
			}
		}
	}
}

void SoftwareRenderer::CycleRenderState()
{
	std::cout << "Renderstate: ";
	switch (m_State)
	{
	case RenderState::texture:
		m_State = RenderState::depth;
		std::cout << "depth\n";
		break;
	case RenderState::depth:
		m_State = RenderState::observedArea;
		std::cout << "observed area\n";
		break;
	case RenderState::observedArea:
		m_State = RenderState::phong;
		std::cout << "phong\n";
		break;
	case RenderState::phong:
		m_State = RenderState::texture;
		std::cout << "combined\n";
		break;
	}
}

void SoftwareRenderer::ToggleNormalMap()
{
	m_UsingNormalMap = !m_UsingNormalMap;
}

ColorRGB SoftwareRenderer::PixelShading(const Vertex_Out& vertex) const
{
	ColorRGB returnColor{};
	//normal map
	Vector3 sampledNormal{ vertex.normal };

	if (m_UsingNormalMap)
	{
		const Vector3 binormal{ Vector3::Cross(vertex.normal, vertex.tangent) };
		const Matrix tangentSpaceAxis{ vertex.tangent,
										binormal,
										vertex.normal,
										Vector3::Zero
		};
		const ColorRGB normalColor = m_pNormals->Sample(vertex.uv); //normal

		sampledNormal.x = 2.f * normalColor.r - 1.f;
		sampledNormal.y = 2.f * normalColor.g - 1.f;
		sampledNormal.z = 2.f * normalColor.b - 1.f;

		sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);

		sampledNormal.Normalize();
	}
	//observed area
	float cosineLaw{ Vector3::Dot(sampledNormal, -m_Light.direction)};
	cosineLaw = Saturate(cosineLaw);

	//sample diffuse color
	const ColorRGB diffuseColor = m_pTexture->Sample(vertex.uv) / PI; //Diffuse

	//Phong
	const float shininess{ 25.f };

	const float specular = m_pSpecular->Sample(vertex.uv).r; //Specular
	const float phongExp = m_pPhongExponent->Sample(vertex.uv).r * shininess; //Phong exponent

	const ColorRGB phongColor = Phong(specular, phongExp, -m_Light.direction, vertex.viewDirection, sampledNormal);

	// Remap the Z depth
	const float depthRemap{ Remap(vertex.position.z, 0.995f, 1.0f) };
	switch (m_State)
	{
	case RenderState::texture:
		returnColor = m_Light.intensity * diffuseColor * cosineLaw + phongColor + m_Light.ambientColor;
		break;

	case RenderState::depth:
		// Set the color to showcase the depth
		returnColor = { depthRemap, depthRemap, depthRemap };
		break;

	case RenderState::observedArea:
		returnColor = { cosineLaw, cosineLaw, cosineLaw };
		break;

	case RenderState::phong:
		returnColor = phongColor;
		break;
	}
	
	return returnColor;
}

ColorRGB SoftwareRenderer::Phong(float specular, float exp, const Vector3& l, const Vector3& v, const Vector3& n) const
{
	//todo: W3
	//assert(false && "Not Implemented Yet");
	const Vector3 reflect = Vector3::Reflect(l,n);
	const float cosAlpha = std::max(0.f, Vector3::Dot(reflect, v));
	const float value = specular * powf(cosAlpha, exp);
	return ColorRGB{ value,value,value };
}

Mesh_PosTexSoftwareVehicle* SoftwareRenderer::InitializeMesh()
{
	std::vector<Vertex_PosTex> vertices{ };
	std::vector<uint32_t> indices{ };

	Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

	const Vector3 position{ Vector3{0.f, 0.f, 50.f} };
	const Vector3 rotation{ };
	const Vector3 scale{ Vector3{ 1.f, 1.f, 1.f } };
	Matrix worldMatrix = Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(position);

	return new Mesh_PosTexSoftwareVehicle(vertices, indices, worldMatrix, PrimitiveTopology::TriangleList);
}